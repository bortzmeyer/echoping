/*
 * echoping : uses the TCP echo service to measure (roughly) response times.
 * 
 * Written by Stephane Bortzmeyer <bortz@users.sourceforge.net>. See
 * the AUTHORS file for other contributors.
 * 
 * $Id$
 * */

char *progname;
unsigned short timeout_flag;

#include	"echoping.h"

/*
 * An option to define only if you want to drive echoping from another
 * process. Useless but harmless otherwise. In practice, while OSF/1 is happy
 * with it, SunOS refuses to use fflush on a NULL and Linux fails.
 */
#undef FLUSH_OUTPUT

/* Global variables for main and printstats */

int return_code = 0;
unsigned int number = 1;
struct timeval max, min, total, median, temp;
unsigned int successes, attempts = 0;
unsigned int size = DEFLINE;
unsigned int j = 0;
struct result
{
  unsigned short valid;
  struct timeval timevalue;
};
struct result results[MAXNUMBER];
struct timeval good_results[MAXNUMBER];
extern int tvcmp ();

int
main (argc, argv)
     int argc;
     char *argv[];
{
  extern char *optarg;
  extern int optind;

  signed char ch;

  int sockfd;
  FILE *files = NULL;
  CHANNEL channel;
  struct hostent *hostptr;
  struct sockaddr_in serv_addr;
  struct sockaddr_in udp_cli_addr;	/* client's Internet socket
					 * addr */
  struct servent *sp = NULL;
  int verbose = FALSE;
  char *server_address;
  u_int addr;
  struct in_addr *ptr;
  int n, nr = 0;
  int rc;
#ifdef OPENSSL
  int sslcode;	
  char rand_file[MAXLINE];
#endif
  char *sendline, recvline[MAXLINE + 1];
#ifdef ICP
  char retcode[DEFLINE];
  int length;
#endif
  struct timeval newtv, oldtv;
  void printstats ();

  unsigned int wait = 1;
  unsigned char fill = ' ';
  unsigned short fill_requested = 0;
  unsigned int i = 0;

  void to_alarm ();		/* our alarm() signal handler */
  void interrupted ();
  unsigned int timeout = 10;
  unsigned short timeout_requested = 0;
  unsigned short size_requested = 0;
  char *url = "";
  short port = 0;
  char *text_port = malloc (6);
#if USE_SIGACTION
  struct sigaction mysigaction;
#endif

  char *port_name = ECHO_TCP_PORT;
  unsigned short port_to_use = USE_ECHO;
  unsigned short http = 0;
  unsigned short smtp = 0;
  unsigned short udp = 0;
  unsigned short icp = 0;
#ifdef ICP
  icp_opcode opcode = ICP_OP_QUERY;
#endif

  unsigned short ttcp = 0;
  unsigned short tcp = 0;
  unsigned short ssl = 0;

  unsigned short stop_at_newlines = 1;

#ifdef OPENSSL
  SSL_METHOD *meth;
  SSL_CTX *ctx = NULL;
  SSL *sslh = NULL;
#endif

  null_timeval.tv_sec = 0;
  null_timeval.tv_usec = 0;
  max_timeval.tv_sec = 1000000000;
  max_timeval.tv_usec = 999999;

  return_code = 0;
  number = 1;
  total = null_timeval;
  median = null_timeval;
  max = null_timeval;
  min = max_timeval;

  for (i = 0; i <= MAXNUMBER; i++)
    {
      results[i].valid = 0;
    }
  progname = argv[0];
  while ((ch = getopt (argc, argv, "vs:n:w:dch:i:rut:f:SC")) != EOF)
    {
      switch (ch)
	{
	case 'v':
	  verbose = TRUE;
	  break;
	case 'r':
	  ttcp = 1;
	  break;
	case 'u':
	  udp = 1;
	  break;
	case 'C':
	  ssl = 1;
	  break;
	case 'd':
	  port_name = DISCARD_TCP_PORT;
	  port_to_use = USE_DISCARD;
	  break;
	case 'c':
	  port_name = CHARACTER_GENERATOR_TCP_PORT;
	  port_to_use = USE_CHARGEN;
	  stop_at_newlines = 0;
	  break;
	case 'i':
	  port_name = DEFAULT_ICP_UDP_PORT;
	  port_to_use = USE_ICP;
	  udp = 1;
	  icp = 1;
	  url = optarg;
	  break;
	case 'h':
	  port_name = DEFAULT_HTTP_TCP_PORT;
	  port_to_use = USE_HTTP;
	  http = 1;
	  url = optarg;
	  break;
	case 'f':
	  fill = *optarg;
	  fill_requested = 1;
	  break;
	case 'S':
	  port_name = "smtp";
	  port_to_use = USE_SMTP;
	  smtp = 1;
	  break;
	case 's':
	  size = atoi (optarg);
	  if (size > MAXLINE)
	    {
	      (void) fprintf (stderr,
			      "%s: packet size too large, max is %d.\n",
			      progname, MAXLINE);
	      exit (1);
	    }
	  if (size <= 0)
	    {
	      (void) fprintf (stderr, "%s: illegal packet size.\n", progname);
	      exit (1);
	    }
	  size_requested = 1;
	  break;
	case 't':
	  timeout = atoi (optarg);
	  timeout_requested = 1;
	  if (size <= 0)
	    {
	      (void) fprintf (stderr, "%s: illegal timeout.\n", progname);
	      exit (1);
	    }
	  break;
	case 'n':
	  number = atoi (optarg);
	  if (number > MAXNUMBER)
	    {
	      (void) fprintf (stderr,
			      "%s: number of iterations too large, max is %d.\n",
			      progname, MAXNUMBER);
	      exit (1);
	    }
	  if (number <= 0)
	    {
	      (void) fprintf (stderr,
			      "%s: illegal number of iterations.\n",
			      progname);
	      exit (1);
	    }
	  break;
	case 'w':
	  wait = atoi (optarg);
	  if (wait <= 0)
	    /* atoi returns zero when there is an error. So we cannot use 
	       '-w 0' to specify no waiting. */
	    {
	      (void) fprintf (stderr,
			      "%s: illegal waiting time.\n", progname);
	      exit (1);
	    }
	  break;
	default:
	  usage ();
	}
    }
  if (udp && ((port_to_use == USE_CHARGEN) ||
	      (port_to_use == USE_HTTP) || (port_to_use == USE_SMTP)))
    {
      (void) fprintf (stderr,
		      "%s: I don't know how to use this port with UDP.\n",
		      progname);
      exit (1);
    }
/*  
   Version 2.1 now allows global timeouts for TCP connections
   *
   if (!udp && (timeout_requested))
   {
   (void) fprintf (stderr,
   "%s: Time out ignored for TCP connections.\n", progname);
   exit (1);
   }
 */
  if ((http || smtp) && (fill_requested))
    {
      (void) fprintf (stderr,
		      "%s: Filling incompatible with HTTP connections.\n",
		      progname);
      exit (1);
    }
#ifndef USE_TTCP
  if (ttcp)
    {
      (void) fprintf (stderr,
		      "%s: not compiled with T/TCP support.\n", progname);
      exit (1);
    }
#endif
#ifndef OPENSSL
  if (ssl)
    {
      (void) fprintf (stderr,
		      "%s: not compiled with SSL support.\n", progname);
      exit (1);
    }
#endif
#ifndef HTTP
  if (http)
    {
      (void) fprintf (stderr,
		      "%s: Not compiled with HTTP support.\n", progname);
      exit (1);
    }
#endif
#ifndef SMTP
  if (smtp)
    {
      (void) fprintf (stderr,
		      "%s: Not compiled with SMTP support.\n", progname);
      exit (1);
    }
#endif
#ifndef ICP
  if (icp)
    {
      (void) fprintf (stderr,
		      "%s: Not compiled with ICP support.\n", progname);
      exit (1);
    }
#endif
  if ((http || smtp) && size_requested)
    {
      (void) fprintf (stderr,
		      "%s: %s and message size specification are incompatible.\n",
		      http ? "HTTP" : "SMTP", progname);
      exit (1);
    }
  if (ssl && !http)
    {
      (void) fprintf (stderr,
		      "%s: SSL is only supported for HTTP requests.\n",
		      progname);
      exit (1);
    }
  if (udp && ttcp)
    {
      (void) fprintf (stderr,
		      "%s: UDP and T/TCP are incompatible.\n", progname);
      exit (1);
    }
  if (ssl && http)
    {
      port_name = DEFAULT_HTTPS_TCP_PORT;
    }
  if (!udp && !ttcp)
    {
      tcp = 1;
    }
  argc -= optind;
  argv += optind;
  if (argc != 1)
    {
      usage ();
    }
  if (verbose)
    {
      printf ("\nThis is %s, version %s.\n\n", progname, VERSION);
    }
  server = argv[0];
#ifdef HTTP
  if (http || icp)
    {
      if (icp)
	{
	  find_server_and_port (server, &port, port_name);
	  if (port == 0)
	    port = 3130;
	}
      else
	{
	  find_server_and_port (server, &port, port_name);
	  if (port == 0)
	    port = 80;
	}

      sprintf (text_port, "(port %d)", ntohs (port));
    }
#endif
  signal (SIGINT, interrupted);
  if ((addr = inet_addr (server)) == INADDR_NONE)
    {
      if ((hostptr = gethostbyname (server)) == NULL)
	{
	  err_quit ("gethostbyname error for host: %s %s",
		    server, sys_err_str ());
	}
      server_address = *(hostptr->h_addr_list);	/* First item of the
						 * list */
      /*
       * addr = (u_long) *server_address; 
       */
      /* ptr.s_addr = addr; */
      ptr = (struct in_addr *) server_address;	/* hostptr->h_addr_list
						 * points actually to
						 * u_longs, not strings */
      addr = ptr->s_addr;
    }
  else
    {
      ptr = (struct in_addr *) malloc (sizeof (struct in_addr));
      ptr->s_addr = addr;
    }
  if (!http && !icp)		/* Already find */
    {
      if (!udp)
	{
	  if ((sp = getservbyname (port_name, "tcp")) == NULL)
	    {
	      err_quit ("tcp_open: unknown service: %s/tcp", port_name);
	    }
	}
      else
	{
	  if ((sp = getservbyname (port_name, "udp")) == NULL)
	    {
	      err_quit ("tcp_open: unknown service: %s/udp", port_name);
	    }
	}
    }
  /*
   * Fill in the structure "serv_addr" with the address of the server
   * that we want to connect with.
   */

  bzero ((char *) &serv_addr, sizeof (serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = addr;
  if (!http && !icp)
    {
      serv_addr.sin_port = sp->s_port;
    }
  else
    {
      serv_addr.sin_port = port;
    }

#ifdef HTTP
  if (http)
    {
      sendline = make_http_sendline (url, server, (int) ntohs (port));
    }
  else
#endif
#ifdef SMTP
  if (smtp)
    {
      sendline = "QUIT\r\n";
    }
  else
#endif
#ifdef ICP
  if (icp)
    {
      sendline = make_icp_sendline (url, &addr, opcode, &length);
    }
  else
#endif
  if (!fill_requested)
    {
      sendline = random_string (size);
    }
  else
    {
      sendline = (char *) malloc (size);
      for (i = 0; i < size; i++)
	sendline[i] = fill;
    }
  n = strlen (sendline);

#ifdef OPENSSL
  if (ssl)
    {
      SSL_load_error_strings ();
      SSLeay_add_ssl_algorithms ();
      /* The following RAND_ calls are only for systems insecure
         enough to fail to have /dev/urandom. Bug #132001 */
      RAND_file_name (rand_file, sizeof( rand_file));
      RAND_write_file (rand_file);
      RAND_load_file (rand_file, 1024);
      meth = SSLv2_client_method ();
      if ((ctx = SSL_CTX_new (meth)) == NULL)
	err_sys ("Cannot create a new SSL context");
    }
#endif

  for (i = 1; i <= number; i++)
    {

      attempts++;
#ifdef OPENSSL
      if (ssl)
	/* Despite what the OpenSSL documentation says, we must
	   allocate a new SSL structure at each iteration, otherwise,
	   *some* SSL servers fail at the second iteration with:
	   error:1406D0D9:SSL routines:GET_SERVER_HELLO:reuse cert type not zero 
	   Bug #130151 */
	if ((sslh = SSL_new (ctx)) == NULL)
	  err_sys ("Cannot initialize SSL context");
#endif
      if (!udp)
	{
	  /*
	   * Open a TCP socket (an Internet stream socket).
	   */

	  if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
	    err_sys ("Can't open stream socket");
	}
      else
	{
	  if ((sockfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
	    err_sys ("Can't open datagram socket");
	  /* Bind socket for reply. Not necessary? */
	  bzero ((char *) &udp_cli_addr, sizeof (udp_cli_addr));
	  udp_cli_addr.sin_family = AF_INET;
	  udp_cli_addr.sin_addr.s_addr = htonl (INADDR_ANY);
	  udp_cli_addr.sin_port = htons (0);
	  if (bind (sockfd, (struct sockaddr *) &udp_cli_addr,
		    sizeof (udp_cli_addr)) < 0)
	    {
	      err_sys ("bind error");
	    }
	}
      if (verbose)
	{
	  if (tcp)
	    {
	      printf
		("Trying to connect to internet address %s %s to transmit %u bytes...\n",
		 inet_ntoa (*ptr), (port == 0 ? "" : text_port), n);
	    }
#ifdef ICP
	  if (icp)
	    {
	      printf
		("Trying to send an ICP packet of %u bytes to the internet address %s...\n",
		 length, inet_ntoa (*ptr));
	    }
#endif
	  else
	    {
	      printf ("Trying to send %u bytes to internet address %s...\n",
		      size, inet_ntoa (*ptr));
	    }
#ifdef FLUSH_OUTPUT
	  if (fflush ((FILE *) NULL) != 0)
	    {
	      err_sys ("I cannot flush");
	    }
#endif
	}
      if (tcp && timeout_requested)	/* echoping's timeout has a different semantic in TCP and UDP */
	{
#ifdef USE_SIGACTION
	  mysigaction.sa_handler = to_alarm;
	  sigemptyset (&mysigaction.sa_mask);
	  /* Default behavior doesn't seem portable? */
#ifdef SA_INTERRUPT
	  mysigaction.sa_flags = SA_INTERRUPT;
#else
	  mysigaction.sa_flags = (int) 0;
#endif
	  if ((sigaction (SIGALRM, &mysigaction, NULL)) < 0)
	    err_sys ("Cannot set signal handler");
#else
	  signal (SIGALRM, to_alarm);
#endif
	  timeout_flag = 0;	/* for signal handler */
	  alarm (timeout);
	}
      (void) gettimeofday (&oldtv, (struct timezone *) NULL);
      if (!ttcp && !icp)
	{
	  /*
	   * Connect to the server.
	   */

	  if (connect (sockfd, (struct sockaddr *) &serv_addr,
		       sizeof (serv_addr)) < 0)
	    {
	      if ((errno == EINTR) && (timeout_flag))
		{
		  printf ("Timeout while connecting\n");
		  continue;
#ifdef FLUSH_OUTPUT
		  if (fflush ((FILE *) NULL) != 0)
		    {
		      err_sys ("I cannot flush");
		    }
#endif
		}
	      else
		err_sys ("Can't connect to server");
	    }
	  if (verbose && tcp)
	    {
	      printf ("Connected...\n");
#ifdef FLUSH_OUTPUT
	      if (fflush ((FILE *) NULL) != 0)
		{
		  err_sys ("I cannot flush");
		}
#endif
	    }
	  if (!udp && !ssl)
	    if ((files = fdopen (sockfd, "r")) == NULL)
	      err_sys ("Cannot fdopen");
#ifdef OPENSSL
	  if (ssl)
	    {
	      SSL_set_fd (sslh, sockfd);
	      if (SSL_connect (sslh) == -1)
		if ((errno == EINTR) && (timeout_flag))
		  {
		    printf ("Timeout while starting SSL\n");
		    continue;
		  }
	      if (verbose)
		printf ("SSL connection using %s\n", SSL_get_cipher (sslh));
	      /* We could check the server's certificate or other funny
	         things */
	    }
#endif
	}
      /* Not T/TCP */
      else
	{
	  /* No initial connection */
	}
      if ((port_to_use == USE_ECHO) || (port_to_use == USE_DISCARD) ||
	  (port_to_use == USE_HTTP) || (port_to_use == USE_ICP) ||
	  (port_to_use == USE_SMTP))
	{
#ifdef USE_TTCP
	  if (ttcp)
	    {
	      if (sendto (sockfd, sendline, n, MSG_EOF,
			  (struct sockaddr *) &serv_addr,
			  sizeof (serv_addr)) != n)
		err_sys ("sendto error on socket");
	      if (verbose)
		{
		  printf ("T/TCP connection done\n");
		}
	    }
	  else
#endif
	  if (!udp)
	    {
	      if (!ssl)
		{
		  /* Write something to the server */
		  if (writen (sockfd, sendline, n) != n)
		    {
		      if ((nr < 0 || nr != n) && timeout_flag)
			{
			  nr = n;
			  printf ("Timeout while writing\n");
			  continue;
			}
		      else
			err_sys ("writen error on socket");
		    }
		}
#ifdef OPENSSL
	      else
		{
		  if ((rc = SSL_write (sslh, sendline, n)) != n)
		    {
		      if ((nr < 0 || nr != n) && timeout_flag)
			{
			  nr = n;
			  printf ("Timeout while writing\n");
			  continue;
			}
		      else
			{
			  sslcode = ERR_get_error ();
			  err_sys ("SSL_write error on socket: %s",
				   ERR_error_string (sslcode, NULL));
			}
		    }
		}
#endif
	    }
	  else
	    {
#ifdef ICP
	      if (icp)
		{
		  if (sendto (sockfd, sendline, length, 0,
			      &serv_addr, sizeof (serv_addr)) != length)
		    err_sys ("sendto error on socket");
		}
	      else
#endif
		/*
		 * if (sendto(sockfd, sendline, n, 0,
		 * &serv_addr, sizeof(serv_addr)) != n)
		 * err_sys("sendto error on socket");
		 */
	      if (send (sockfd, sendline, n, 0) != n)
		err_sys ("send error on socket");
	    }
	  if (verbose)
	    {
#ifdef ICP
	      if (icp)
		printf ("Sent (%d bytes)...\n", length);
	      else
#endif
		printf ("Sent (%d bytes)...\n", n);

#ifdef FLUSH_OUTPUT
	      if (fflush ((FILE *) NULL) != 0)
		{
		  err_sys ("I cannot flush");
		}
#endif
	    }
	}
      if ((port_to_use == USE_ECHO) || (port_to_use == USE_CHARGEN) ||
	  (port_to_use == USE_HTTP) || (port_to_use == USE_ICP) ||
	  (port_to_use == USE_SMTP))
	{
	  if (!udp)
	    {
	      if (!http && !smtp)
		{
		  /* Read from the server */
		  nr = readline (files, recvline, n, stop_at_newlines);
		}
#ifdef HTTP
	      else if (http)
		{
		  if (!ssl)
		    channel.fs = files;
#ifdef OPENSSL
		  else
		    channel.ssl = sslh;
#endif
		  nr = read_from_server (channel, ssl);
		}
#endif
#ifdef SMTP
	      else if (smtp)
		{
		  nr = smtp_read_response_from_server (files);
		}
#endif
	    }
	  else
	    {
#ifdef USE_SIGACTION
	      mysigaction.sa_handler = to_alarm;
	      sigemptyset (&mysigaction.sa_mask);
#ifdef SA_INTERRUPT
	      mysigaction.sa_flags = SA_INTERRUPT;
#else
	      mysigaction.sa_flags = (int) 0;
#endif
	      if ((sigaction (SIGALRM, &mysigaction, NULL)) < 0)
		err_sys ("Cannot set signal handler");
#else
	      signal (SIGALRM, to_alarm);
#endif
	      timeout_flag = 0;	/* for signal handler */
	      alarm (timeout);
#ifdef ICP
	      if (icp)
		{
		  nr = recv_icp (sockfd, recvline, retcode);
		  if (verbose)
		    {
		      printf ("%s\n", retcode);
		    }
		}
	      else
		{
#endif
		  nr = recv (sockfd, recvline, n, 0);
		  /*
		   * nr = recvfrom(sockfd, recvline, n, 0,
		   * (struct sockaddr *) 0, (int *) 0);
		   * recvfrom fails on SunOS on connected
		   * sockets.
		   */
		  /*
		   * Todo: in UDP, we should loop to read: we
		   * can have several reads necessary.
		   */
		  alarm (0);
		  if ((nr < 0) && (errno == EINTR) && (timeout_flag))
		    {
		      nr = n;
		      printf ("Timeout\n");
#ifdef FLUSH_OUTPUT
		      if (fflush ((FILE *) NULL) != 0)
			{
			  err_sys ("I cannot flush");
			}
#endif
		    }
#ifdef ICP
		}
#endif
	    }
	  if (!http && !icp && !smtp)
	    {
	      if ((nr < 0 || nr != n) && timeout_flag)
		/* if ((nr < 0 || nr != n) && (errno == EINTR) && timeout_flag) */
		{
		  printf ("Timeout while reading (%d byte(s) read)\n",
			  (nr == -1) ? 0 : nr);
		  nr = n;
#ifdef FLUSH_OUTPUT
		  if (fflush ((FILE *) NULL) != 0)
		    {
		      err_sys ("I cannot flush");
		    }
#endif
		  continue;
		}
	      if (nr < 0 || nr != n)
		err_sys ("readline error: %d bytes read, %d bytes requested",
			 nr, n);
	    }
	  else
	    /* This is HTTP */
	    {
	      if ((nr < 0) && (errno == EINTR) && (timeout_flag))
		{
		  printf ("Timeout while reading (%d byte(s) read)\n",
			  (nr == -1) ? 0 : nr);
#ifdef FLUSH_OUTPUT
		  if (fflush ((FILE *) NULL) != 0)
		    {
		      err_sys ("I cannot flush");
		    }
#endif
		  continue;
		}
	      if (nr < 0)
		{
		  err_ret ("Error reading HTTP reply");
		}
	    }
	  if (verbose)
	    printf ("%d bytes read from server.\n", nr);
	}
      /* That's all, folks */
      if (http)
	{
#ifdef OPENSSL
	  if (ssl)
	    SSL_shutdown (channel.ssl);
	  else
#endif
	    fclose (channel.fs);
	}
      close (sockfd);

      (void) gettimeofday (&newtv, (struct timezone *) NULL);
      if (tcp)
	alarm (0);
      temp = newtv;
      tvsub (&temp, &oldtv);
      if (!timeout_flag)
	{
	  tvadd (&total, &temp);

	  /* Check */
	  if (port_to_use == USE_ECHO)
	    {
	      if (strcmp (sendline, recvline) != 0)
		{
		  printf (" I wrote:\n%s\n", sendline);
		  printf (" and I got back:\n%s\n", recvline);
		  err_quit ("Strange server");
		}
	      if (verbose)
		{
		  printf ("Checked\n");
#ifdef FLUSH_OUTPUT
		  if (fflush ((FILE *) NULL) != 0)
		    {
		      err_sys ("I cannot flush");
		    }
#endif
		}
	    }
	  if (port_to_use == USE_CHARGEN)
	    {
	      sendline = CHARGENERATED;
	      recvline[strlen (sendline)] = 0;
	      if (strcmp (sendline, recvline) != 0)
		{
		  /* Todo: it does not work if the size is lower than the
		     length of CHARGENERATED */
		  printf (" I got back:\n%s\n", recvline);
		  printf (" instead of the most common:\n%s\n", sendline);
		  err_ret ("Strange server");
		}
	      if (verbose)
		{
		  printf ("Checked\n");
		}
	    }
	  tvsub (&newtv, &oldtv);
	  tvmin (&min, &newtv);
	  tvmax (&max, &newtv);
	  printf ("Elapsed time: %d.%06d seconds\n",
		  (int) newtv.tv_sec, (int) newtv.tv_usec);
#ifdef FLUSH_OUTPUT
	  if (fflush ((FILE *) NULL) != 0)
	    {
	      err_sys ("I cannot flush");
	    }
#endif
	  results[i - 1].valid = 1;
	  results[i - 1].timevalue = newtv;
	  successes++;
	}
      if (number > 1)
	{
#ifdef OPENSSL
	  if (ssl)
	    {
	      /* SSL_clear (sslh); No, we have to free. Bug #130151 */
	      SSL_free (sslh);
	    }
#endif
	  sleep (wait);
	}
    }
  printstats ();
  if (successes >= 1)
    exit (0);
  else
    exit (1);
  /* It would be nice to clean here (SSL, etc) */
}


void
printstats ()
{

  int i;

  /* if ((number > 1) && ((!udp) || (successes > 0))) { */
  if (successes > 1)
    {
      printf ("---\n");
      if (successes < attempts)
	printf ("Warning: %d message(s) lost (%d %%)\n", attempts - successes,
		((attempts - successes) * 100) / attempts);
      printf ("Minimum time: %d.%06d seconds (%.0f bytes per sec.)\n",
	      (int) min.tv_sec, (int) min.tv_usec,
	      (double) size / tv2double (min));
      printf ("Maximum time: %d.%06d seconds (%.0f bytes per sec.)\n",
	      (int) max.tv_sec, (int) max.tv_usec,
	      (double) size / tv2double (max));
      tvavg (&total, successes);
      printf ("Average time: %d.%06d seconds (%.0f bytes per sec.)\n",
	      (int) total.tv_sec, (int) total.tv_usec,
	      (double) size / tv2double (total));
      /* The number of bytes/second, as printed above, is not really
         meaningful: size does not reflect the number of bytes exchanged.
         With echo, N = 2*size, with discard, N = size, with http, N = size + (response)... */
      for (i = 0; i < number; i++)
	{
	  if (results[i].valid)
	    good_results[j++] = results[i].timevalue;
	}
      if (successes != j)	/* Todo: bug! */
	err_quit ("successes (%d) is different from j (%d)", successes, j);
      qsort (good_results, successes, sizeof (struct timeval), tvcmp);
      /*
       * for (i = 1; i <= number; i++) { printf("---\nTime %d th:
       * %d.%06d seconds\n", i, results[i-1].tv_sec,
       * results[i-1].tv_usec);       }
       */
      if ((successes % 2) == 1)
	{
	  /*
	   * printf("Searching good_results[%d]\n", (successes
	   * + 1) / 2 - 1);
	   */
	  median = good_results[((successes + 1) / 2 - 1)];
	}
      else
	{
	  /*
	   * printf("Searching good_results[%d] and [%d]\n",
	   * (successes / 2) - 1, successes / 2);
	   */
	  tvadd (&median, &good_results[(successes / 2) - 1]);
	  tvadd (&median, &good_results[successes / 2]);
	  tvavg (&median, 2);
	}
      printf ("Median  time: %d.%06d seconds (%.0f bytes per sec.)\n",
	      (int) median.tv_sec, (int) median.tv_usec,
	      (double) size / tv2double (median));
    }
}

/*
 * Signal handler for timeouts (SIGALRM). This function is called when the
 * alarm() value that was set counts down to zero.  This indicates that we
 * haven't received a response from the server to the last datagram we sent.
 * All we do is set a flag and return from the signal handler. The occurrence
 * of the signal interrupts the recvfrom() system call (errno = EINTR) above,
 * and we then check the timeout_flag flag.
 */

void
to_alarm ()
{
  timeout_flag = 1;		/* set flag for function above */
}

void
interrupted ()
{
  printf ("Interrupted by user\n");
  printstats ();
  exit (1);
}

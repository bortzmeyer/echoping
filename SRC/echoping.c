/*
 * echoping : uses the TCP echo (or other TCP or UDP protocols)
 * service to measure (roughly) response times.
 * 
 * Written by Stephane Bortzmeyer <bortz@users.sourceforge.net>. See
 * the AUTHORS file for other contributors. 
 * 
 * $Id$
 *
 *  */


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
int rc;
unsigned int number = 1;
struct timeval max, min, total, median, stddev, temp;
struct timeval conntv, connectedtv, sendtv, recvtv;
unsigned int successes, attempts = 0;
unsigned int size = DEFLINE;
unsigned int j = 0;

int family = PF_UNSPEC;

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

  int result;

  int sockfd;
  struct addrinfo hints, hints_numeric, *res;
  int error;
  char hbuf[NI_MAXHOST], pbuf[NI_MAXSERV];
#ifdef NI_WITHSCOPEID
  const int niflags = NI_NUMERICHOST | NI_NUMERICSERV | NI_WITHSCOPEID;
#else
  const int niflags = NI_NUMERICHOST | NI_NUMERICSERV;
#endif

  FILE *files = NULL;
  CHANNEL channel;
  int verbose = FALSE;
  int n, nr = 0;
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

#ifdef HAVE_USLEEP
  double wait = 1.0;
#else
  unsigned int wait = 1;
#endif
  unsigned char fill = ' ';
  unsigned short fill_requested = 0;
  unsigned int i = 0;

  void to_alarm ();		/* our alarm() signal handler */
  void interrupted ();
  unsigned int timeout = 10;
  unsigned short timeout_requested = 0;
  unsigned short size_requested = 0;
  char *url = "";
#if USE_SIGACTION
  struct sigaction mysigaction;
#endif

  char port_name[NI_MAXSERV];
  unsigned short port_to_use = USE_ECHO;
  unsigned short http = 0;
  unsigned short smtp = 0;
  unsigned short discard = 0;
  unsigned short udp = 0;
  unsigned short icp = 0;

  unsigned short nocache = 0;

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
#ifdef GNUTLS
  gnutls_session session;
  gnutls_certificate_credentials xcred;
  int tls_result;
  const int cert_type_priority[3] = { GNUTLS_CRT_X509,
    GNUTLS_CRT_OPENPGP, 0
  };
#endif

  int priority;
  int priority_requested = 0;
  int tos;
  int tos_requested = 0;
  char *arg_end, *p;

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
  stddev = null_timeval;
  strcpy (port_name, ECHO_TCP_PORT);

  for (i = 0; i <= MAXNUMBER; i++)
    {
      results[i].valid = 0;
    }
  progname = argv[0];
  while ((result =
	  getopt (argc, argv, "vs:n:w:dch:i:rut:f:SCp:P:aA46")) != -1)
    {
      switch ((char) result)
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
	  strcpy (port_name, DISCARD_TCP_PORT);
	  port_to_use = USE_DISCARD;
	  discard = 1;
	  break;
	case 'c':
	  strcpy (port_name, CHARACTER_GENERATOR_TCP_PORT);
	  port_to_use = USE_CHARGEN;
	  stop_at_newlines = 0;
	  break;
	case 'i':
	  strcpy (port_name, DEFAULT_ICP_UDP_PORT);
	  port_to_use = USE_ICP;
	  udp = 1;
	  icp = 1;
	  url = optarg;
	  break;
	case 'h':
	  strcpy (port_name, DEFAULT_HTTP_TCP_PORT);
	  port_to_use = USE_HTTP;
	  http = 1;
	  url = optarg;
	  break;
	case 'a':
	  nocache = 1;
	  break;
	case 'A':
	  nocache = 2;
	  break;
	case 'f':
	  fill = *optarg;
	  fill_requested = 1;
	  break;
	case 'S':
	  strcpy (port_name, "smtp");
	  port_to_use = USE_SMTP;
	  smtp = 1;
	  break;
	case 'p':
	  priority = (int) strtol (optarg, &arg_end, 0);
	  if (arg_end == optarg || arg_end == '\0')
	    {
	      (void) fprintf (stderr,
			      "%s: socket priority (-p) should be numeric.\n",
			      progname);
	      exit (1);
	    }
	  else
	    {
	      priority_requested = 1;
	    }
	  break;
	case 'P':
	  tos = (int) strtol (optarg, &arg_end, 0);
	  if (arg_end == optarg || arg_end == '\0')
	    {
	      (void) fprintf (stderr,
			      "%s: IP type of service (-P) should be "
			      "numeric.\n", progname);
	      exit (1);
	    }
	  else
	    {
	      tos_requested = 1;
	    }
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
#ifdef HAVE_USLEEP
	  wait = atof (optarg);
#else
	  wait = atoi (optarg);
#endif
	  if (wait <= 0)
	    /* atoi returns zero when there is an error. So we cannot use 
	       '-w 0' to specify no waiting. */
	    {
	      (void) fprintf (stderr,
			      "%s: illegal waiting time.\n", progname);
	      exit (1);
	    }
	  break;
	case '4':
	  family = AF_INET;
	  break;
	case '6':
	  family = AF_INET6;
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
#if ! (defined(OPENSSL) || defined(GNUTLS))
  if (ssl)
    {
      (void) fprintf (stderr,
		      "%s: not compiled with SSL/TLS support.\n", progname);
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
      strcpy (port_name, DEFAULT_HTTPS_TCP_PORT);
    }
#ifndef USE_TOS
  if (tos_requested)
    {
      (void) fprintf (stderr,
		      "%s: Not compiled with Type Of Service support.\n",
		      progname);
      exit (1);
    }
#endif
#ifndef USE_PRIORITY
  if (priority_requested)
    {
      (void) fprintf (stderr,
		      "%s: Not compiled with socket priority support.\n",
		      progname);
      exit (1);
    }
#endif
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
#ifdef LIBIDN
  locale_server = server;
  utf8_server = stringprep_locale_to_utf8 (server);
  if (utf8_server)
    server = utf8_server;
  else
    err_quit ("Cannot convert %s to UTF-8 encoding: wrong locale (%s)?",
	      server, stringprep_locale_charset ());
#endif
  if (!http && !icp)
    {
      for (p = server; *p && (*p != ':'); p++)
	{
	}
      if (*p && (*p == ':'))
	{
	  (void) fprintf (stderr,
			  "%s: The syntax hostname:port is only for HTTP or ICP\n",
			  progname);
	  exit (1);
	}
    }
  signal (SIGINT, interrupted);
  memset (&hints, 0, sizeof (hints));
  hints.ai_family = family;
  hints.ai_socktype = udp ? SOCK_DGRAM : SOCK_STREAM;

#ifdef HTTP
  if (http || icp)
    {
      char *text_port = NULL;

      if (*server == '[')
	{			/* RFC 2732 */
	  server++;
	  for (p = server; *p && *p != ']'; p++)
	    {
	    }
	  p++;
	  if (*p == ':')
	    {
	      p--;
	      *p = 0;
	      text_port = p + 2;
	      strncpy (port_name, text_port, NI_MAXSERV);
	    }
	  else
	    {
	      p--;
	      if (*p == ']')
		{
		  /* No port number */
		  *p = 0;
		}
	      else
		{
		  (void) fprintf (stderr,
				  "%s: Invalid syntax for IPv6 address.\n",
				  progname);
		  exit (1);
		}
	    }
	}
      else
	{
	  for (p = server; *p; p++)
	    {
	      if (*p == ':')
		{
		  *p = 0;
		  text_port = p + 1;
		  strncpy (port_name, text_port, NI_MAXSERV);
		}
	    }
	}
      if (text_port == NULL)
	{
	  error = getaddrinfo (server, port_name, &hints, &res);
	  if (error)
	    {
	      if (error == EAI_SERVICE)
		{
		  if (strcmp (port_name, DEFAULT_HTTP_TCP_PORT) == 0)
		    {
		      strcpy (port_name, "80");
		    }
		  else if (strcmp (port_name, DEFAULT_HTTPS_TCP_PORT) == 0)
		    {
		      strcpy (port_name, "443");
		    }
		  else if (strcmp (port_name, DEFAULT_ICP_UDP_PORT) == 0)
		    {
		      strcpy (port_name, "3130");
		    }
		}
	    }
	}
    }
#endif

#ifdef LIBIDN
  /* Check if it is an address or a name (libidn will have trouble with 
     IPv6 addresses otherwise) */
  memset (&hints_numeric, 0, sizeof (hints_numeric));
  hints_numeric.ai_family = family;
  hints_numeric.ai_flags = AI_NUMERICHOST;
  error = getaddrinfo (server, port_name, &hints_numeric, &res);
  if (error && error == EAI_NONAME)	/* A name, not an address */
    {
      if ((result =
	   idna_to_ascii_8z (utf8_server, &ace_server,
			     IDNA_USE_STD3_ASCII_RULES)) != IDNA_SUCCESS)
	{
	  if (result == IDNA_CONTAINS_LDH) 
	    err_quit ("Illegal name for host: %s", server); /* foo@bar or 
                                                               similar errors */
	  else
	    err_quit ("IDN error for host: %s %d", server, result);
	}
      if (strcmp (utf8_server, ace_server))
	{
	  if (verbose)
	    printf ("ACE name of the server: %s\n", ace_server);
	  server = ace_server;
	}
    }				/* Else it is an address, do not IDNize it */
#endif
  error = getaddrinfo (server, port_name, &hints, &res);
  if (error)
    {
      err_quit ("getaddrinfo error for host: %s %s",
		server, gai_strerror (error));
    }

  if (getnameinfo (res->ai_addr, res->ai_addrlen, hbuf, sizeof (hbuf),
		   pbuf, sizeof (pbuf), niflags) != 0)
    {
      strcpy (hbuf, "?");
      strcpy (pbuf, "?");
    }

  /*
   * Fill in the structure "serv_addr" with the address of the server
   * that we want to connect with.
   */

#ifdef HTTP
  if (http)
    {
      sendline = make_http_sendline (url, server, atoi (pbuf), nocache);
      /* printf ("DEBUG: sending %s\n", sendline); */
    }
  else
#endif
#ifdef SMTP
  if (smtp)
    {
      sendline = "QUIT\r\n";	/* Surprises some SMTP servers which log
				   a frightening NOQUEUE. Anyone knows
				   better? */
    }
  else
#endif
#ifdef ICP
  if (icp)
    {
      if (res->ai_family == AF_INET)
	{
	  sendline =
	    make_icp_sendline (url, &(res->ai_addr), opcode, &length);
	}
      else
	{

	  /* TODO: we should be able to create a ICP hostid for IPv6 addresses... 
	     See the Squid IPv6 patch at 
	     http://devel.squid-cache.org/projects.html#ipv6, for instance the 
	     following code. */
	  sendline = make_icp_sendline (url, (void *) NULL, opcode, &length);
	  /*
	     -    headerp->shostid = theOutICPAddr.s_addr;
	     +    ** FIXME ** we should get more unique data from IPv6 address 
	     +xmemcpy (&headerp->shostid, &theOutICPAddr,
	     sizeof (headerp->shostid));
	   */
	}
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
      RAND_file_name (rand_file, sizeof (rand_file));
      RAND_write_file (rand_file);
      RAND_load_file (rand_file, 1024);
      meth = SSLv23_client_method ();
      if ((ctx = SSL_CTX_new (meth)) == NULL)
	err_sys ("Cannot create a new SSL context");
    }
#endif
#ifdef GNUTLS
  if (ssl)
    {
      gnutls_global_init ();
      gnutls_certificate_allocate_credentials (&xcred);
      /* gnutls_certificate_set_x509_trust_file(xcred, CAFILE, GNUTLS_X509_FMT_PEM); */
    }
#endif

  for (i = 1; i <= number; i++)
    {
      if (i > 1)
	{
#ifdef HAVE_USLEEP
	  usleep (wait * 1000000);
#else
	  sleep (wait);
#endif
	}
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
      /*
       * Open a socket.
       */
      if ((sockfd =
	   socket (res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
	err_sys ("Can't open socket");
      if (udp)
	{
	  struct addrinfo hints2, *res2;

	  memset (&hints2, 0, sizeof (hints2));
	  hints2.ai_family = res->ai_family;
	  hints2.ai_flags = AI_PASSIVE;
	  hints2.ai_socktype = SOCK_DGRAM;
	  error = getaddrinfo (NULL, "0", &hints2, &res2);
	  if (error)
	    {
	      err_sys ("getaddrinfo error");
	    }
	  if (bind (sockfd, res2->ai_addr, res2->ai_addrlen) < 0)
	    {
	      err_sys ("bind error");
	    }
	}
#ifdef USE_PRIORITY
      if (priority_requested)
	{
	  if (verbose)
	    {
	      printf ("Setting socket priority to %d (0x%02x)\n",
		      priority, (unsigned int) priority);
	    }
	  if (setsockopt (sockfd,
			  SOL_SOCKET,
			  SO_PRIORITY,
			  (void *) &priority, (socklen_t) sizeof (priority)))
	    {
	      err_sys ("Failed setting socket priority");
	    }
	}
#endif
#if USE_TOS
      if (tos_requested)
	{
	  if (verbose)
	    {
	      printf ("Setting IP type of service octet to %d (0x%02x)\n",
		      tos, (unsigned int) tos);
	    }
	  if (setsockopt (sockfd,
			  SOL_IP,
			  IP_TOS, (void *) &tos, (socklen_t) sizeof (tos)))
	    {
	      err_sys ("Failed setting IP type of service octet");
	    }
	}
#endif
      if (verbose)
	{
	  if (tcp)
	    {
	      printf
		("Trying to connect to internet address %s %s to transmit %u bytes...\n",
		 hbuf, pbuf, n);
	    }
#ifdef ICP
	  if (icp)
	    {
	      printf
		("Trying to send an ICP packet of %u bytes to the internet address %s...\n",
		 length, hbuf);
	    }
#endif
	  else
	    {
	      printf ("Trying to send %u bytes to internet address %s...\n",
		      size, hbuf);
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
	  (void) gettimeofday (&conntv, (struct timezone *) NULL);
	  if (connect (sockfd, res->ai_addr, res->ai_addrlen) < 0)
	    {
	      if ((errno == EINTR) && (timeout_flag))
		{
		  printf ("Timeout while connecting\n");
		  close (sockfd);
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
	  else
	    {
	      if (tcp)
		{
		  (void) gettimeofday (&connectedtv,
				       (struct timezone *) NULL);
		  temp = connectedtv;
		  tvsub (&temp, &conntv);
		  if (verbose)
		    {
		      printf ("Connected...\n");
		      printf ("TCP Latency: %d.%06d seconds\n",
			      (int) temp.tv_sec, (int) temp.tv_usec);
		    }
		}
	    }

	  if (verbose && tcp)
	    {
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
		    close (sockfd);
		    continue;
		  }
	      if (verbose)
		printf ("SSL connection using %s\n", SSL_get_cipher (sslh));
	      /* We could check the server's certificate or other funny
	         things */
	    }
#endif
#ifdef GNUTLS
	  if (ssl)
	    {
	      tls_result = gnutls_init (&session, GNUTLS_CLIENT);
	      if (tls_result != 0)
		err_sys ("Cannot create a new TLS session");
	      gnutls_set_default_priority (session);
	      gnutls_certificate_type_set_priority (session,
						    cert_type_priority);
	      gnutls_credentials_set (session, GNUTLS_CRD_CERTIFICATE, xcred);
	      gnutls_transport_set_ptr (session,
					(gnutls_transport_ptr) sockfd);
	      tls_result = gnutls_handshake (session);
	      if (tls_result < 0)
		{
		  if ((errno == EINTR) && (timeout_flag))
		    {
		      printf ("Timeout while starting TLS\n");
		      close (sockfd);
		      continue;
		    }
		  else
		    {
		      err_sys ("Cannot start the TLS session: %s",
			       gnutls_strerror (tls_result));
		    }
		}
	      if (verbose)
		printf ("TLS connection using \"%s\"\n",
			gnutls_cipher_get_name (gnutls_cipher_get (session)));
	      /* We could check the server's certificate or other funny
	         things. See http://www.gnu.org/software/gnutls/documentation/gnutls/gnutls.html#SECTION00622000000000000000 */
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
			  res->ai_addr, res->ai_addrlen) != n)
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
			  close (sockfd);
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
			  close (sockfd);
			  continue;
			}
		      else
			{
			  sslcode = ERR_get_error ();
			  err_sys ("SSL_write error on socket: %s",
				   ERR_error_string (sslcode, NULL));
			}
		    }
		  /* printf ("DEBUG: writing %s with SSL\n", sendline); */
		}
#endif
#ifdef GNUTLS
	      else
	      {
		if ((rc =
		     gnutls_record_send (session, sendline,
					 strlen (sendline))) != n)
		  {
		    if ((nr < 0 || nr != n) && timeout_flag)
		      {
			nr = n;
			printf ("Timeout while writing\n");
			close (sockfd);
			continue;
		      }
		    else
		      {
			err_sys ("gnutls_record_send error %d on socket: %s",
				 rc, gnutls_strerror (rc));
		      }
		  }
		/* printf ("DEBUG: writing %s with TLS\n", sendline); */
	      }
#endif
	      /* Write something to the server */
	      if (writen (sockfd, sendline, n) != n)
		{
		  if ((nr < 0 || nr != n) && timeout_flag)
		    {
		      nr = n;
		      printf ("Timeout while writing\n");
		      close (sockfd);
		      continue;
		    }
		  else
		    err_sys ("writen error on socket");
		}
	    }
	  else
	    {
#ifdef ICP
	      if (icp)
		{
		  if (sendto (sockfd, sendline, length, 0,
			      res->ai_addr, res->ai_addrlen) != length)
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
	      (void) gettimeofday (&sendtv, (struct timezone *) NULL);
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

      if (tcp && !discard)
	{
	  fd_set mask;
	  int n = 0;

	  FD_ZERO (&mask);

	  if (!(http && ssl))
	    n = fileno (files);
#ifdef OPENSSL
	  else
	    {
	      n = SSL_get_fd (sslh);
	    }
#endif
#ifdef GNUTLS
	  else
	  {
	    n = sockfd;
	  }
#endif
	  FD_SET (n, &mask);
	  if (select (n + 1, &mask, 0, 0, NULL) > 0)
	    {
	      (void) gettimeofday (&recvtv, (struct timezone *) NULL);
	      temp = recvtv;
	      tvsub (&temp, &sendtv);
	      if (verbose)
		printf ("Application Latency: %d.%06d seconds\n",
			(int) temp.tv_sec, (int) temp.tv_usec);
	    }

	}

      if ((port_to_use == USE_ECHO) || (port_to_use == USE_CHARGEN) ||
	  (port_to_use == USE_HTTP) || (port_to_use == USE_ICP) ||
	  (port_to_use == USE_SMTP))
	{
	  if (!udp)
	    {
	      if (!http && !smtp && !discard)
		{
		  /* Read from the server */
		  nr = readline (files, recvline, n, stop_at_newlines);
		}
	      else if (discard)
		{
		  /* No reply, no read */
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
#ifdef GNUTLS
		  else
		  channel.tls = session;
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
	  if (!http && !icp && !smtp && !discard)
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
		  close (sockfd);
		  continue;
		}
	      if (nr < 0 || nr != n)
		err_sys ("readline error: %d bytes read, %d bytes requested",
			 nr, n);
	    }
	  else
	    /* This is probably HTTP */
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
		  close (sockfd);
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
      if (tcp)
	alarm (0);
      if (http)
	{
#ifdef OPENSSL
	  if (ssl)
	    SSL_shutdown (channel.ssl);
	  else
#endif
#ifdef GNUTLS
	  if (ssl)
	    shutdown (sockfd, SHUT_RDWR);
	  else
#endif
	    fclose (channel.fs);
	}
      close (sockfd);

      (void) gettimeofday (&newtv, (struct timezone *) NULL);
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
		  /* TODO: it does not work if the size is lower than the
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
#ifdef GNUTLS
	  if (ssl)
	    {
	      gnutls_bye (channel.tls, GNUTLS_SHUT_RDWR);
	      gnutls_deinit (session);
	      /* gnutls_certificate_free_credentials(xcred); */
	    }
#endif
	}
    }				/* End of main loop */
  printstats ();
  if (successes >= 1)
    exit (0);
  else
    exit (1);
  /* It would be nice to clean here (OpenSSL, etc) */
#ifdef GNUTLS
  if (ssl)
    {
      gnutls_global_deinit ();
    }
#endif
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
      tvstddev (&stddev, successes, total, results);
      printf ("Standard deviation: %d.%06d\n",
	      (int) stddev.tv_sec, (int) stddev.tv_usec);
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
      printf ("Median time: %d.%06d seconds (%.0f bytes per sec.)\n",
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

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

char           *progname;

#include	"echoping.h"

/*
 * An option to define only if you want to drive echoping from another
 * process. Useless but harmless otherwise. In practice, while OSF/1 is happy
 * with it, SunOS refuses to use fflush on a NULL and Linux fails.
 */
#undef FLUSH_OUTPUT             /* Not really supported, see the TODO */

/* Global variables for main and printstats */

int             return_code = 0;
int             rc;
unsigned int    number = 1;
struct timeval  max, min, total, median, stddev, temp, measured;
struct timeval  conntv, connectedtv, sendtv, recvtv;
unsigned int    successes, attempts = 0;
unsigned int    size = DEFLINE;
unsigned int    j = 0;
int             n_stddev = 0;

int             family = AF_UNSPEC;

struct result   results[MAX_ITERATIONS];
struct timeval  good_results[MAX_ITERATIONS];
extern int      tvcmp();

int
main(argc, argv)
    int             argc;
    const char     *argv[];
{

    int             result;
    int             remaining = argc;
    char          **leftover;

    int             sockfd = -1;
    struct addrinfo hints, *res;
#ifdef LIBIDN
    struct addrinfo hints_numeric;
#endif
    int             error;
    char            hbuf[NI_MAXHOST], pbuf[NI_MAXSERV];
#ifdef NI_WITHSCOPEID
    const int       niflags = NI_NUMERICHOST | NI_NUMERICSERV | NI_WITHSCOPEID;
#else
    const int       niflags = NI_NUMERICHOST | NI_NUMERICSERV;
#endif

    FILE           *files = NULL;
    CHANNEL         channel;
    int             verbose = FALSE;
    int             dump_config = FALSE;
    int             module_find = FALSE;
    int             n, nr = 0;
#ifdef OPENSSL
    int             sslcode;
    char            rand_file[MAX_LINE];
#endif
    char           *sendline, recvline[MAX_LINE + 1];
    boolean         accept_http_redirects = FALSE;
    char           *http_hostname = NULL;
#ifdef ICP
    char            retcode[DEFLINE];
    int             length;
#endif
    struct timeval  newtv, oldtv;
    void            printstats();

#ifdef HAVE_USLEEP
    float           wait = 1.0;
#else
    unsigned int    wait = 1;
#endif
    unsigned char   fill = ' ';
    char           *fill_s;
    boolean         fill_requested = FALSE;
    unsigned int    i = 0;
    char           *plugin_name = NULL;
    char           *complete_plugin_name = NULL;
    char           *ext;
    void           *plugin = NULL;
    int             plugin_result = -3; /* Initialize to illegal value */

    void            to_alarm(); /* our alarm() signal handler */
    void            interrupted();
    unsigned int    timeout = 10;
    boolean         timeout_requested = 0;
    boolean         size_requested = 0;
    char           *url = "";
    boolean         measure_data_transfer_only = FALSE;
#if USE_SIGACTION
    struct sigaction mysigaction;
#endif

    char           *plugin_port_name, *port_name;
    boolean         plugin_raw = FALSE;
    boolean         port_to_use = USE_ECHO;
    boolean         http = 0;
    boolean         smtp = 0;
    boolean         discard = 0;
    boolean         chargen = 0;
    boolean         udp = 0;
    boolean         icp = 0;

    boolean         nocache = 0;

#ifdef ICP
    icp_opcode      opcode = ICP_OP_QUERY;
#endif

    boolean         tcp = FALSE;
    boolean         ssl = FALSE;

    boolean         stop_at_newlines = 1;

#ifdef OPENSSL
    SSL_METHOD     *meth;
    SSL_CTX        *ctx = NULL;
    SSL            *sslh = NULL;
#endif
#ifdef GNUTLS
    gnutls_session  session;
    gnutls_certificate_credentials xcred;
    int             tls_result;
    const int       cert_type_priority[3] = { GNUTLS_CRT_X509,
        GNUTLS_CRT_OPENPGP, 0
    };
#endif

    int             priority;
    int             priority_requested = 0;
    int             tos;
    int             tos_requested = 0;
    int             protocol;
    boolean         sctp_requested = FALSE;
    int             sctp = 0;
#ifdef HAVE_TCP_INFO
    struct tcp_info tcpinfo;
    socklen_t       socket_length = sizeof(tcpinfo);
#endif
    char           *p;
    echoping_options global_options;

    /* popt variables */
    const struct poptOption options[] = {
        {"verbose", 'v', POPT_ARG_NONE, &verbose, 'v'},
        {"dump-configuration", 'V', POPT_ARG_NONE, &dump_config, 'V',
         "Displays echoping compiled-in configuration"},
        {"help", '?', POPT_ARG_NONE, NULL, '?'},
        {"size", 's', POPT_ARG_INT, &size, 's'},
        {"number", 'n', POPT_ARG_INT, &number, 'n', "Number of iterations"},
#ifdef HAVE_USLEEP
        {"wait", 'w', POPT_ARG_FLOAT, &wait, 'w',
         "Delay between iterations"},
#else
        {"wait", 'w', POPT_ARG_INT, &wait, 'w', "Delay between iterations"},
#endif
        {"discard", 'd', POPT_ARG_NONE, &discard, 'd'},
        {"chargen", 'c', POPT_ARG_NONE, &chargen, 'c'},
        {"http", 'h', POPT_ARG_STRING, &url, 'h'},
        {"accept-http-redirects", 'R', POPT_ARG_NONE, &accept_http_redirects,
         'R',
         "Accept HTTP return codes 3xx (redirections)"},
        {"hostname", 'H', POPT_ARG_STRING, &http_hostname, 'H',
         "Hostname to use in HTTP Host: header"},
        {"icp", 'i', POPT_ARG_STRING, &url, 'i',
         "ICP protocol, for Web proxies/caches"},
        {"udp", 'u', POPT_ARG_NONE, &udp, 'u'},
        {"timeout", 't', POPT_ARG_INT, &timeout, 't'},
        {"fill", 'f', POPT_ARG_STRING, &fill_s, 'f'},
        {"smtp", 'S', POPT_ARG_NONE, &smtp, 'S'},
        {"ssl", 'C', POPT_ARG_NONE, &ssl, 'C'},
        {"priority", 'p', POPT_ARG_INT, &priority, 'p'},
        {"type-of-service", 'P', POPT_ARG_INT, &tos, 'P'},
        {"sctp", 'T', POPT_ARG_NONE, &sctp, 'T'},
        {"check-original", 'a', POPT_ARG_NONE, NULL, 'a',
         "For HTTP through a proxy/cache"},
        {"ignore-cache", 'A', POPT_ARG_NONE, NULL, 'A',
         "For HTTP through a proxy/cache"},
        {"ipv4", '4', POPT_ARG_NONE, NULL, '4'},
        {"ipv6", '6', POPT_ARG_NONE, NULL, '6'},
        {"module", 'm', POPT_ARG_STRING, &plugin_name, 'm',
         "Loads the given plugin"},
        {"data-only", 'D', POPT_ARG_NONE, NULL, 'D'},
        {"num-std-dev", 'N', POPT_ARG_INT, &n_stddev, 'N',
         "Number of standard deviations to classify outliers"},
        POPT_TABLEEND
    };
    poptContext     poptcon;

    global_options.udp = FALSE;
    global_options.verbose = FALSE;

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
    port_name = malloc(NI_MAXSERV);
    strcpy(port_name, ECHO_TCP_PORT);

    for (i = 0; i <= MAX_ITERATIONS; i++) {
        results[i].valid = 0;
    }
    progname = (char *) argv[0];

    poptcon = poptGetContext(NULL, argc, argv, options, POPT_CONTEXT_POSIXMEHARDER);

    while ((result = poptGetNextOpt(poptcon)) != -1) {
        if (result < -1) {
            fprintf(stderr, "%s: %s\n",
                    poptBadOption(poptcon, POPT_BADOPTION_NOALIAS),
                    poptStrerror(result));
            usage(poptcon);
        }
        remaining--;
        switch ((char) result) {
        case '?':
            poptPrintHelp(poptcon, stdout, 0);
            fprintf(stdout, " hostname [plugin-options...]\n");
            fprintf(stdout,
                    "  (You can get a list of available plugins with \"ls %s\")\n",
                    PLUGINS_DIR);
            exit(0);
        case 'V':
            printf("%s\n", COMPILATION_OPTIONS);
            exit(0);
        case 'v':
            break;
        case 'r':
            break;
        case 'u':
            break;
        case 'C':
            break;
        case 'd':
            strcpy(port_name, DISCARD_TCP_PORT);
            port_to_use = USE_DISCARD;
            break;
        case 'c':
            strcpy(port_name, CHARACTER_GENERATOR_TCP_PORT);
            port_to_use = USE_CHARGEN;
            stop_at_newlines = 0;
            break;
        case 'i':
            remaining--;
            strcpy(port_name, DEFAULT_ICP_UDP_PORT);
            port_to_use = USE_ICP;
            udp = 1;
            icp = 1;
            break;
        case 'h':
            remaining--;
            strcpy(port_name, DEFAULT_HTTP_TCP_PORT);
            port_to_use = USE_HTTP;
            http = 1;
            break;
        case 'R':
            accept_http_redirects = TRUE;
            break;
        case 'H':
            remaining--;
            break;
        case 'a':
            nocache = 1;
            break;
        case 'A':
            nocache = 2;
            break;
        case 'f':
            remaining--;
            if (strlen(fill_s) > 1)
                err_quit("Argument --fill should be a one-character string");
            fill = fill_s[0];
            fill_requested = 1;
            break;
        case 'S':
            strcpy(port_name, "smtp");
            port_to_use = USE_SMTP;
            break;
        case 'D':
            measure_data_transfer_only = TRUE;
            break;
        case 'N':
            remaining--;
            break;
        case 'p':
            remaining--;
            priority_requested = 1;
            break;
        case 'P':
            remaining--;
            tos_requested = 1;
            break;
        case 'T':
            sctp_requested = TRUE;
            break;
        case 's':
            remaining--;
            if (size > MAX_LINE) {
                (void) fprintf(stderr,
                               "%s: packet size too large, max is %d.\n",
                               progname, MAX_LINE);
                exit(1);
            }
            if (size <= 0) {
                (void) fprintf(stderr, "%s: illegal packet size.\n", progname);
                exit(1);
            }
            size_requested = 1;
            break;
        case 't':
            remaining--;
            timeout_requested = 1;
            if (size <= 0) {
                (void) fprintf(stderr, "%s: illegal timeout.\n", progname);
                exit(1);
            }
            break;
        case 'n':
            remaining--;
            if (number > MAX_ITERATIONS) {
                (void) fprintf(stderr,
                               "%s: number of iterations too large, max is %d.\n",
                               progname, MAX_ITERATIONS);
                exit(1);
            }
            if (number <= 0) {
                (void) fprintf(stderr,
                               "%s: illegal number of iterations.\n", progname);
                exit(1);
            }
            break;
        case 'w':
            remaining--;
            if (wait <= 0)
                /* 
                 * atoi returns zero when there is an error.
                 * So we cannot use '-w 0' to specify no
                 * waiting.
                 */
            {
                (void) fprintf(stderr, "%s: illegal waiting time.\n", progname);
                exit(1);
            }
            break;
        case '4':
            family = AF_INET;
            break;
        case '6':
            family = AF_INET6;
            break;
        case 'm':
            remaining--;
            module_find = TRUE;
            break;
        default:
            printf("Unknown character option %d (%c)", result, (char) result);
            usage(poptcon);
        }
    }
    if (udp && ((port_to_use == USE_CHARGEN) ||
                (port_to_use == USE_HTTP) || (port_to_use == USE_SMTP))) {
        (void) fprintf(stderr,
                       "%s: I don't know how to use this port with UDP.\n",
                       progname);
        exit(1);
    }
    if ((http || smtp) && (fill_requested)) {
        (void) fprintf(stderr,
                       "%s: Filling incompatible with HTTP connections.\n",
                       progname);
        exit(1);
    }
    if (n_stddev && number <= 2) {
        (void) fprintf(stderr,
                       "%s: Average and standard deviation are meaningless since you perform only two or less iteration(s).\n",
                       progname);
        exit(1);
    }
#if ! (defined(OPENSSL) || defined(GNUTLS))
    if (ssl) {
        (void) fprintf(stderr, "%s: not compiled with SSL/TLS support.\n", progname);
        exit(1);
    }
#endif
#ifndef HTTP
    if (http) {
        (void) fprintf(stderr, "%s: Not compiled with HTTP support.\n", progname);
        exit(1);
    }
#endif
#ifndef SMTP
    if (smtp) {
        (void) fprintf(stderr, "%s: Not compiled with SMTP support.\n", progname);
        exit(1);
    }
#endif
#ifndef ICP
    if (icp) {
        (void) fprintf(stderr, "%s: Not compiled with ICP support.\n", progname);
        exit(1);
    }
#endif
    if ((http || smtp) && size_requested) {
        (void) fprintf(stderr,
                       "%s: %s and message size specification are incompatible.\n",
                       progname, http ? "HTTP" : "SMTP");
        exit(1);
    }
    if (ssl && !http) {
        (void) fprintf(stderr,
                       "%s: SSL is only supported for HTTP requests.\n", progname);
        exit(1);
    }
    if (!udp && !icp)
        tcp = TRUE;
    if (ssl && http) {
        strcpy(port_name, DEFAULT_HTTPS_TCP_PORT);
    }
    if (!http && accept_http_redirects) {
        (void) fprintf(stderr,
                       "%s: accept-http-redirects does not make sens if you do not use HTTP.\n",
                       progname);
        exit(1);
    }
#ifndef USE_TOS
    if (tos_requested) {
        (void) fprintf(stderr,
                       "%s: Not compiled with Type Of Service support.\n", progname);
        exit(1);
    }
#endif
#ifndef USE_PRIORITY
    if (priority_requested) {
        (void) fprintf(stderr,
                       "%s: Not compiled with socket priority support.\n", progname);
        exit(1);
    }
#endif
#ifndef HAVE_SCTP
    if (sctp_requested) {
        (void) fprintf(stderr, "%s: Not compiled with SCTP support.\n", progname);
        exit(1);
    }
#endif
    remaining--;                /* No argv[0] this time */
    leftover = (char **) &argv[argc - remaining];
    if (plugin_name) {
        ext = strstr(plugin_name, ".so");
        if ((ext == NULL) || (strcmp(ext, ".so") != 0))
            sprintf(plugin_name, "%s.so", plugin_name);
        plugin = dlopen(plugin_name, RTLD_NOW);
        if (!plugin) {
            /* Retries with the absolute name */
            complete_plugin_name = (char *) malloc(MAX_LINE);
            sprintf(complete_plugin_name, "%s/%s", PLUGINS_DIR, plugin_name);
            plugin = dlopen(complete_plugin_name, RTLD_NOW);
        }
        if (!plugin) {
#if DEBIAN
            /* A bit of help for the poor user */
            fprintf(stderr,
                    "You may have to load the recommended packages "
                    "to run this plugin.\n"
                    "'dpkg -s echoping | grep Recommends' to know them.\n\n");
#endif
            err_sys
                ("Cannot load \"%s\" (I tried the short name, then the complete name in \"%s\"): %s",
                 plugin_name, PLUGINS_DIR, dlerror());
        }
        plugin_init = dlsym(plugin, "init");
        if (!plugin_init) {
            err_sys("Cannot find init in %s: %s", plugin_name, dlerror());
        }
        global_options.udp = udp;
        global_options.verbose = verbose;
        if (family == AF_INET)
            global_options.only_ipv4 = 1;
        else
            global_options.only_ipv4 = 0;
        if (family == AF_INET6)
            global_options.only_ipv6 = 1;
        else
            global_options.only_ipv6 = 0;
        plugin_port_name =
            plugin_init(remaining, (const char **) leftover, global_options);
        if (plugin_port_name != NULL) {
            strcpy(port_name, plugin_port_name);
            plugin_raw = FALSE;
            plugin_start = dlsym(plugin, "start");
            if (!plugin_start) {
                err_sys("Cannot find start in %s: %s", plugin_name, dlerror());
            }
        } else {
            port_name = 0;
            plugin_raw = TRUE;
            plugin_raw_start = dlsym(plugin, "start_raw");
            if (!plugin_raw_start) {
                err_sys("Cannot find start_raw in %s: %s", plugin_name, dlerror());
            }
        }
        plugin_execute = dlsym(plugin, "execute");
        if (!plugin_execute) {
            err_sys("Cannot find execute in %s: %s", plugin_name, dlerror());
        }
        plugin_terminate = dlsym(plugin, "terminate");
        if (!plugin_terminate) {
            err_sys("Cannot find terminate in %s: %s", plugin_name, dlerror());
        }
    }
    if (remaining == 0) {
        (void) fprintf(stderr, "No host name indicated\n");
        usage(poptcon);
    }
    if (!module_find && remaining != 1) {
        printf("%d args remaining, should be 1\n", remaining);
        usage(poptcon);
    }
    if (verbose) {
        printf("\nThis is %s, version %s.\n\n", progname, VERSION);
    }
    server = leftover[0];
#ifdef LIBIDN
    locale_server = server;
    utf8_server = stringprep_locale_to_utf8(server);
    if (utf8_server)
        server = utf8_server;
    else
        err_quit("Cannot convert %s to UTF-8 encoding: wrong locale (%s)?",
                 server, stringprep_locale_charset());
#endif
    if (!http && !icp) {
        for (p = server; *p && (*p != ':'); p++) {
        }
        if (*p && (*p == ':')) {
            (void) fprintf(stderr,
                           "%s: The syntax hostname:port is only for HTTP or ICP\n",
                           progname);
            exit(1);
        }
    }
    signal(SIGINT, interrupted);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = family;
    hints.ai_socktype = udp ? SOCK_DGRAM : SOCK_STREAM;

#ifdef HTTP
    if (http || icp) {
        char           *text_port = NULL;

        if (*server == '[') {   /* RFC 3986, section 3.2.2 */
            server++;
            for (p = server; *p && *p != ']'; p++) {
            }
            p++;
            if (*p == ':') {
                p--;
                *p = 0;
                text_port = p + 2;
                strncpy(port_name, text_port, NI_MAXSERV);
            } else {
                p--;
                if (*p == ']') {
                    /* No port number */
                    *p = 0;
                } else {
                    (void) fprintf(stderr,
                                   "%s: Invalid syntax for IPv6 address.\n",
                                   progname);
                    exit(1);
                }
            }
        } else {
            for (p = server; *p; p++) {
                if (*p == ':') {
                    *p = 0;
                    text_port = p + 1;
                    if (strcmp(text_port, ""))  /* See bug * * #850672 */
                        strncpy(port_name, text_port, NI_MAXSERV);
                }
            }
        }
        if (text_port == NULL || !strcmp(text_port, "")) {
            error = getaddrinfo(server, port_name, &hints, &res);
            if (error) {
                if (error == EAI_SERVICE) {
                    if (strcmp(port_name, DEFAULT_HTTP_TCP_PORT)
                        == 0) {
                        strcpy(port_name, "80");
                    } else if (strcmp(port_name, DEFAULT_HTTPS_TCP_PORT) == 0) {
                        strcpy(port_name, "443");
                    } else if (strcmp(port_name, DEFAULT_ICP_UDP_PORT) == 0) {
                        strcpy(port_name, "3130");
                    }
                }
            }
        }
    }
#endif

#ifdef LIBIDN
    /* 
     * Check if it is an address or a name (libidn will have trouble with
     * IPv6 addresses otherwise)
     */
    memset(&hints_numeric, 0, sizeof(hints_numeric));
    hints_numeric.ai_family = family;
    hints_numeric.ai_flags = AI_NUMERICHOST;
    error = getaddrinfo(server, port_name, &hints_numeric, &res);
    if (error && error == EAI_NONAME) { /* A name, not an address */
        if ((result =
             idna_to_ascii_8z(utf8_server, &ace_server,
                              IDNA_USE_STD3_ASCII_RULES)) != IDNA_SUCCESS) {
            if (result == IDNA_CONTAINS_LDH)
                err_quit("Illegal name for host: %s", server);  /* foo@bar or
                                                                 * similar errors */
            else
                err_quit("IDN error for host: %s %d", server, result);
        }
        if (strcmp(utf8_server, ace_server)) {
            if (verbose)
                printf("ACE name of the server: %s\n", ace_server);
            server = ace_server;
        }
    }                           /* Else it is an address, do not IDNize it */
#endif
    if (!plugin || !plugin_raw) {
        error = getaddrinfo(server, port_name, &hints, &res);
        if (error) {
            err_quit("getaddrinfo error for host: %s %s",
                     server, gai_strerror(error));
        }
        if (getnameinfo(res->ai_addr, res->ai_addrlen, hbuf, sizeof(hbuf),
                        pbuf, sizeof(pbuf), niflags) != 0) {
            strcpy(hbuf, "?");
            strcpy(pbuf, "?");
        }
    }
    if (plugin) {
        if (verbose) {
            printf("Running start() for the plugin %s...\n", plugin_name);
        }
        if (plugin_raw)
            plugin_raw_start();
        else
            plugin_start(res);
    }
#ifdef HTTP
    if (http) {
        if (http_hostname != NULL)
            server = http_hostname;
        sendline = make_http_sendline(url, server, atoi(pbuf), nocache);
    } else
#endif
#ifdef SMTP
    if (smtp) {
        sendline = "QUIT\r\n";  /* Surprises some SMTP servers which log a
                                 * frightening NOQUEUE. Anyone knows better? * * * * 
                                 * See bug #1512776 */
    } else
#endif
#ifdef ICP
    if (icp) {
        if (res->ai_family == AF_INET) {
            sendline = make_icp_sendline(url, &(res->ai_addr), opcode, &length);
        } else {

            /* 
             * TODO: we should be able to create a ICP hostid for
             * IPv6 addresses... See the Squid IPv6 patch at
             * http://devel.squid-cache.org/projects.html#ipv6,
             * for instance the following code.
             */
            sendline = make_icp_sendline(url, (void *) NULL, opcode, &length);
            /* 
             * - headerp->shostid = theOutICPAddr.s_addr; + ** FIXME ** we
             * should get more unique data from IPv6 address +xmemcpy
             * (&headerp->shostid, &theOutICPAddr, sizeof
             * (headerp->shostid)); */
        }
    } else
#endif
    if (!fill_requested) {
        sendline = random_string(size);
    } else {
        sendline = (char *) malloc(size + 1);
        for (i = 0; i < size; i++)
            sendline[i] = fill;
        sendline[size] = 0;
    }
    n = strlen(sendline);

#ifdef OPENSSL
    if (ssl) {
        SSL_load_error_strings();
        SSLeay_add_ssl_algorithms();
        /* 
         * The following RAND_ calls are only for systems insecure
         * enough to fail to have /dev/urandom. Bug #132001
         */
        RAND_file_name(rand_file, sizeof(rand_file));
        RAND_write_file(rand_file);
        RAND_load_file(rand_file, 1024);
        meth = SSLv23_client_method();
        if ((ctx = SSL_CTX_new(meth)) == NULL)
            err_sys("Cannot create a new SSL context");
        /* 
         * Bug reported by Sebastian Siewior
         * <bigeasy@foo.fh-furtwangen.de>. It seems that OpenSSL
         * crashes on non blocking sockets when trying to close them
         * (OpenSSL will try write on a closed socket).
         */
#ifdef USE_SIGACTION
        mysigaction.sa_handler = SIG_IGN;
        sigemptyset(&mysigaction.sa_mask);
        if ((sigaction(SIGPIPE, &mysigaction, NULL)) < 0);      /* Ignore it */
#else
        signal(SIGPIPE, SIG_IGN);
#endif
    }
#endif
#ifdef GNUTLS
    if (ssl) {
        gnutls_global_init();
        gnutls_certificate_allocate_credentials(&xcred);
        /* 
         * gnutls_certificate_set_x509_trust_file(xcred, CAFILE,
         * GNUTLS_X509_FMT_PEM);
         */
    }
#endif

    for (i = 1; i <= number; i++) {
        timeout_flag = 0;
        if (timeout_requested)
            alarm(timeout);
        if (i > 1) {
#ifdef HAVE_USLEEP
            /* 
             * SUSv3 states that the argument to usleep() shall
             * be less * than 1000000, so split into two calls if
             * necessary.  Bug #1473872, fix by Jeff Rizzo -
             * riz@sourceforge
             */
            if (wait >= 1) {
                sleep((unsigned int) wait);
            }
            usleep((wait - (unsigned int) wait) * 1000000);
#else
            sleep(wait);
#endif
        }
        attempts++;
#ifdef OPENSSL
        if (ssl)
            /* 
             * Despite what the OpenSSL documentation says, we
             * must allocate a new SSL structure at each
             * iteration, otherwise, some* SSL servers fail at
             * the second iteration with: error:1406D0D9:SSL
             * routines:GET_SERVER_HELLO:reuse cert type not zero
             * Bug #130151
             */
            if ((sslh = SSL_new(ctx)) == NULL)
                err_sys("Cannot initialize SSL context");
#endif
        /* 
         * Open a socket.
         */
        protocol = res->ai_protocol;
#ifdef HAVE_SCTP
        if (sctp)
            protocol = IPPROTO_SCTP;
#endif
        if (!plugin) {
            if ((sockfd = socket(res->ai_family, res->ai_socktype, protocol)) < 0)
                err_sys("Can't open socket");
            if (udp) {
                struct addrinfo hints2, *res2;

                memset(&hints2, 0, sizeof(hints2));
                hints2.ai_family = res->ai_family;
                hints2.ai_flags = AI_PASSIVE;
                hints2.ai_socktype = SOCK_DGRAM;
                error = getaddrinfo(NULL, "0", &hints2, &res2);
                if (error) {
                    err_sys("getaddrinfo error");
                }
                if (bind(sockfd, res2->ai_addr, res2->ai_addrlen) < 0) {
                    err_sys("bind error");
                }
            }
#ifdef USE_PRIORITY
            if (priority_requested) {
                if (verbose) {
                    printf
                        ("Setting socket priority to %d (0x%02x)\n",
                         priority, (unsigned int) priority);
                }
                if (setsockopt(sockfd,
                               SOL_SOCKET,
                               SO_PRIORITY,
                               (void *) &priority, (socklen_t) sizeof(priority))) {
                    err_sys("Failed setting socket priority");
                }
            }
#endif
#if USE_TOS
            if (tos_requested) {
                if (verbose) {
                    printf
                        ("Setting IP type of service octet to %d (0x%02x)\n",
                         tos, (unsigned int) tos);
                }
                if (setsockopt(sockfd,
                               SOL_IP,
                               IP_TOS, (void *) &tos, (socklen_t) sizeof(tos))) {
                    err_sys("Failed setting IP type of service octet");
                }
            }
#endif
        }
        if (verbose) {
            if (!plugin) {
                if (tcp) {
                    printf
                        ("Trying to connect to internet address %s %s to transmit %u bytes...\n",
                         hbuf, pbuf, n);
                }
#ifdef ICP
                if (icp) {
                    printf
                        ("Trying to send an ICP packet of %u bytes to the internet address %s...\n",
                         length, hbuf);
                }
#endif
                else {
                    printf
                        ("Trying to send %u bytes to internet address %s...\n",
                         size, hbuf);
                }
            } else {
                if (plugin_raw)
                    printf("Trying to call plugin %s...\n", plugin_name);
                else
                    printf
                        ("Trying to call plugin %s for internet address %s %s...\n",
                         plugin_name, hbuf, pbuf);
            }
        }
#ifdef FLUSH_OUTPUT
        if (fflush((FILE *) NULL) != 0) {
            err_sys("I cannot flush");
        }
#endif
        if ((tcp || plugin) && timeout_requested) {     /* echoping's timeout has a
                                                         * different semantic in TCP
                                                         * and UDP */
#ifdef USE_SIGACTION
            mysigaction.sa_handler = to_alarm;
            sigemptyset(&mysigaction.sa_mask);
            /* Default behavior doesn't seem portable? */
#ifdef SA_INTERRUPT
            mysigaction.sa_flags = SA_INTERRUPT;
#else
            mysigaction.sa_flags = (int) 0;
#endif
            if ((sigaction(SIGALRM, &mysigaction, NULL)) < 0)
                err_sys("Cannot set signal handler");
#else
            signal(SIGALRM, to_alarm);
#endif
            timeout_flag = 0;   /* for signal handler */
            alarm(timeout);
        }
        (void) gettimeofday(&oldtv, (struct timezone *) NULL);
        /* work out the time it took... */
        if (measure_data_transfer_only) {
            measured = recvtv;
            tvsub(&measured, &sendtv);
        } else {
            measured = newtv;
            tvsub(&measured, &oldtv);
        }
        if (plugin) {
            plugin_result = plugin_execute();
            /* If plugin_result == -1, there is a temporary error and we did not get 
             * data, we must not use it in the average / median calculations. So,
             * successes will not be incremented later. */
            if (plugin_result == -2)
                err_quit("");
        } else {
            if (!icp) {
                /* 
                 * Connect to the server.
                 */
                (void) gettimeofday(&conntv, (struct timezone *) NULL);
                if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
                    if ((errno == EINTR) && (timeout_flag)) {
                        printf("Timeout while connecting\n");
                        close(sockfd);
                        continue;
#ifdef FLUSH_OUTPUT
                        if (fflush((FILE *) NULL) != 0) {
                            err_sys("I cannot flush");
                        }
#endif
                    } else
                        err_sys("Can't connect to server");
                    /* TODO: it would be better to continue: if -n was given, other
                     * iterations may succeed. A flag indicating success or error is 
                     * * probably necessary, it would replace the mess around 'if
                     * (!timeout_flag && (!plugin || plugin_result >= 0))' */
                } else {
                    if (tcp) {
                        (void) gettimeofday(&connectedtv, (struct timezone *) NULL);
                        temp = connectedtv;
                        tvsub(&temp, &conntv);
                        if (verbose) {
                            printf("Connected...\n");
                            printf
                                ("TCP Latency: %d.%06d seconds\n",
                                 (int) temp.tv_sec, (int) temp.tv_usec);
                        }
                    }
                }
                if (verbose && tcp) {
#ifdef FLUSH_OUTPUT
                    if (fflush((FILE *) NULL) != 0) {
                        err_sys("I cannot flush");
                    }
#endif
                }
                if (!udp && !ssl)
                    if ((files = fdopen(sockfd, "r")) == NULL)
                        err_sys("Cannot fdopen");
#ifdef OPENSSL
                if (ssl) {
                    SSL_set_fd(sslh, sockfd);
                    if (SSL_connect(sslh) == -1)
                        if ((errno == EINTR)
                            && (timeout_flag)) {
                            printf("Timeout while starting SSL\n");
                            close(sockfd);
                            continue;
                        }
                    if (verbose)
                        printf("SSL connection using %s\n", SSL_get_cipher(sslh));
                    /* 
                     * We could check the server's
                     * certificate or other funny things
                     */
                }
#endif
#ifdef GNUTLS
                if (ssl) {
                    tls_result = gnutls_init(&session, GNUTLS_CLIENT);
                    if (tls_result != 0)
                        err_sys("Cannot create a new TLS session");
                    gnutls_set_default_priority(session);
                    gnutls_certificate_type_set_priority(session,
                                                         cert_type_priority);
                    gnutls_credentials_set(session, GNUTLS_CRD_CERTIFICATE, xcred);
                    gnutls_transport_set_ptr(session, (gnutls_transport_ptr)
                                             (long) sockfd);
                    tls_result = gnutls_handshake(session);
                    if (tls_result < 0) {
                        if ((errno == EINTR)
                            && (timeout_flag)) {
                            printf("Timeout while starting TLS\n");
                            close(sockfd);
                            continue;
                        } else {
                            err_sys
                                ("Cannot start the TLS session: %s",
                                 gnutls_strerror(tls_result));
                        }
                    }
                    if (verbose)
                        printf
                            ("TLS connection using \"%s\"\n",
                             gnutls_cipher_get_name(gnutls_cipher_get(session)));
                    /* 
                     * We could check the server's
                     * certificate or other funny things.
                     * See
                     * http://www.gnu.org/software/gnutls/
                     * documentation/gnutls/gnutls.html#SE
                     * CTION00622000000000000000
                     */
                }
#endif
            } else {
                /* No initial connection */
            }
            if ((port_to_use == USE_ECHO) || (port_to_use == USE_DISCARD)
                || (port_to_use == USE_HTTP) || (port_to_use == USE_ICP)
                || (port_to_use == USE_SMTP)) {
                if (!udp) {
                    if (!ssl) {
                        /* 
                         * Write something to the
                         * server
                         */
#ifdef SMTP
                        if (port_to_use == USE_SMTP) {
                            /* Get the greeting line */
                            nr = smtp_read_response_from_server(files);
                        }
#endif
                        if (writen(sockfd, sendline, n) != n) {
                            if ((nr < 0 || nr != n)
                                && timeout_flag) {
                                printf
                                    ("Timeout while writing (%d byte(s) written so far)\n",
                                     (nr == -1) ? 0 : nr);
                                nr = n;
                                close(sockfd);
                                continue;
                            } else
                                err_sys("writen error on TCP socket %d", sockfd);
                        }
                    }
#ifdef OPENSSL
                    else {
                        if ((rc = SSL_write(sslh, sendline, n)) != n) {
                            if ((nr < 0 || nr != n)
                                && timeout_flag) {
                                nr = n;
                                printf("Timeout while writing\n");
                                close(sockfd);
                                continue;
                            } else {
                                sslcode = ERR_get_error();
                                err_sys
                                    ("SSL_write error on socket: %s",
                                     ERR_error_string(sslcode, NULL));
                            }
                        }
                    }
#endif
#ifdef GNUTLS
                    else
                    {
                        if ((rc =
                             gnutls_record_send(session, sendline, strlen(sendline)))
                            != n) {
                            if ((nr < 0 || nr != n)
                                && timeout_flag) {
                                nr = n;
                                printf("Timeout while writing\n");
                                close(sockfd);
                                continue;
                            } else {
                                err_sys
                                    ("gnutls_record_send error %d on socket: %s",
                                     rc, gnutls_strerror(rc));
                            }
                        }
                    }
#endif
                } else {
#ifdef ICP
                    if (icp) {
                        if (sendto
                            (sockfd, sendline, length, 0,
                             res->ai_addr, res->ai_addrlen) != length)
                            err_sys("sendto error on socket");
                    } else
#endif
                        /* 
                         * if (sendto(sockfd, sendline, n, 0,
                         * &serv_addr, sizeof(serv_addr)) != n)
                         * err_sys("sendto error on socket");
                         */
                    if (send(sockfd, sendline, n, 0) != n)
                        err_sys("send error on socket");
                }
                if (verbose) {
                    (void) gettimeofday(&sendtv, (struct timezone *)
                                        NULL);
#ifdef ICP
                    if (icp)
                        printf("Sent (%d bytes)...\n", length);
                    else
#endif
                        printf("Sent (%d bytes)...\n", n);

#ifdef FLUSH_OUTPUT
                    if (fflush((FILE *) NULL) != 0) {
                        err_sys("I cannot flush");
                    }
#endif
                }
            }
            if (tcp && !discard) {
                fd_set          mask;
                int             n = 0;

                FD_ZERO(&mask);

                if (!(http && ssl))
                    n = fileno(files);
#ifdef OPENSSL
                else {
                    n = SSL_get_fd(sslh);
                }
#endif
#ifdef GNUTLS
                else
                {
                    n = sockfd;
                }
#endif
                FD_SET(n, &mask);
                if (select(n + 1, &mask, 0, 0, NULL) > 0) {
                    (void) gettimeofday(&recvtv, (struct timezone *)
                                        NULL);
                    temp = recvtv;
                    tvsub(&temp, &sendtv);
                    if (verbose)
                        printf
                            ("Application Latency: %d.%06d seconds\n",
                             (int) temp.tv_sec, (int) temp.tv_usec);
                }
            }
            if ((port_to_use == USE_ECHO) || (port_to_use == USE_CHARGEN)
                || (port_to_use == USE_HTTP) || (port_to_use == USE_ICP)
                || (port_to_use == USE_SMTP)) {
                if (!udp) {
                    if (!http && !smtp && !discard) {
                        /* Read from the server */
                        nr = readline(files, recvline, n, stop_at_newlines);
                    } else if (discard) {
                        /* No reply, no read */
                    }
#ifdef HTTP
                    else if (http) {
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
                        nr = read_from_server(channel, ssl, accept_http_redirects);
                    }
#endif
#ifdef SMTP
                    else if (smtp) {
                        nr = smtp_read_response_from_server(files);
                    }
#endif

                } else {        /* UDP */
#ifdef USE_SIGACTION
                    mysigaction.sa_handler = to_alarm;
                    sigemptyset(&mysigaction.sa_mask);
#ifdef SA_INTERRUPT
                    mysigaction.sa_flags = SA_INTERRUPT;
#else
                    mysigaction.sa_flags = (int) 0;
#endif
                    if ((sigaction(SIGALRM, &mysigaction, NULL))
                        < 0)
                        err_sys("Cannot set signal handler");
#else
                    signal(SIGALRM, to_alarm);
#endif
                    timeout_flag = 0;   /* for signal handler */
#ifdef ICP
                    if (icp) {
                        nr = recv_icp(sockfd, recvline, retcode);
                        if (verbose) {
                            printf("%s\n", retcode);
                        }
                    } else {
#endif
                        nr = recv(sockfd, recvline, n, 0);
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
                        if ((nr < 0) && (errno == EINTR)
                            && (timeout_flag)) {
                            nr = n;
                            printf("Timeout\n");
#ifdef FLUSH_OUTPUT
                            if (fflush((FILE *) NULL) != 0) {
                                err_sys("I cannot flush");
                            }
#endif
                        }
#ifdef ICP
                    }
#endif
                }
                if (!http && !icp && !smtp && !discard) {
                    if ((nr < 0 || nr != n) && timeout_flag)
                        /* 
                         * if ((nr < 0 || nr != n) &&
                         * (errno == EINTR) &&
                         * timeout_flag)
                         */
                    {
                        printf
                            ("Timeout while reading (%d byte(s) read)\n",
                             (nr == -1) ? 0 : nr);
                        nr = n;
#ifdef FLUSH_OUTPUT
                        if (fflush((FILE *) NULL) != 0) {
                            err_sys("I cannot flush");
                        }
#endif
                        close(sockfd);
                        continue;
                    }
                    if (nr < 0 || nr != n)
                        err_sys
                            ("readline error: %d bytes read, %d bytes requested",
                             nr, n);
                } else
                    /* This is probably HTTP */
                {
                    /* 
                     * printf ("DEBUG: received %d bytes
                     * (flag is %d, errno is %d)\n", nr,
                     * timeout_flag, errno);
                     */
                    if ((errno == EINTR) && timeout_flag) {
                        printf
                            ("Timeout while reading (%d byte(s) read so far)\n",
                             (nr == -1) ? 0 : nr);
#ifdef FLUSH_OUTPUT
                        if (fflush((FILE *) NULL) != 0) {
                            err_sys("I cannot flush");
                        }
#endif
                        close(sockfd);
                        continue;
                    }
                    if (nr < 0) {
                        err_ret("Error reading HTTP reply");
                    }
                }
                if (verbose)
                    printf("%d bytes read from server.\n", nr);
            }
        }                       /* That's all, folks */
        alarm(0);
#ifdef HAVE_TCP_INFO
        /* Thanks to Perry Lorier <perry@coders.net> for the tip. See a longer paper 
         * in http://linuxgazette.net/136/pfeiffer.html */
        if (tcp && verbose) {
            if (getsockopt(sockfd, SOL_TCP, TCP_INFO, &tcpinfo, &socket_length)
                != -1) {
                /* TODO: find out the meaning of the various fields inthe struct
                 * tcp_info (it seems documented only in the Linux kernel sources)
                 * and display stuff like reordering (see RFC 4737), window, lost
                 * packets, etc. */
                printf
                    ("Estimated TCP RTT: %.04f seconds (std. deviation %0.03f)\n",
                     tcpinfo.tcpi_rtt / 1000000.0, tcpinfo.tcpi_rttvar / 1000000.0);
            }
        }
#endif
        if (http) {
#ifdef OPENSSL
            if (ssl)
                SSL_shutdown(channel.ssl);
            else
#endif
#ifdef GNUTLS
            if (ssl)
                shutdown(sockfd, SHUT_RDWR);
            else
#endif
                fclose(channel.fs);
        }
        close(sockfd);

        (void) gettimeofday(&newtv, (struct timezone *) NULL);
        temp = newtv;
        tvsub(&temp, &oldtv);
        if (!timeout_flag && (!plugin || plugin_result >= 0)) { /* If it worked... */
            tvadd(&total, &temp);

            /* Check */
            if (!plugin) {
                if (port_to_use == USE_ECHO) {
                    if (strcmp(sendline, recvline) != 0) {
                        printf(" I wrote:\n%s\n", sendline);
                        printf(" and I got back:\n%s\n", recvline);
                        err_quit("Strange server");
                    }
                    if (verbose) {
                        printf("Checked\n");
#ifdef FLUSH_OUTPUT
                        if (fflush((FILE *) NULL) != 0) {
                            err_sys("I cannot flush");
                        }
#endif
                    }
                }
                if (port_to_use == USE_CHARGEN) {
                    sendline = CHARGENERATED;
                    recvline[strlen(sendline)] = 0;
                    if (strcmp(sendline, recvline) != 0) {
                        /* 
                         * TODO: it does not work if
                         * the size is lower than the
                         * length of CHARGENERATED
                         */
                        printf(" I got back:\n%s\n", recvline);
                        printf(" instead of the most common:\n%s\n", sendline);
                        err_ret("Strange server");
                    }
                    if (verbose) {
                        printf("Checked\n");
                    }
                }
            }
            tvsub(&newtv, &oldtv);
            tvmin(&min, &newtv);
            tvmax(&max, &newtv);
            printf("Elapsed time: %d.%06d seconds\n",
                   (int) newtv.tv_sec, (int) newtv.tv_usec);
#ifdef FLUSH_OUTPUT
            if (fflush((FILE *) NULL) != 0) {
                err_sys("I cannot flush");
            }
#endif
            results[i - 1].valid = 1;
            if (measure_data_transfer_only)
                results[i - 1].timevalue = measured;
            else
                results[i - 1].timevalue = newtv;
            successes++;
        }
        if (number > 1) {
#ifdef OPENSSL
            if (ssl) {
                /* 
                 * SSL_clear (sslh); No, we have to free. Bug
                 * #130151
                 */
                SSL_free(sslh);
            }
#endif
#ifdef GNUTLS
            if (ssl) {
                gnutls_bye(channel.tls, GNUTLS_SHUT_RDWR);
                gnutls_deinit(session);
                /* 
                 * gnutls_certificate_free_credentials(xcred);
                 *
                 */
            }
#endif
        }
    }                           /* End of main loop */

    /* Clean */
    if (plugin)
        plugin_terminate();
    /* It would be nice to clean here for OpenSSL */
#ifdef GNUTLS
    if (ssl) {
        gnutls_global_deinit();
    }
#endif
    printstats();
    if (successes >= 1)
        exit(0);
    else
        exit(1);
}

void
printstats()
{

    int             i;

    /* if ((number > 1) && ((!udp) || (successes > 0))) { */
    if (successes > 1) {
        printf("---\n");
        if (successes < attempts)
            printf("Warning: %d message(s) lost (%d %%)\n",
                   attempts - successes, ((attempts - successes) * 100) / attempts);
        printf("Minimum time: %d.%06d seconds (%.0f bytes per sec.)\n",
               (int) min.tv_sec, (int) min.tv_usec, (double) size / tv2double(min));
        printf("Maximum time: %d.%06d seconds (%.0f bytes per sec.)\n",
               (int) max.tv_sec, (int) max.tv_usec, (double) size / tv2double(max));
        tvavg(&total, successes);
        printf("Average time: %d.%06d seconds (%.0f bytes per sec.)\n",
               (int) total.tv_sec, (int) total.tv_usec,
               (double) size / tv2double(total));
        /* 
         * The number of bytes/second, as printed above, is not
         * really meaningful: size does not reflect the number of
         * bytes exchanged. With echo, N = 2*size, with discard, N =
         * size, with http, N = size + (response)...
         */
        tvstddev(&stddev, successes, total, results);
        printf("Standard deviation: %d.%06d\n",
               (int) stddev.tv_sec, (int) stddev.tv_usec);
        for (i = 0; i < number; i++) {
            if (results[i].valid)
                good_results[j++] = results[i].timevalue;
        }
        if (successes != j)     /* Todo: bug! */
            err_quit("successes (%d) is different from j (%d)", successes, j);
        qsort(good_results, successes, sizeof(struct timeval), tvcmp);
        /* 
         * for (i = 1; i <= number; i++) { printf("---\nTime %d th:
         * %d.%06d seconds\n", i, results[i-1].tv_sec,
         * results[i-1].tv_usec);       }
         */
        if ((successes % 2) == 1) {
            /* 
             * printf("Searching good_results[%d]\n", (successes
             * + 1) / 2 - 1);
             */
            median = good_results[((successes + 1) / 2 - 1)];
        } else {
            /* 
             * printf("Searching good_results[%d] and [%d]\n",
             * (successes / 2) - 1, successes / 2);
             */
            tvadd(&median, &good_results[(successes / 2) - 1]);
            tvadd(&median, &good_results[successes / 2]);
            tvavg(&median, 2);
        }
        printf("Median time: %d.%06d seconds (%.0f bytes per sec.)\n",
               (int) median.tv_sec, (int) median.tv_usec,
               (double) size / tv2double(median));
        if (n_stddev) {
            tvstddevavg(&stddev, successes, total, results, (double) n_stddev);
            printf
                ("Average of values within %d standard deviation%s: %d.%06d\n",
                 n_stddev, (n_stddev == 1 ? "" : "s"),
                 (int) stddev.tv_sec, (int) stddev.tv_usec);
        }
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
to_alarm()
{
    /* printf ("DEBUG: timeout handler called\n"); */
    timeout_flag = 1;           /* set flag for function above */
}

void
interrupted()
{
    printf("Interrupted by user\n");
    printstats();
    exit(1);
}

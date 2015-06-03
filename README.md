echoping
========

echoping **was** a small program to test (approximatively) performances of a remote host by sending it requests such as HTTP requests.

echoping is **no longer** maintained.

To use it, simply:

```
% echoping machine.somewhere.org
```

or use the options before the machine name (see the man page).

See the DETAILS file for various traps when benchmarking networks, specially with this program.

In any case, be polite: don't bother the remote host with many repeated requests, especially with large size. Ask for permission if you often test hosts which aren't yours.

Current features:

    * plugins, so you can extend echoping with any protocol you like and/or use,
    * Supports IPv6 as well as IPv4,
    * Supports IDN (Unicode domain names like café.gennic.net),
    * uses the protocols echo, discard, chargen or HTTP,
    * can use cryptographic connections with HTTP,
    * uses UDP instead of TCP for the protocols which accept it (like echo),
    * can repeat the test and display various measures about it, 

Examples of output:

```
    (Simple test with 1000 bytes echo TCP packets)

    % echoping -v -s 1000 mycisco

    This is echoping, version 5.0.0.

    Trying to connect to internet address 172.21.0.14 7 to transmit 1000 bytes...
    Connected...
    TCP Latency: 0.003165 seconds
    Sent (1000 bytes)...
    Application Latency: 0.322183 seconds
    1000 bytes read from server.
    Checked
    Elapsed time: 0.326960 seconds

    (Repeated tests with average / mean and median displayed.)

    % echoping -n 10 faraway-machine
    [...]
    Minimum time: 6.722336 seconds (38 bytes per sec.)
    Maximum time: 17.975060 seconds (14 bytes per sec.)
    Average time: 10.873267 seconds (24 bytes per sec.)
    Standard deviation: 3.102793
    Median  time: 9.218506 seconds (28 bytes per sec.)

    (Testing a Web server with an HTTP request for its home page.)

    %  echoping -h / mywww
    Elapsed time: 0.686792 seconds
```

The exit status is set if there is any problem, so you can use echoping to test repeatedly a Web server, to be sure it runs fine (SmokePing does it).
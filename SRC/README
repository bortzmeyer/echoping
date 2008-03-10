"echoping" is a small program to test (approximatively) performances
of a remote host by sending it TCP "echo" (or other protocol, like
HTTP) packets.

To install it, see the INSTALL file. Or type "./configure; make" if
you're in a hurry :-)

To use it, simply:

% echoping machine.somewhere.org

or use the options before the machine name (see the man page).

See the DETAILS file for various traps when benchmarking networks,
specially with this program

In any case, be polite: don't bother the remote host with many repeated 
requests, especially with large size. Ask for permission if you often
test hosts which aren't yours.

Current features:

- uses the protocols echo, discard, chargen, HTTP (with SSL/TLS if you
  wish), ICP or SMTP,
- uses UDP instead of TCP for the protocols which accept it (like echo), 
- can repeat the test and display various measures about it,
- supports IPv6 as well as IPv4,
- supports IDN (Unicode domain names),
- supports plugins written by you, to test even more protocols (see PLUGINS),

Examples of output:

(Simple test with 1000 bytes echo TCP packets)
% echoping -v -s 1000 mycisco
This is echoping, version 2.0.
Trying to connect to internet address 10.99.64.1 to transmit 256 bytes...
Connected...
Sent (1000 bytes)...
Checked
Elapsed time: 0.059597 seconds
%

(Repeated tests with average and median displayed.)
% echoping -n 10 mymachine
[...]
Minimum time: 0.170719 seconds (1500 bytes per sec.)
Maximum time: 0.211176 seconds (1212 bytes per sec.)
Average time: 0.184577 seconds (1387 bytes per sec.)
Median time: 0.181332 seconds (1412 bytes per sec.)

(Testing a Web server with an HTTP request for its home page.)
%  echoping -h / mywww
Elapsed time: 0.686792 seconds

(The exit status is set if there is any problem, so you can use echoping
to test repeatedly a Web server, to be sure it runs fine.)



-------------
The reference site for echoping is:

http://echoping.sourceforge.net/

The distribution is from:

http://sourceforge.net/project/showfiles.php?group_id=4581

Stephane Bortzmeyer <bortz@users.sourceforge.net>. October 1995 for the
first version. 

--------------------- 
If you want to help and/or motivate echoping and its developer, you
can (but are not forced to do so, echoping is free software):

* Give money through Sourceforge's donation system
  (http://sourceforge.net/project/project_donations.php?group_id=4581)

* Use my Amazon's wish list
  (http://www.amazon.com/gp/registry/23ELBV1YZ93SC)

* Send me a postcard. I love postcards:
       Stephane Bortzmeyer
       127, rue Brancion
       75015 Paris
       France



$Id$




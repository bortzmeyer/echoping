set terminal postscript portrait monochrome
#set terminal tpic
#set terminal x11
set xlabel "Test number"
set ylabel "Milli-seconds"
set size 0.85,0.75
set title "Response time of the RIPE whois server"
set logscale y
plot "whois-ripe.dat" with points pointsize 3 title "Tests", 2.713 with lines title "Median", 11.394 with lines title "Average"
pause -1 "Hit return to continue"



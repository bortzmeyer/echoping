#!/bin/sh

PATH=/bin:/usr/bin:/usr/local/bin

DELAY=`echoping -n 3 -h /tests/bidon www.netaktiv.com | \
        grep Median | \
        awk '{print int ($3 * 1000)}' `
echo $DELAY
echo $DELAY

UPTIME=`wget -O - -q http://www.netaktiv.com/server-status | \
    grep uptime | \
    awk -F: '{print $2}' | sed 's/<br>//' `

echo $UPTIME
echo Web server


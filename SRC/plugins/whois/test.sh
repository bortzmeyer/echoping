#!/bin/sh

# $Id$

../../echoping -m whois -v whois.nic.fr --dump echoping.fr # Does not exist
../../echoping -m whois -v whois.nic.fr --dump nic.fr # Exists
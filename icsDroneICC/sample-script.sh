#!/bin/sh

###################################################################
# Just a sample shell script that will restart icsDrone if
# it loses connection.  It includes support for timeseal,
# the restart + soft/hardquit remote commands and logging.
#
# The script assumes the timeseal, icsDrone and crafty binary are
# all in the same directory.
#
# You could run this like: ./sample-script.sh > /dev/null &
# but I would advide runing it in foreground first to make sure
# it works.
###################################################################

# Correct these
export FICSHANDLE=guest
export FICSPASSWD=doesnotmatter

# Options passed to icsDrone
OWNER=Hawk
PROGRAM="./crafty"

# "tmplog" is the log of the current session
TMPLOG=tmplog.gz
# "biglog" is the log of all previous sessions
BIGLOG=log.gz

# Time to wait (in seconds) before attemting to reconnect
TIMETOWAIT=60

# Use timeseal if we have it
if [ -x timeseal ]; then
    TIMESEAL=1
    ICSHOST="localhost"
    REALHOST="64.71.131.140"   # freechess.org
else
    TIMESEAL=0
    ICSHOST="freechess.org"
fi

# Main loop
while (true);
do
    if [ $TIMESEAL -eq 1 ]; then 
	echo Starting timeseal
	./timeseal $REALHOST 5000 -p 5000 &
    fi

    sleep 1

    echo Starting icsDrone
    ./icsdrone -icsHost $ICSHOST -program "$PROGRAM" -owner $OWNER -autoConfigure true -logFile $TMPLOG

    # Store exit value
    r=$?

    echo icsDrone exited

    # Append the tmp log to the big log
    cat $TMPLOG >> $BIGLOG
    
    # The *quit remote commands cause exitvalues bigger than 1 (the 'restart' exitvalue)
    if [ $r -gt 1 ]; then
	break;
    fi

    sleep $TIMETOWAIT;
done;

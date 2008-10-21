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
export FICSHANDLE=minimardi
export FICSPASSWD=secretpassword

# Options passed to icsDrone
OWNER=palito
PROGRAM="/home/palle/minimardi/minimardi"
export PATH=$PATH:$PROGRAM:./
TIMESTAMP_PID=/home/palle/icsDrone/timestamp.pid
#ENGINE_PID=/home/palle/icsDrone/engine.pid
# "tmplog" is the log of the current session
TMPLOG=tmplog.gz
# "biglog" is the log of all previous sessions
BIGLOG=log.gz

# Time to wait (in seconds) before attemting to reconnect
TIMETOWAIT=300

# Use timeseal if we have it
if [ -x timestamp ]; then
    TIMESEAL=1
    ICSHOST="chessclub.com"
    REALHOST="207.99.83.228"   # freechess.org
else
    TIMESEAL=0
    ICSHOST="chessclub.com"
fi

# Main loop
while (true);
do
    if [ -f $TIMESTAMP_PID ]
    then
    	echo "Timestamp already running"
    	kill `cat $TIMESTAMP_PID`
    fi
    if [ $TIMESEAL -eq 1 ]; then 
	echo Starting timeseal
	/home/palle/icsDrone/timestamp $REALHOST 5000 -p 5000 &
	echo $! > $TIMESTAMP_PID
    fi

    sleep 1
    #if [ -f $ENGINE_PID ]
    #then 
    #	kill `cat $ENGINE_PID`
    #fi
    echo Starting date: `date`
    echo Starting icsDrone    
    ./icsdrone -icsHost $ICSHOST -program "$PROGRAM" -owner $OWNER -craftyMode on -console on -logFile $TMPLOG -sendGameEnd "seek 1 0\nseek 3 0\nseek 15 0" -killEngine on  -haveCmdResult on -haveCmdNew on   
    #echo $! > $ENGINE_PID
    # Store exit value
    r=$?

    echo icsDrone exited with code=$r
    echo Exit `date`
    # Append the tmp log to the big log
    cat $TMPLOG >> $BIGLOG
    
    # The *quit remote commands cause exitvalues bigger than 1 (the 'restart' exitvalue)
    if [ $r -gt 1 ]; then
	break;
    fi

    sleep $TIMETOWAIT;
done;

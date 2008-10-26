#!/bin/sh
# The script requires the following binaries:
# timestamp (optional)
# icsdrone-icc.bin (should be in /usr/sbin)
# pgnextract (should be in /usr/bin) 
# engine (eg. /usr/games/minimardi)


# Configuration:
# ICC login
export FICSHANDLE=minimardi
# ICC password
export FICSPASSWD=secretpassword
# Account owner
OWNER=palito
# Absolute path to the engine
PROGRAM="/usr/games/minimardi"

# Use tmp as the running directory, 
# some temporary files (tmp.pgn) are written 
# to the running directory.
cd /tmp

#icsdrone reads .icsdronerc which we use
#to send the first seek
echo "seek 3 0" > ./.icsdronerc

###### MINIMARDI SPECIFIC - OK TO DELETE ########
if [ -f Book.txt ]
then
	echo "minimardi will use book."
else
	ln -s /usr/games/Book.txt Book.txt
fi
###### MINIMARDI SPECIFIC - OK TO DELETE ########    

# No need to modify stuff from here ..
ICSDRONEBIN="/usr/sbin/icsdrone-icc.bin"
TIMESTAMP_PID=/tmp/timestamp.pid

export PATH=$PATH:/usr/bin:$PROGRAM:./
# Time to wait (in seconds) before attemting to reconnect
TIMETOWAIT=300

# Use timestmap if we have it
if [ -x /usr/bin/timestamp ]; then
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
	echo Starting timestamp
	timestamp $REALHOST 5000 -p 5000 &
	echo $! > $TIMESTAMP_PID
    fi

    sleep 1
    echo Starting icsDroneICC    
    $ICSDRONEBIN -icsHost $ICSHOST -program "$PROGRAM" -owner $OWNER -craftyMode on -console off -logFile off -sendGameEnd "seek 1 0\nseek 3 0\nseek 15 0" -killEngine on  -haveCmdResult on -haveCmdNew on   
    r=$?
    echo icsDrone exited with code=$r
    
    # The *quit remote commands cause exitvalues bigger than 1 (the 'restart' exitvalue)
    if [ $r -gt 1 ]; then
	break;
    fi

    sleep $TIMETOWAIT;
done;

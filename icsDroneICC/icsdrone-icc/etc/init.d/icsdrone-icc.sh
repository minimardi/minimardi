#!/bin/bash
ICSDRONE_SCRIPT="/usr/sbin/icsdrone-icc-main.sh"
ICSDRONE_OUT="/dev/null"

do_start()
{
	echo "Starting ICSDroneICC"
	nohup nice $ICSDRONE_SCRIPT > $ICSDRONE_OUT 2>&1 &
}
do_stop()
{
	echo "Stopping ICSDroneICC"
	killall icsdrone-icc-main.sh
	killall icsdrone-icc.bin
	killall timestamp
}
case "$1" in
	start)
		do_start
		;;
	stop)
		do_stop
		;;
	restart)
		do_stop
		do_start
		;;
	*)
		echo "Usage: $SCRIPTNAME {start|stop|restart}"
		exit 3
		;;
esac


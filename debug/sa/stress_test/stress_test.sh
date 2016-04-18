#!/bin/sh

CUR_DIR=`dirname $0`

TARGET="single bit error"
#DMESG="cat $CUR_DIR/dummy.txt"
DMESG="dmesg"
LOGFILE="$CUR_DIR/logfile.txt"
TTYDEV="/dev/ttyO0"
SLEEP_TIME=30

echo "-----------------------sleep $SLEEP_TIME s---------------------------" | tee $TTYDEV
sleep $SLEEP_TIME

datetime=`date "+%Y-%m-%d %H:%M:%S"`
echo $datetime >> $LOGFILE
$DMESG >> $LOGFILE

count=`$DMESG | grep "$TARGET" | wc -l`
echo count=$count
if [ $count -ge 1 ]; then
	$DMESG | grep "$TARGET" | tee -a $LOGFILE
	echo "------------found target and do reboot------------" | tee -a $LOGFILE
	#reboot -f
else
	echo "-----------------the system will be reboot--------------------" | tee -a $LOGFILE
	#reboot -f
fi

echo "" >> $LOGFILE
sync
sleep 3
reboot -f

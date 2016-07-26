#!/bin/bash

for((i=1;i<9999999999;i++))
do
	sudo adb wait-for-device
	echo "End:   ---> `date`"
	sudo adb shell reboot -f
	echo "-------------------------------------------------------"
	echo "/------ test count: $i ------/"
	echo "Begin: ---> `date`"
	sleep 30
done



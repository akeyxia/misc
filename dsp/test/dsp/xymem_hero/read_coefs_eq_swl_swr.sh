#! /system/bin/bash

#PH=/sys/devices/platform/imx-i2c.1/i2c-1/1-001c
PH=/sys/devices/platform/imx-i2c.0/i2c-0/0-0060


declare -i base=16#0 


echo "EQ SWL & SWR, 40 values"
for((i=0; i<40; i++)); do
	base=16#F244D1+i
	addr=`printf "0x%X" $base`
	read_Y.sh $addr
done
echo














































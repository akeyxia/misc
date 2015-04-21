#! /system/bin/bash

#PH=/sys/devices/platform/imx-i2c.1/i2c-1/1-001c
PH=/sys/devices/platform/imx-i2c.0/i2c-0/0-0060


declare -i base=16#0 


echo "EQ FL bands 5 to 8, 40 values"
for((i=0; i<40; i++)); do
	base=16#D1269+i
	addr=`printf "0x%X" $base`
	read_Y.sh $addr
done
echo














































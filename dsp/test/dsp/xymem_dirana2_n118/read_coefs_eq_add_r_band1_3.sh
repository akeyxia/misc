#! /system/bin/bash

#PH=/sys/devices/platform/imx-i2c.1/i2c-1/1-001c
PH=/sys/devices/platform/imx-i2c.0/i2c-0/0-0060


declare -i base=16#0 


echo "EQ Add. R bands 1 to 3, 21 values"
for((i=0; i<21; i++)); do
	base=16#D11DC+i
	addr=`printf "0x%X" $base`
	read_Y.sh $addr
done
echo














































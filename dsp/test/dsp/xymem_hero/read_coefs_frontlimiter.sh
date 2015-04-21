#! /system/bin/bash

#PH=/sys/devices/platform/imx-i2c.1/i2c-1/1-001c
PH=/sys/devices/platform/imx-i2c.0/i2c-0/0-0060


declare -i base=16#0 


echo "Front Limiter, 6 values(including 1 fixed value)"
read_X.sh 0xF2048E

for((i=0; i<4; i++)); do
	base=16#F2435F+i
	addr=`printf "0x%X" $base`
	read_Y.sh $addr
done
echo














































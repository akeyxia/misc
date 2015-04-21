#! /system/bin/bash

#PH=/sys/devices/platform/imx-i2c.1/i2c-1/1-001c
PH=/sys/devices/platform/imx-i2c.0/i2c-0/0-0060


declare -i base=16#0 


echo "Front/Rear Limiter, 5 values"
read_X.sh 0x0D01C8

for((i=0; i<4; i++)); do
	base=16#D1088+i
	addr=`printf "0x%X" $base`
	read_Y.sh $addr
done
echo














































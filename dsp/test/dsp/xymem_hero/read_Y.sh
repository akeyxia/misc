#! /system/bin/bash

#PH=/sys/devices/platform/imx-i2c.1/i2c-1/1-001c
PH=/sys/devices/platform/imx-i2c.0/i2c-0/0-0060

y_addr=$1

echo $y_addr > $PH/yaddr              
value=`cat $PH/ydata`                  
echo read Y: $y_addr $value



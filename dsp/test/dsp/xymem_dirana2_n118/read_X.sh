#! /system/bin/sh

PH=/sys/devices/platform/imx-i2c.1/i2c-1/1-001c
#PH=/sys/devices/platform/imx-i2c.0/i2c-0/0-0060

x_addr=$1

echo $x_addr > $PH/xaddr              
value=`cat $PH/xdata`                  
echo read X: $x_addr $value



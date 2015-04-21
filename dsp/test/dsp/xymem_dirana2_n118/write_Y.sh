#! /system/bin/sh

PH=/sys/devices/platform/imx-i2c.1/i2c-1/1-001c
#PH=/sys/devices/platform/imx-i2c.0/i2c-0/0-0060

y_addr=$1
y_data=$2

echo $y_addr > $PH/yaddr              
#value=`cat $PH/ydata`                  
#echo Y:FMIC_1_SupThr 0x0111D9 $value
echo $y_data > $PH/ydata
echo write Y: $y_addr $y_data


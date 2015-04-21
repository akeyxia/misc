#! /system/bin/bash

#PH=/sys/devices/platform/imx-i2c.1/i2c-1/1-001c
PH=/sys/devices/platform/imx-i2c.0/i2c-0/0-0060

x_addr=$1
x_data=$2

echo $x_addr > $PH/xaddr              
#value=`cat $PH/xdata`                  
#echo X:FMIC_1_SupThr 0x0111D9 $value
echo $x_data > $PH/xdata
echo write X: $x_addr $x_data


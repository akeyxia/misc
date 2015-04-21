#! /system/bin/bash

#PH=/sys/devices/platform/imx-i2c.1/i2c-1/1-001c
PH=/sys/devices/platform/imx-i2c.0/i2c-0/0-0060

read_Y.sh 0xF2434D
read_Y.sh 0xF2434E
read_Y.sh 0xF2434F
read_Y.sh 0xF24350
read_Y.sh 0xF242FF
read_Y.sh 0xF24300
read_Y.sh 0xF24355
read_Y.sh 0xF24356
read_Y.sh 0xF24345
read_X.sh 0xF20497
read_X.sh 0xF2049A


#! /system/bin/sh

CUR_PH=$(cd `busybox dirname $0`; pwd)

mount -o remount,rw /dev/block/mmc0blk2 /system

busybox cp $CUR_PH/bash /system/bin
chmod 777 /system/bin/bash

busybox cp $CUR_PH/read_X.sh /system/bin
chmod 777 /system/bin/read_X.sh

busybox cp $CUR_PH/write_X.sh /system/bin
chmod 777 /system/bin/write_X.sh

busybox cp $CUR_PH/read_Y.sh /system/bin
chmod 777 /system/bin/read_Y.sh

busybox cp $CUR_PH/write_Y.sh /system/bin
chmod 777 /system/bin/write_Y.sh

mount -o remount,ro /dev/block/mmc0blk02 /system


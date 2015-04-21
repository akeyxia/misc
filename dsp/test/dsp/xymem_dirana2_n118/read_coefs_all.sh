#! /system/bin/bash

#PH=/sys/devices/platform/imx-i2c.1/i2c-1/1-001c
PH=/sys/devices/platform/imx-i2c.0/i2c-0/0-0060

./read_coefs_delays.sh
./read_coefs_changain.sh
./read_coefs_gpf2.sh
./read_coefs_gpf3.sh
./read_coefs_gpf4.sh
./read_coefs_eq_swl.sh
./read_coefs_eq_swr.sh
./read_coefs_eq_fl_band5_8.sh
./read_coefs_eq_fr_band5_8.sh
./read_coefs_eq_rl_band5_8.sh
./read_coefs_eq_rr_band5_8.sh
./read_coefs_eq_add_l_band4_5.sh
./read_coefs_eq_add_r_band4_5.sh
./read_coefs_eq_fl_band1_4.sh
./read_coefs_eq_fr_band1_4.sh
./read_coefs_eq_rl_band1_4.sh
./read_coefs_eq_rr_band1_4.sh
./read_coefs_eq_add_l_band1_3.sh
./read_coefs_eq_add_r_band1_3.sh
./read_coefs_front_rear_limiter.sh
./read_coefs_eq_scalers.sh
./read_coefs_upcalers.sh

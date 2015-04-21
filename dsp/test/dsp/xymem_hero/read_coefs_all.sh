#! /system/bin/bash

#PH=/sys/devices/platform/imx-i2c.1/i2c-1/1-001c
PH=/sys/devices/platform/imx-i2c.0/i2c-0/0-0060

./read_coefs_delays.sh
./read_coefs_changain.sh
./read_coefs_gpf.sh
./read_coefs_gpf2.sh
./read_coefs_gpf3.sh
./read_coefs_gpf4.sh
./read_coefs_gpf5.sh
./read_coefs_eq_swl_swr.sh
./read_coefs_eq_fl_band5_7.sh
./read_coefs_eq_fr_band5_7.sh
./read_coefs_eq_rl_band5_7.sh
./read_coefs_eq_rr_band5_7.sh
./read_coefs_eq_fl_band1_4.sh
./read_coefs_eq_fr_band1_4.sh
./read_coefs_eq_rl_band1_4.sh
./read_coefs_eq_rr_band1_4.sh
./read_coefs_navivoicefilter.sh
./read_coefs_phonevoicefilter.sh
./read_coefs_bmt_bass.sh
./read_coefs_bmt_mid.sh
./read_coefs_bmt_treble.sh
./read_coefs_frontscd.sh
./read_coefs_rearscd.sh
./read_coefs_frontlimiter.sh
./read_coefs_rearlimiter.sh
./read_coefs_eq_scalers.sh
./read_coefs_upcalers.sh

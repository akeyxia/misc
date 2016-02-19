#!/bin/bash
echo write device id:IP31-ZXQ-A1-H-N0001
sudo ./ip31-write-param_x86 --write-deviceID IP31-ZXQ-A1-H-N0001
echo -e "read device id:: \c"
sudo ./ip31-write-param_x86 --read-deviceID

echo
echo write btaddr:0123456789ab
sudo ./ip31-write-param_x86 --write-btaddr 0123456789ab
echo -e "read btaddr:: \c"
sudo ./ip31-write-param_x86 --read-btaddr

echo
echo write wifiaddr:ab0123456789
sudo ./ip31-write-param_x86 --write-wifiaddr ab0123456789
echo -e "read wifiaddr:: \c"
sudo ./ip31-write-param_x86 --read-wifiaddr

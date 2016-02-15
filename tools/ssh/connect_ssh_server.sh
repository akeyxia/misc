#!/bin/bash

usage()
{
	echo "
-------------------------------------------------------
@usage: $0 user_name remote_ip
-------------------------------------------------------
	"
	exit 1
}

if [ $# -lt 1 ]; then
	usage
fi

user_name=$1

if [ $# -ge 2 ]; then
	remote_ip=$2
else
	remote_ip=10.25.20.11
fi

echo ${user_name}@${remote_ip}

ssh ${user_name}@${remote_ip}


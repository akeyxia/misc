#!/bin/bash

usage()
{
	echo "
-------------------------------------------------------
@usage: $0 user_name -push/pull src_path dest_path
-------------------------------------------------------
	"
	exit 1
}

if [ $# -lt 4 ]; then
	usage
fi

remote_ip=10.25.20.12
user_name=$1
action=$2
src_path=$3
dest_path=$4

echo ${user_name}@${remote_ip} ${action} ${src_path} ${dest_path}

case ${action} in
	-pull)
		scp ${user_name}@${remote_ip}:${src_path} ${dest_path}
		;;
	-push)
		scp ${src_path} ${user_name}@${remote_ip}:${dest_path}
		;;
    *)
	    echo "unkown parameter";
		usage;;
esac

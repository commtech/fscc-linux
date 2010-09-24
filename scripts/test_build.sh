#!/bin/bash

BASE_PATH="~/Kernels/"

START=16
END=35

function build_kernel()
{
	echo "Building linux-2.6.$1"
	make clean
	make KERNEL_PATH=$BASE_PATH""linux-2.6.$1/ > /dev/null

    if [ $? -ne 0 ]; then
        exit $?
    fi
}

if [ $1 ] && [ $2 ]; then
	for i in $(seq $1 $2)
	do
		build_kernel $i
	done
else
	if [ $1 ]; then
		build_kernel $1
	else
		for i in $(seq $START $END)
		do
			build_kernel $i
		done
	fi
fi

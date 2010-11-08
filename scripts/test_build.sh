#!/bin/bash

BASE_KERNEL_PATH="~/Kernels/"
BASE_C_EXAMPLES_PATH=~/Repos/fscc-linux/examples/c/

START=16
END=35

function build_kernel()
{
	echo "Building linux-2.6.$1"
	make clean
	make KERNEL_PATH=$BASE_KERNEL_PATH""linux-2.6.$1/ > /dev/null

    if [ $? -ne 0 ]; then
        exit $?
    fi
}

for FILE in `ls $BASE_C_EXAMPLES_PATH*.c`
do
    gcc -Wall -Wextra -ansi -pedantic $FILE
    echo $FILE
    if [ $? -ne 0 ]; then
        exit $?
    fi
done

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

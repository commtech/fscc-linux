#!/bin/sh

# Creates a soft link between the fscc sysfs directory and here so it is less
# typing when you would like to use it's functionality.

ln -s /sys/class/fscc sysfs

#!/bin/sh

2to3 -p examples/python/*.py cli/fscc.py cli/fscc-cli
if [ "$?" -ne "0" ]; then
    exit $?
fi

pyflakes cli/fscc.py cli/fscc-cli examples/python/*.py
if [ "$?" -ne "0" ]; then
    exit $?
fi

#!/bin/sh

2to3 -p examples/python/*.py cli/fscc.py cli/fscc-cli gui/*.py gui/fscc-gui
if [ $? -ne 0 ]; then
    exit $?
fi

pyflakes cli/fscc.py cli/fscc-cli examples/python/*.py gui/*.py gui/fscc-gui
if [ $? -ne 0 ]; then
    exit $?
fi

#!/bin/sh

REINDENT_PATH=~/reindent.py

sed -i 's/[[:space:]]*$//g' src/*.* examples/c/*.* examples/c#/*.* include/*.*
python $REINDENT_PATH -r .
python $REINDENT_PATH -r cli/fscc-cli

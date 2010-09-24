#!/bin/sh

REINDENT_PATH=~/reindent.py

sed -i 's/[[:space:]]*$//g' **/*.*
python $REINDENT_PATH -r .
python $REINDENT_PATH -r cli/fscc-cli

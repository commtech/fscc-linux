#!/bin/sh

sed -i 's/[[:space:]]*$//g' src/*.* examples/c/*.* examples/c#/*.* include/*.*
reindent -r .
reindent cli/fscc-cli gui/fscc-gui

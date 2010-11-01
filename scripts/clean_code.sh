#!/bin/sh

sed -i 's/[[:space:]]*$//g' src/*.* examples/c/*.* examples/c#/*.* include/*.*
reindent -r . cli/fscc-cli gui/fscc-gui
chmod u+x cli/fscc-cli gui/fscc-gui

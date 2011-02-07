#!/bin/sh

# Downloads and installs the various tools related to the FSCC (command line,
# graphical settings editor and python libraries).

rm -r tools
mkdir tools
cd tools

wget http://pyfscc.googlecode.com/files/pyfscc.tar.gz
wget http://fscc-cli.googlecode.com/files/fscc-cli.tar.gz
# wget http://fscc-gui.googlecode.com/files/fscc-gui.tar.gz

tar xf pyfscc*.tar.gz
tar xf fscc-cli*.tar.gz
tar xf fscc-gui*.tar.gz

cd pyfscc*
python setup.py install
cd ..

cd fscc-cli*
python setup.py install
cd ..

# cd fscc-gui*
# python setup.py install
# cd ..

cd ..
rm -r tools/

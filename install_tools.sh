#!/bin/sh

# Downloads and installs the various tools related to the FSCC (command line,
# graphical settings editor and python libraries).

rm -r tools
mkdir tools
cd tools

wget http://hg.commtech-fastcom.com/pyfscc/get/tip.tar.gz
mv tip.tar.gz pyfscc.tar.gz
tar xzf pyfscc.tar.gz
wget http://hg.commtech-fastcom.com/fscc-cli/get/tip.tar.gz
mv tip.tar.gz fscc-cli.tar.gz
tar xzf fscc-cli.tar.gz
#wget http://hg.commtech-fastcom.com/fscc-gui/get/tip.tar.gz
#mv tip.tar.gz fscc-gui.tar.gz
#tar xzf fscc-gui.tar.gz

cd fastcom-pyfscc*
python setup.py install
cd ..

cd fastcom-fscc-cli*
python setup.py install
cd ..

# cd fscc-gui*
# python setup.py install
# cd ..

cd ..

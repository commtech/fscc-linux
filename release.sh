#!/bin/sh
VER=2.5.3

git clone --recursive git@github.com:commtech/fscc-linux.git fscc-linux-$VER
cd fscc-linux-$VER
rm -rf .git*
cd -
tar -czf fscc-linux-$VER.tar.gz fscc-linux-$VER/
rm -rf fscc-linux-$VER

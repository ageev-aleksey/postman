#!/bin/sh
mkdir external
# CUnit
wget https://github.com/jacklicn/CUnit/archive/v2.1-3.tar.gz -P external
tar -xvf  external/v2.1-3.tar.gz -C external
cd ./external/CUnit-2.1-3/
libtoolize --force
aclocal
autoheader
automake --force-missing --add-missing
autoconf
./configure
make
make install

#!/bin/bash

set -x

rm -fr ./build
mkdir -p build
pushd build
cmake ..
make
sudo make install && sudo ldconfig
popd

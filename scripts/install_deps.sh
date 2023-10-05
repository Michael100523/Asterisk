#!/bin/bash

set -x

DOTNET_SDK="${DOTNET_SDK:-dotnet-sdk-7.0}"
GRPC_RELEASE_TAG="${GRPC_RELEASE:-1.54.1}"
AST_VERSION="${AST_VERSION:-20.2.1}"

# basic system update
sudo apt update
sudo apt install -y build-essential autoconf libtool pkg-config cmake apt-trans-https git

# Install .NET
wget https://packages.microsoft.com/config/debian/11/packages-microsoft-prod.deb -O packages-microsoft-prod.deb
sudo dpkg -i packages-microsoft-prod.deb
rm packages-microsoft-prod.deb
sudo apt update
sudo apt install -y ${DOTNET_SDK}

# Install gRPC from sources
mkdir -p third-party
pushd third-party
git clone -b v${GRPC_RELEASE_TAG} https://github.com/grpc/grpc
pushd grpc
git submodule update --init
mkdir -p cmake/build
pushd cmake/build
cmake -DgRPC_INSTALL=ON -DBUILD_SHARED_LIBS=ON ../..
make -j 4
sudo make install && sudo ldconfig
popd
popd
popd

# Install Asterisk from sources
mkdir -p third-party
pushd third-party
wget https://downloads.asterisk.org/pub/telephony/asterisk/releases/asterisk-${AST_VERSION}.tar.gz
tar -xvzf asterisk-${AST_VERSION}.tar.gz
rm asterisk-${AST_VERSION}.tar.gz
mv asterisk-${AST_VERSION} asterisk
pushd asterisk
sudo ./contrib/scripts/install_prereq install
./configure
#   todo tweak menuselect to limit what is built and installed
make
sudo make install
sudo make samples
sudo make config
sudo make install-logrotate
popd
popd


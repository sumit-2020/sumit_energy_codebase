#!/bin/sh

pushd .
cd riffa_driver/linux/

sudo make setup

make

sudo make install

popd

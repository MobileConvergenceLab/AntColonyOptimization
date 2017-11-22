#! /usr/bin/env bash

apt-get install git -y
apt-get install mininet -y
apt-get install ovs -y
apt-get install libpstream -y
apt-get install libpstreams-dev -y
apt-get install libglib2.0-dev -y

make

#!/bin/sh

echo "Building minimardi"
cd ./src
make clean
make
cp ./minimardi ../minimardi/usr/games
cp ./Book.txt ../minimardi/usr/games
cd ..
dpkg --build minimardi ./


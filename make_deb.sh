#!/bin/sh

echo "Building minimardi"
cd ./src
make clean
make
cp ./minimardi ../minimardi/usr/games
cp ./Book.txt ../minimardi/usr/games
cd ..

rm -Rf ./build
mkdir build
cp -R ./minimardi ./build
cd build
find -type d | grep CVS | sed "s/^/rm -rf \"/"|sed "s/$/\"/" | sh 
dpkg --build minimardi ./
rm -Rf ./minimardi
cd ..

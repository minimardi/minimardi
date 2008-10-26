#!/bin/sh

echo "Building icsdroneicc"
make clean
make
cp ./icsdrone ./icsdrone-icc/usr/sbin/icsdrone-icc.bin
chmod 755 ./icsdrone-icc/usr/sbin/*
chmod 755 ./icsdrone-icc/usr/bin/*
chmod 755 ./icsdrone-icc/etc/init.d/*
chmod 755 ./icsdrone-icc/DEBIAN/*
chmod 644 ./icsdrone-icc/DEBIAN/control
gzip ./icsdrone-icc/usr/share/man/man1/icsdrone-icc.1
gzip ./icsdrone-icc/usr/share/doc/icsdrone-icc/copyright
dpkg --build icsdrone-icc ./


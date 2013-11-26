#!/bin/bash

GIT=~/git/pastry/
BUILD=~/cross/pastry/
TARGET=~/cross/release
WUALA=/home/david/WualaDrive/Danvil/Share/PragmaDanvil/pastry/

cd $BUILD
make -j5

cd $TARGET
rm libpastry.dll
rm pastry-example-*.exe
rm -rf $TARGET/assets
cp $BUILD/examples/pastry-example-*.exe .
cp $BUILD/src/libpastry.dll .
cp -R $GIT/examples/assets/ .

#tar -cpf – *.exe *.dll | 7za a -si -mx=9 pastry.tar.7z 1>/dev/null 2>&1
rm pastry.7z
7z a -t7z pastry.7z *.exe *.dll assets

cp pastry.7z $WUALA

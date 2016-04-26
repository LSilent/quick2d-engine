#!/bin/sh
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
SRCDIR=$DIR/LuaJIT-2.1.0-beta2
DESTDIR=$DIR/prebuilt/mac
MACOSX_DEPLOYMENT_TARGET="10.6"

rm -fr $DESTDIR
mkdir -p $DESTDIR

cd $SRCDIR
make clean
make CC="gcc -m64 -arch x86_64" clean all

if [ -f $SRCDIR/src/libluajit.a ]; then
    mv $SRCDIR/src/libluajit.a $DESTDIR/libluajit.a
fi;

make clean

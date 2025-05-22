#!/usr/bin/env bash

if [ $# -lt 1 ]; then
	echo "file name?"
	exit 1
fi

NAME="$1"

make test
./build/e ./test/${NAME}.m
cp ./test/${NAME}.s object
cd object
make
./asm ${NAME}.s
./machine ${NAME}.o
#!/bin/bash

make clean

cd build
make clean

for file in `find .`
do
	if [ -f "../"$file ]; then
		rm "../"$file
	fi
done

cd ..


while [ `find -type d -empty | wc -w` -ne 0 ]
do
	find -type d -empty | xargs rm -rf 
done

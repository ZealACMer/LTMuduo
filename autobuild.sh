#!/bin/bash

set -e

#如果没有build目录，创建该目录
if [ ! -d `pwd`/build ]; then
    mkdir `pwd`/build
fi

rm -rf `pwd`/build/*

cd `pwd`/build &&
    cmake .. &&
    make

#回到项目根目录 
cd ..

#把头文件拷贝到/usr/include/ltmuduo 动态库so拷贝到/usr/lib
if [ ! -d /usr/include/ltmuduo ]; then
    mkdir /usr/include/ltmuduo
fi

for header in `ls *.h`
do
    cp $header /usr/include/ltmuduo
done

cp `pwd`/lib/libltmuduo.so /usr/lib

#刷新动态库缓存
ldconfig

#terminal => chmod +x autobuild.sh
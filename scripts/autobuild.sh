#!/bin/bash

set -e
cd `dirname $0`
cd ..

# 如果没有build目录，创建该目录
if [ ! -d `pwd`/build ]; then
    mkdir `pwd`/build
fi

rm -rf `pwd`/build/*

cmake -S . -B build
cmake --build build 
cmake --install build

# 回到项目根目录

# 把头文件拷贝到 /usr/include/mymuduo  so库拷贝到 /usr/lib    PATH
if [ ! -d /usr/include/tcprpc ]; then 
    mkdir /usr/include/tcprpc
fi

if [ ! -d /usr/include/mytcp ]; then 
    mkdir /usr/include/mytcp
fi

if [ ! -d /usr/include/mylog ]; then 
    mkdir /usr/include/mylog
fi

if [ ! -d /usr/include/mytool ]; then 
    mkdir /usr/include/mytool
fi

cd `pwd`/public/include/RpcServer

# 拷贝hpp文件
for header in `ls *.h`
do
    cp $header /usr/include/tcprpc
done

cd ../TcpServer

for header in `ls *.h`
do
    cp $header /usr/include/mytcp
done

cd ../Log

for header in `ls *.h`
do
    cp $header /usr/include/mylog
done

cd ../Tool
for header in `ls *.h`
do
    cp $header /usr/include/mytool
done

cd ../..
cp `pwd`/lib/* /usr/lib

ldconfig
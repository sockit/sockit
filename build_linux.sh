#!/bin/bash

# ./build_linux.sh will build the sockit project for your platform.
# ./build_linux.sh 32 will attempt to make it for 32-bit platforms; 
# ./build_linux.sh 64 will do the same for 64-bit platforms.
# ./build_linux.sh clean - clean before building
# ./build_linux.sh 64 clean - clean before building 64-bit.

if [ $# -gt 0 ] && [ "$1" == "32" ]
then
    export CFLAGS="-m32"
    export CXXFLAGS="-m32"
    ../../prepmake.sh -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_C_CFLAGS="-m32 -O3" -DCMAKE_CXX_FLAGS="-m32 -D__UNIX__ -O3"
elif [ $# -gt 0 ] && [ "$1" == "64" ]
then
    ../../prepmake.sh -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_C_CFLAGS="-m64 -O3" -DCMAKE_CXX_FLAGS="-m64 -D__UNIX__ -O3"
else
    ../../prepmake.sh -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_C_CFLAGS="-mtune=native -O3" -DCMAKE_CXX_FLAGS="-mtune=native -O3 -D__UNIX__"
fi

cd ../../build

if [ $# -gt 0 ] && [ "$1" == "clean" ]
then
    make clean
elif [ $# -gt 1 ] && [ "$2" == "clean" ]
then
    make clean
fi

CORES=lscpu | grep "Core(s) per socket" | awk '{ print $4 }'
make -j $CORES --no-print-directory
cd -

mkdir -p bin/linux
if [ $# -gt 0 ] && [ "$1" == "32" ]
then
    cp ../../build/bin/SockIt/npSockIt.so bin/linux/npSockIt32.so
elif [ $# -gt 0 ] && [ "$1" == "64" ]
then
    cp ../../build/bin/SockIt/npSockIt.so bin/linux/npSockIt64.so
else
    cp ../../build/bin/SockIt/npSockIt.so bin/linux/npSockIt.so
fi

cp bin/linux/*.so ~/.mozilla/plugins

#!/bin/bash

# ./build.sh will build the sockit project for your platform.
# ./build.sh 32 will attempt to make it for 32-bit platforms; 
# ./build.sh 64 will do the same for 64-bit platforms.

if [ $# -gt 0 ] && [ $1 -eq 32 ]
then
    export CFLAGS="-m32"
    export CXXFLAGS="-m32"
    ../../prepmake.sh -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_C_CFLAGS="-m32" -DCMAKE_CXX_FLAGS="-m32 -D__UNIX__"
else
    ../../prepmake.sh -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_CXX_FLAGS="-D__UNIX__"
fi

cd ../../build
CORES=lscpu | grep "Core(s) per socket" | awk '{ print $4 }'
make -j $CORES --no-print-directory
cd -

mkdir -p bin/linux
if [ $# -gt 0 ] && [ $1 -eq 32 ]
then
    cp ../../build/bin/SockIt/npSockIt.so bin/linux/npSockIt32.so
else
    cp ../../build/bin/SockIt/npSockIt.so bin/linux/npSockIt64.so
fi

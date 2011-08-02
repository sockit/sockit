#!/bin/bash

# This will attempt to build a universal binary of sockit for Mac OSX
# The plugin binaries can be found in ../../build/projects/sockit/<buildtype>/sockit.plugin
# copy those to your ~/Library/Internet Plug-Ins/ folder to install

cd ../..
./prepmac.sh -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_CXX_FLAGS="-D__OSX__"
cd build
xcodebuild __OSX__="true"

# NOTE: You will need to update this 'sockit' path to be the path of the directory into
#   which you cloned the sockit repo, if it is not the default
cd ../projects/sockit

mkdir -p bin/mac
cp -a ../../build/bin/SockIt/Debug/SockIt.plugin bin/mac/

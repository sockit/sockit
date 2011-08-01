#!/bin/bash

# This will attempt to build a universal binary of sockit for Mac OSX
# The plugin binaries can be found in ../../build/projects/sockit/<buildtype>/sockit.plugin
# copy those to your ~/Library/Internet Plug-Ins/ folder to install

cd ../..
./prepmac.sh -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_CXX_FLAGS="-D__OSX__"
cd build
xcodebuild __OSX__="true"

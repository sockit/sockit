#!/bin/bash

google-chrome --pack-extension=bin/linux/linux

# Again, takes an argument, 32 or 64
if [ "$1" = "32" ]
then
    mv bin/linux/linux.crx bin/linux/sockit_linux_32.crx
else
    mv bin/linux/linux.crx bin/linux/sockit_linux_64.crx
fi

#!/bin/bash

# Again, takes an argument, 32 or 64
if [ "$1" = "32" ]
then
    cp bin/linux/npSockIt32.so bin/linux/npSockIt.so
else
    cp bin/linux/npSockIt64.so bin/linux/npSockIt.so
fi


google-chrome --pack-extension=bin/linux
rm bin/linux/npSockIt.so


# Again, takes an argument, 32 or 64
rm bin/linux.pem
if [ "$1" = "32" ]
then
    mv bin/linux.crx bin/linux/sockit_linux_32.crx
else
    mv bin/linux.crx bin/linux/sockit_linux_64.crx
fi

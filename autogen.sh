#!/bin/sh

## 
## Sets up and generates all of the stuff that
## automake and libtool do for us.
##

RESTCLIENTREPO="https://github.com/emcvipr/rest-client-c"
echo "Setting up libtool"
libtoolize --force
echo "Setting up aclocal"
aclocal -I m4
echo "Generating configure"
autoconf
echo "Generating config.h.in"
autoheader
echo "Generating Makefile.in"
automake --add-missing
echo "retrieving rest-client-c dependency repo"
if [ -d "dep"  ]; then
  echo "The rest client dep directory already exists will attempt to pull from github repo"
  sh -c "cd dep && git pull origin master $RESTCLIENTREPO"
else
  echo "rest client dep directory does NOT exist. It will be created and checked out"
  sh -c "mkdir dep && cd dep && git clone $RESTCLIENTREPO"
fi
echo "** runing autogen.sh in dep/rest-client-c"
sh -c "cd dep/rest-client-c && ./autogen.sh"


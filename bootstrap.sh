#!/bin/bash

# test if build dir exists
if [ ! -d "build" ]; then
  mkdir build
# delete build dir if it exists
else 
  rm -rf build
fi

pushd build
conan install ..

if [ $? -ne 0 ]; then
  popd
  exit 1
fi

cmake .. -G "Unix Makefiles"

if [ $? -ne 0 ]; then
  popd
  exit 1
fi

popd

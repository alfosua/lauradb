#!/bin/bash

# delete build dir if exists
if [ -d "build" ]; then
  echo "cleaning..."
  rm -rf build
fi

mkdir build
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

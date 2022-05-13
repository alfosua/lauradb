#!/bin/bash

cmake --build build

# if first argument is --run, run the executable
if [ "$1" = "--run" ]; then
  ./build/bin/lauradb
fi
#!/bin/bash

if [ "$1" == "-c" ]; then
    make
fi

./build/test
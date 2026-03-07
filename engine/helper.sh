#!/bin/bash

ENGINE_DIR=./engine
ENGINE_EXE=rcengine

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)

cd $SCRIPT_DIR

case "$1" in
    "init-release")
        rm -rf build
        set -e
        mkdir build
        cd build
        cmake -DCMAKE_BUILD_TYPE=Release ..
        ;;
    "init-debug")
        rm -rf build
        set -e
        mkdir build
        cd build
        cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
        cp -f compile_commands.json ../compile_commands.json
        ;;
    "build")
        set -e
        cd build
        cmake --build .
        cp -f bin/$ENGINE_EXE ../$ENGINE_EXE
        rm bin/$ENGINE_EXE
        ;;
    "fast-build")
        rm -rf build
        set -e
        mkdir build
        cd build
        cmake -DCMAKE_BUILD_TYPE=Release ..
        cmake --build .
        cp -f bin/$ENGINE_EXE ../$ENGINE_EXE
        rm -rf bin
        ;;
    "*")
        echo "Invalid argument $1, valid argument is init-release, init-debug, build, or fast-build."
        ;;
esac

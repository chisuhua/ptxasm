#!/bin/bash
SCRIPT_DIR=$(cd $(dirname "${BASH_SOURCE[0]}") >/dev/null && pwd)

if [ -z "${CMAKE_INSTALL_PREFIX}" ]; then
    export CMAKE_INSTALL_PREFIX="$OPUTOOL_BUILD"
fi

if [ -z "${ANTLR_JAR}" ]; then
    export ANTLR_JAR="$SCRIPT_DIR/../3rdparty/antlr4/antlr-4.13.2-complete.jar"
fi

if [ -z "${ANTLR_RUNTIME_DIR}" ]; then
    export ANTLR_RUNTIME_DIR="$SCRIPT_DIR/../3rdparty/antlr4/src/runtime/src"
fi


CMD="build"

for arg in "$@"; do
  if [ "$arg" == "-n" ]; then
    NORUN=true
  elif [ "$arg" == "-force" ]; then
    FORCE=true
  elif [ "$arg" == "clean" ]; then
    CMD="clean"
  elif echo "${BUILD_OPT[@]}" | grep -wq "$arg"; then
    if [ $arg == dbg ]; then
      BUILD_TYPE=Debug
    elif [ $arg == rel ]; then
      BUILD_TYPE=Release
    elif [ $arg == reldbg ]; then
      BUILD_TYPE=RelWithDebInfo
    fi
    CMD="build"
  elif echo "${OPT[@]}" | grep -wq "$arg"; then
    BUILD_ARG+="$arg "
  else
    Usage
  fi
done

if [ $CMD == "build" ]; then
    rm -rf build
    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=$CMAKE_INSTALL_PREFIX -DCMAKE_BUILD_TYPE=$BUILD_TYPE  ..
    make -j 8
    make install
elif [ $CMD == "clean" ]; then
    rm -rf build
    rm -rf src
fi


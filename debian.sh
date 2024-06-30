#!/bin/bash

sdk_config="Release"
readonly sdk_config

sdk_dir="/home/jmh/github/VTKCMake/install/lib/cmake/vtk"
(
    source "$HOME/github/emsdk/emsdk_env.sh"
    export CC=/usr/bin/clang-16
    export CXX=/usr/bin/clang++-16
    emcmake cmake -GNinja -S. -B./out -DCMAKE_BUILD_TYPE=$sdk_config -DVTK_DIR=$sdk_dir
    cmake --build ./out
)

npm i
npm run build

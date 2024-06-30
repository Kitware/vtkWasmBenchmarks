#!/bin/sh

set -e
set -x

# wasm64-threads-v9.3.0-3223-g6e1fa5e1fa-20240604
# wasm32-v9.3.0-3223-g6e1fa5e1fa-20240604
sdk_version="wasm32-v9.3.0-3223-g6e1fa5e1fa-20240604"
#sdk_version="v9.3.0-2490-g97c85d6930-20240401"
readonly sdk_version

sdk_image="kitware/vtk-wasm-sdk"
readonly sdk_image

sdk_config="Release"
readonly sdk_config

sdk_dir="/VTK-install/$sdk_config/lib/cmake/vtk"
readonly sdk_dir

docker run \
    --rm \
    -u "$(id -u):$(id -g)" \
    -v"$PWD":/vtkWasmBenchmarks \
    $sdk_image:$sdk_version \
    emcmake cmake -GNinja -S /vtkWasmBenchmarks -B /vtkWasmBenchmarks/out -DCMAKE_BUILD_TYPE=$sdk_config -DVTK_DIR=$sdk_dir

docker run \
    --rm \
    -v"$PWD":/vtkWasmBenchmarks \
    $sdk_image:$sdk_version \
    cmake --build /vtkWasmBenchmarks/out

npm i

npm run build

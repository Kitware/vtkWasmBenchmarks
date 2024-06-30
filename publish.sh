#!/bin/sh

rm -Rf ./public/vtkWasmBenchmarks
cp -r dist public/vtkWasmBenchmarks
cp -r dist/index.html public

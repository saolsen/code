#!/bin/bash

pushd cmake
emcmake cmake -S . -B cmake-web -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-web --target tazar
pushd cmake-web
mv tazar.html index.html
mkdir tazar-web
cp tazar.js tazar-web/
cp azar.wasm tazar-web/
cp index.html tazar-web/
zip -r tazar-web.zip tazar-web
popd
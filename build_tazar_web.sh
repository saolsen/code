#!/bin/bash

pushd cmake || exit
emcmake cmake -S . -B cmake-web -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-web --target tazar-v1
pushd cmake-web || exit
mv tazar-v1.html index.html
mkdir tazar-web
cp tazar-v1.js tazar-web/
cp tazar-v1.wasm tazar-web/
cp index.html tazar-web/
zip -r tazar-web.zip tazar-web
popd || exit

~/emsdk/upstream/emscripten/emcc -o ./out/appWASM.js ./src/main.c ./src/context.c -O3 -s ALLOW_MEMORY_GROWTH=0 -s USE_WEBGL2=1 -s FULL_ES3=1 -s WASM=1 -s NO_EXIT_RUNTIME=1 -std=gnu17 -s LLD_REPORT_UNDEFINED -s EXPORTED_FUNCTIONS='["_createContext"]' -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]'

all:
	g++ stl2gltf.cpp -std=c++11 -stdlib=libc++ -I .
release:
	g++ stl2gltf.cpp -std=c++11 -stdlib=libc++ -O3 -I .
em:
	em++ stl2gltf.cpp -std=c++11 -stdlib=libc++-s -s DEMANGLE_SUPPORT=1 -I .  -s EXPORTED_FUNCTIONS='["_make_bin"]' -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "FS_createDataFile", "FS_readFile"]'  -s TOTAL_MEMORY=128MB

em_release:
	em++ stl2gltf.cpp -std=c++11 -stdlib=libc++-s -s DEMANGLE_SUPPORT=1 -I .  -s EXPORTED_FUNCTIONS='["_make_bin"]' -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "FS_createDataFile", "FS_readFile"]' -O3  -s TOTAL_MEMORY=128MB

wasm:
	em++ stl2gltf.cpp -std=c++11 -stdlib=libc++-s -s DEMANGLE_SUPPORT=1 -I .  -s EXPORTED_FUNCTIONS='["_make_bin"]' -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "FS_createDataFile", "FS_readFile"]' -s TOTAL_MEMORY=128MB -s WASM=1

wasm_release:
	em++ stl2gltf.cpp -std=c++11 -stdlib=libc++-s -s DEMANGLE_SUPPORT=1 -I .  -s EXPORTED_FUNCTIONS='["_make_bin"]' -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "FS_createDataFile", "FS_readFile"]' -O3  -s TOTAL_MEMORY=128MB -s WASM=1

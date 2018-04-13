EM_CXX_FLAG := -s DEMANGLE_SUPPORT=1 -s EXPORTED_FUNCTIONS='["_make_bin"]' -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "FS_createDataFile", "FS_readFile", "FS_unlink"]' -s ALLOW_MEMORY_GROWTH=1
O3 := -O3
WASM := -s WASM=1

all:
	g++ stl2gltf.cpp -std=c++11 -I .
release:
	g++ stl2gltf.cpp -std=c++11 -I . ${O3}
em:
	em++ stl2gltf.cpp -std=c++11 -I . ${EM_CXX_FLAG}
em_release:
	em++ stl2gltf.cpp -std=c++11 -I . ${EM_CXX_FLAG} ${O3}
wasm:
	em++ stl2gltf.cpp -std=c++11 -I . ${EM_CXX_FLAG} ${WASM}
wasm_release:
	em++ stl2gltf.cpp -std=c++11 -I . ${EM_CXX_FLAG} ${O3} ${WASM}

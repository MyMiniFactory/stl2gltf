all:
	g++ stl2gltf.cpp -std=c++11 -stdlib=libc++
release:
	g++ stl2gltf.cpp -std=c++11 -stdlib=libc++ -O3

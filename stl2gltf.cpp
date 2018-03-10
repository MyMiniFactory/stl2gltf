#include <string>
#include <fstream>
#include <iostream>
#include <math.h>       /* floor */
#include <array>
#include <vector>
#include <unordered_map>

#include <chrono>

int main( int argc, char *argv[] )
{
    auto t1 = std::chrono::high_resolution_clock::now();

    std::string filepath = "meshes/perfect.stl";

    if (argc < 2) {
        printf("not stl file given use default \n");
    } else {
        filepath = argv[1];
    }

    std::fstream fbin;
    fbin.open(filepath.c_str(), std::ios::in | std::ios::binary);
    fbin.seekg(80);

    std::uint32_t num_faces;
    fbin.read(reinterpret_cast<char *>(&num_faces), 4);

    struct ArrayHasher {
        std::size_t operator() (const std::array<float, 3> & a) const {
            std::size_t h = 0;
            for (auto e : a) {
                h ^= std::hash<float>{}(e) + 0x9e3779b9 + (h<<6) + (h>>2);
            }
            return h;
        }
    };

    auto tread = std::chrono::high_resolution_clock::now();

    std::unordered_map< std::array<float, 3>, int, ArrayHasher> _vertices;
    std::vector<std::uint32_t> indices;

    size_t len = num_faces*50;
    char *ret = new char[len];
    fbin.read(ret, len);
    std::vector<std::array<float, 3>> all_vertices;

    float tiger = 1.;

    for (int i=0;i<num_faces;i+=1) {
        for (int j=0;j<3;j+=1) {
            std::array<float, 3> v;
            memcpy(&v, &ret[12 + i*50 + j*12], 4*3);
            all_vertices.push_back(v);
        }
    }

    std::vector<float> vertices;
    int vertice_counter = 0;
    for (int i=0;i<all_vertices.size();++i) {
        std::array<float, 3> v = all_vertices[i];

        auto got = _vertices.find(v);
        if ( got == _vertices.end() ) { // not found
            _vertices[v] = vertice_counter;
            indices.push_back(vertice_counter);
            vertice_counter++;
            vertices.insert(vertices.end(), std::begin(v), std::end(v));

        } else { // found
            indices.push_back(_vertices[v]);
        }

    }

    auto tread_end = std::chrono::high_resolution_clock::now();
    std::cout << "calculation total took "
        << std::chrono::duration_cast<std::chrono::milliseconds>(tread_end-tread).count()
        << " milliseconds\n";

    auto twrite = std::chrono::high_resolution_clock::now();

    std::fstream out_bin;
    out_bin.open("out.bin", std::ios::out | std::ios::binary);

    const int num_indices = indices.size();
    const int num_vertices = vertices.size();

    for (int i=0; i < num_indices; i++) {
        out_bin.write((char*)&indices[i], 4);
    }

    for (int i=0; i < num_vertices; i++) {
        out_bin.write((char*)&vertices[i], 4);
    }

    auto twrite_end = std::chrono::high_resolution_clock::now();
    std::cout << "write total took "
        << std::chrono::duration_cast<std::chrono::milliseconds>(twrite_end-twrite).count()
        << " milliseconds\n";

    printf("number faces %u\n", num_faces);
    printf("num indices %d\n", num_indices);
    printf("num vertices %d\n", num_vertices);
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "total took "
              << std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count()
              << " milliseconds\n";
}

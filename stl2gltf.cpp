#include <string>
#include <fstream>
#include <math.h>       /* floor */
#include <array>
#include <vector>
#include <unordered_map>

int main( int argc, char *argv[] )
{
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

    struct ArrayHasher { // do it directly on point3f
        std::size_t operator() (const std::array<float, 3> & a) const {
            std::size_t h = 0;
            for (auto e : a) {
                h ^= std::hash<float>{}(e) + 0x9e3779b9 + (h<<6) + (h>>2);
            }
            return h;
        }
    };

    std::unordered_map< std::array<float, 3>, int, ArrayHasher> _vertices;
    std::vector<float> vertices;
    std::vector<std::uint32_t> indices;

    int vertice_counter = 0;
    for (int i=0;i<num_faces;++i) {
        fbin.seekg(4*3, std::ios_base::cur); // skip normal
        for (int j=0;j<3;++j) {
            float x;
            float y;
            float z;
            fbin.read(reinterpret_cast<char *>(&x), 4);
            fbin.read(reinterpret_cast<char *>(&y), 4);
            fbin.read(reinterpret_cast<char *>(&z), 4);

            // printf("%f %f %f\n", x, y, z);

            const std::array<float, 3> v = {x, y, z};

            auto got = _vertices.find(v);
            if ( got == _vertices.end() ) { // not found
                _vertices[v] = vertice_counter;
                indices.push_back(vertice_counter);
                vertice_counter++;
                vertices.insert(vertices.end(), {x, y, z});

                // printf("vertice_counter %i\n", vertice_counter);
                // printf("v counter %i\n", _vertices[v]);
            } else { // found
                indices.push_back(_vertices[v]);
                // printf("%i\n", vertices[v]);
            }
        }
        fbin.seekg(2, std::ios_base::cur); // skip spacer

    }

    std::fstream out_bin;
    out_bin.open("out.bin", std::ios::out | std::ios::binary);

    const int num_indices = sizeof(indices);
    const int num_vertices = sizeof(num_vertices);

    for (int i; i < num_vertices; i++) {
        out_bin.write((char*)&vertices[i], 4);
    }

    for (int i; i < num_indices; i++) {
        out_bin.write((char*)&indices[i], 4);
    }


    printf("number faces %u\n", num_faces);
    printf("num indices %d\n", sizeof(indices));
    printf("num vertices %d\n", vertices.size());
}

#include <string>
#include <fstream>
#include <array>
#include <vector>
#include <cstring>

#include <vertex.h>

#include <algorithm>
#include <iostream>

// https://stackoverflow.com/a/6500499
inline std::string trim(std::string& str)
{
    str.erase(0, str.find_first_not_of(' '));       //prefixing spaces
    str.erase(str.find_last_not_of(' ')+1);         //surfixing spaces
    return str;
}

inline Vertex get_vector(std::string& str)
{
    // "vertex float float float"
    auto space0 = str.find_first_of(' ');
    str.erase(0, space0); // remove "vertex"
    str.erase(0, str.find_first_not_of(' ')); //prefixing spaces
    auto space1 = str.find_first_of(' ');

    float x = std::stof(str.substr(0, space1));

    str.erase(0, space1+1); // remove x
    str.erase(0, str.find_first_not_of(' ')); //prefixing spaces

    auto space2 = str.find_last_of(' ');

    Vertex v(x,
        std::stof(str.substr(0, space2)),
        std::stof(str.substr(space2, str.size()))
    );
    return v;
}

std::vector<Vertex> load_binary(const std::string filepath) {
    printf("loading binary\n");
    std::fstream fbin;
    fbin.open(filepath.c_str(), std::ios::in | std::ios::binary);
    fbin.seekg(80);

    std::uint32_t num_faces;
    fbin.read(reinterpret_cast<char *>(&num_faces), 4);

    const unsigned int num_indices = num_faces*3;

    size_t len = num_faces*50;
    char *ret = new char[len];
    fbin.read(ret, len);
    std::vector<Vertex> all_vertices(num_indices);

    for (int i=0;i<num_faces;i+=1) {
        for (int j=0;j<3;j++) {
            const int index = i*3+j;
            std::memcpy(&all_vertices[index], &ret[12 + i*50 + j*12], 12);
        }
    }

    fbin.close();

    return all_vertices;
}

std::vector<Vertex> load_binary(uint8_t* buf) {
    printf("loading binary from byteArray\n");

    std::uint32_t num_faces;
    std::memcpy(&num_faces, &buf[80], 4);
    printf("num faces %d\n", num_faces);

    const unsigned int num_indices = num_faces*3;
    std::vector<Vertex> all_vertices(num_indices);

    for (int i=0;i<num_faces;i+=1) {
        for (int j=0;j<3;j++) {
            const int index = i*3+j;
            const int position = 84 + 12 + i*50 + j*12;
            // if (position + 12 >= num_faces*50 + 84) {
                // printf("index ??  %d\n", position);
            // }
            std::memcpy(&all_vertices[index], &buf[position], 12);
        }
    }
    printf("all vertices  %d\n", all_vertices.size());

    return all_vertices;
}

std::vector<Vertex> load_stl(uint8_t* buf) {
    return load_binary(buf);
}

std::vector<Vertex> load_ascii(const std::string filepath) {
    printf("loading ascii\n");

    std::vector<Vertex> all_vertices;

    std::ifstream file;
    file.open(filepath.c_str());

    std::string line;
    while (!file.eof()) {
        std::getline(file, line);
        line = trim(line);
        if (line.rfind("vertex", 0) == 0) {
            all_vertices.push_back(get_vector(line));
        }
    }
    file.close();

    return all_vertices;
}

// thanks to https://github.com/mkeeter/fstl/blob/master/src/loader.cpp
std::vector<Vertex> load_stl(const std::string filepath) {
    std::ifstream file(filepath);
    std::string line;
    std::getline(file, line);
    if (line.rfind("solid ", 0) == 0) {
        std::getline(file, line);
        line = trim(line);
        if (line.rfind("facet", 0) == 0)
        {
            file.close();
            return load_ascii(filepath);
        }
    }
    file.close();
    return load_binary(filepath);
}

extern "C" {

// void new_make_bin(uint8_t* buf) {

    // auto all_vertices = load_stl(buf);
    // printf("new make bin size unsigned char %d\n", size);
    // printf("%d ", buf[0]);
    // for (int i=0;i<100;i++) {
        // printf("%d ", buf[i]);
    // }
    // printf("\nfinished\n");
    // load_binary(buf, size);
// }
// void make_bin(const std::string filepath) {
void make_bin(uint8_t* buf) {

    auto all_vertices = load_stl(buf);
    const uint32_t num_indices = all_vertices.size();
    for (int c=0;c<all_vertices.size();c++)
        all_vertices[c].i = c;

    std::uint32_t *indices;
    indices = (std::uint32_t *) malloc(num_indices * sizeof(std::uint32_t));

    std::sort(all_vertices.begin(), all_vertices.end());

    float minx =  999999;
    float miny =  999999;
    float minz =  999999;
    float maxx = -999999;
    float maxy = -999999;
    float maxz = -999999;

    unsigned int num_vertices = 0;
    for (auto v : all_vertices)
    {
        if (!num_vertices || v != all_vertices[num_vertices-1])
        {
            all_vertices[num_vertices++] = v;
            if (v.x < minx) minx = v.x;
            if (v.x > maxx) maxx = v.x;
            if (v.y < miny) miny = v.y;
            if (v.y > maxy) maxy = v.y;
            if (v.z < minz) minz = v.z;
            if (v.z > maxz) maxz = v.z;
        }
        indices[v.i] = num_vertices - 1;
    }
    all_vertices.resize(num_vertices);

    // auto twrite = std::chrono::high_resolution_clock::now();

    ////////////////////// calculation done start writing ////////////////////
// total_blength, indices_blength, vertices_boffset, vertices_blength,
// number_indices, number_vertices, minx, miny, minz, maxx, maxy, maxz

    const uint32_t indices_bytelength = num_indices*4;
    const uint32_t vertices_bytelength = num_vertices*3*4;

    std::fstream out_bin;
    out_bin.open("out.bin", std::ios::out | std::ios::binary);


    for (int i=0; i < num_indices; i++) {
        out_bin.write((char*)&indices[i], 4);
    }

    // TODO: make sure padding is not necessary
    // since indices bytelength are multiple of u32int

    //const int padding_bytelength = (num_indices*4 + 3) & ~3 - num_indices*4;
    //for (int i=0; i < padding_bytelength; i++) {
        //out_bin.write(" ", 1); // this looks like problem
    //}

    for (int i=0; i < num_vertices; i++) {
        out_bin.write((char*)&all_vertices[i], 12);
    }
    out_bin.close();

    free(indices);

    // auto twrite_end = std::chrono::high_resolution_clock::now();

    // printf("write total took %lld milliseconds\n", std::chrono::duration_cast<std::chrono::milliseconds>(twrite_end-twrite).count());

    printf("num indices %d\n", num_indices);
    printf("num vertices %d\n", num_vertices);

    // total_blength, indices_blength, vertices_boffset, vertices_blength,
    // number_indices, number_vertices, minx, miny, minz, maxx, maxy, maxz
    std::fstream gltf_data;
    gltf_data.open("data.txt", std::ios::out);
    gltf_data << num_indices << " ";
    gltf_data << num_vertices << " ";
    gltf_data << indices_bytelength << " ";
    gltf_data << vertices_bytelength << " ";
    gltf_data << indices_bytelength + vertices_bytelength << " ";
    gltf_data << minx << " ";
    gltf_data << miny << " ";
    gltf_data << minz << " ";
    gltf_data << maxx << " ";
    gltf_data << maxy << " ";
    gltf_data << maxz;
    gltf_data.close();

    printf("boundary %f %f %f\n", minx, miny, minz);
    printf("boundary %f %f %f\n", maxx, maxy, maxz);
}
}


int main( int argc, char *argv[] )
{
    // std::string filepath = "meshes/perfect.stl";

    // if (argc < 2) {
        // printf("not stl file given use default \n");
    // } else {
        // filepath = argv[1];
    // }

    // make_bin(filepath);
}

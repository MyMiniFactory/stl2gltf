#include <string>
#include <fstream>
#include <math.h>       /* floor */
#include <array>
#include <vector>
#include <unordered_map>

#include <vertex.h>

#include <chrono>
#include <assert.h>

extern "C" {

int* make_bin(const std::string filepath, int* export_data, float* boundary) {
    printf("memory address for export data %d\n", export_data);
    // number_indices, number_vertices, minx, miny, minz, maxx, maxy, maxz
    auto t1 = std::chrono::high_resolution_clock::now();

    std::fstream fbin;
    fbin.open(filepath.c_str(), std::ios::in | std::ios::binary);
    fbin.seekg(80);

    std::uint32_t num_faces;
    fbin.read(reinterpret_cast<char *>(&num_faces), 4);

    const unsigned int num_indices = num_faces*3;
    auto tread = std::chrono::high_resolution_clock::now();

    std::uint32_t *indices;
    indices = (std::uint32_t *) malloc(num_indices * sizeof(std::uint32_t));

    size_t len = num_faces*50;
    char *ret = new char[len];
    fbin.read(ret, len);
    std::vector<Vertex> all_vertices(num_indices);
    // indices = (Vertex *) malloc(sizeof(Vertex) * num_indices);

    float minx =  999999;
    float miny =  999999;
    float minz =  999999;
    float maxx = -999999;
    float maxy = -999999;
    float maxz = -999999;

    for (int i=0;i<num_faces;i+=1) {
        for (int j=0;j<3;j++) {
            const int index = i*3+j;
            memcpy(&all_vertices[index], &ret[12 + i*50 + j*12], 12);
            all_vertices[index].i = index;
            // printf("v %f %f %f\n", all_vertices[index].x, all_vertices[index].y, all_vertices[index].z);
        }
    }

    std::sort(all_vertices.begin(), all_vertices.end());

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

    auto tread_end = std::chrono::high_resolution_clock::now();
    printf("calculation total took %lld milliseconds\n", std::chrono::duration_cast<std::chrono::milliseconds>(tread_end-tread).count());

    auto twrite = std::chrono::high_resolution_clock::now();

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

    // TODO: not tested
    const int padding_bytelength = (num_indices*4 + 3) & ~3 - num_indices*4;
    for (int i=0; i < padding_bytelength; i++) {
        out_bin.write(" ", 1); // this looks like problem
    }
    // assert(padding_bytelength == 0);

    for (int i=0; i < num_vertices; i++) {
        out_bin.write((char*)&all_vertices[i], 12);
    }

    free(indices);

    auto twrite_end = std::chrono::high_resolution_clock::now();

    printf("write total took %lld milliseconds\n", std::chrono::duration_cast<std::chrono::milliseconds>(twrite_end-twrite).count());

    printf("number faces %u\n", num_faces);
    printf("num indices %d\n", num_indices);
    printf("num vertices %d\n", num_vertices);

// total_blength, indices_blength, vertices_boffset, vertices_blength,
// number_indices, number_vertices, minx, miny, minz, maxx, maxy, maxz
    export_data[0] = num_indices;
    export_data[1] = num_vertices;
    export_data[2] = indices_bytelength + padding_bytelength;
    export_data[3] = vertices_bytelength;
    export_data[4] = indices_bytelength + padding_bytelength + vertices_bytelength;

    printf("boundary %f %f %f", minx, miny, minz);
    printf("boundary %f %f %f", maxx, maxy, maxz);
    boundary[0] = minx; boundary[1] = miny; boundary[2] = minz;
    boundary[3] = maxx; boundary[4] = maxy; boundary[5] = maxz;

    auto t2 = std::chrono::high_resolution_clock::now();

    printf("total took %lld milliseconds\n", std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count());


    printf("memory address for export data %d\n", export_data);
    return export_data;
}
}


int main( int argc, char *argv[] )
{
    std::string filepath = "meshes/perfect.stl";

    if (argc < 2) {
        printf("not stl file given use default \n");
    } else {
        filepath = argv[1];
    }

    // int data[2] = {0, 0};
    // make_bin(filepath, data);

}

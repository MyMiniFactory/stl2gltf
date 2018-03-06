import os

def stl_to_gltf(binary_stl_path, path_to_gltf_folder):
    import struct

    gltf2 = '''
    {
      "scenes" : [
        {
          "nodes" : [ 0 ]
        }
      ],
      
      "nodes" : [
        {
          "mesh" : 0
        }
      ],

      "meshes" : [
        {
          "primitives" : [ {
            "attributes" : {
              "POSITION" : 0
            }
          } ]
        }
      ],

      "buffers" : [
        {
          "uri" : "out.bin",
          "byteLength" : %d
        }
      ],
      "bufferViews" : [
        {
          "buffer" : 0,
          "byteOffset" : 0,
          "byteLength" : %d,
          "target" : 34962
        }
      ],
      "accessors" : [
        {
          "bufferView" : 0,
          "componentType" : 5126,
          "count" : %d,
          "type" : "VEC3",
          "min" : [ %f, %f, %f ],
          "max" : [ %f, %f, %f ]
        }
      ],
      
      "asset" : {
        "version" : "2.0"
      }
    }
    '''

    header_bytes = 80
    long_int_bytes = 4
    float_bytes = 4
    vec3_bytes = 4 * 3
    spacer_bytes = 2
    num_vertices_in_face = 3

    f = open(path_to_stl, "rb")
    out_path = os.path.join(path_to_gltf_folder, "out.bin")
    o = open(out_path, "wb")

    f.seek(header_bytes) # skip 80 bytes headers

    num_faces_bytes = f.read(long_int_bytes)

    number_faces = int.from_bytes(num_faces_bytes, byteorder='little')

    # the second vec3_bytes is for normal
    total_bytes = header_bytes + long_int_bytes + number_faces * (num_vertices_in_face * vec3_bytes + vec3_bytes + spacer_bytes) 

    assert total_bytes == os.path.getsize(path_to_stl), "stl is not binary or ill formatted"

    vertices_count = number_faces * num_vertices_in_face # each faces has 3 vertices
    byteLength = vertices_count * vec3_bytes # each vec3 has 3 floats, each float is 4 bytes

    minx, maxx = [9999999, -9999999]
    miny, maxy = [9999999, -9999999]
    minz, maxz = [9999999, -9999999]

    for i in range(number_faces):
        f.seek(vec3_bytes, 1) # skip the normals
        for i in range(num_vertices_in_face): # 3 vertices for each face
            # print(struct.unpack('f', f.read(4)))

            x = f.read(float_bytes); o.write(x);
            y = f.read(float_bytes); o.write(y);
            z = f.read(float_bytes); o.write(z);

            x = struct.unpack('f', x)[0]
            y = struct.unpack('f', y)[0]
            z = struct.unpack('f', z)[0]

            if x < minx: minx = x
            if x > maxx: maxx = x
            if y < miny: miny = y
            if y > maxy: maxy = y
            if z < minz: minz = z
            if z > maxz: maxz = z

        f.seek(spacer_bytes, 1) # skip the spacer

    out_path = os.path.join(path_to_gltf_folder, "out.gltf")
    o_gltf = open(out_path, "w")

    o_gltf.write(gltf2 % (
        byteLength, byteLength, vertices_count,
        minx, miny, minz,
        maxx, maxy, maxz)
    )


if __name__ == '__main__':
    import sys

    if len(sys.argv) < 3:
        print("use it like python3 stl_to_gltf.py /path/to/stl /path/to/gltf/folder")
        sys.exit(1)

    path_to_stl = sys.argv[1]
    path_to_gltf_folder = sys.argv[2]

    if not os.path.exists(path_to_stl):
        print("stl file does not exists %s" % path_to_stl)

    if not os.path.isdir(path_to_gltf_folder):
        os.mkdir(path_to_gltf_folder)

    stl_to_gltf(path_to_stl, path_to_gltf_folder)


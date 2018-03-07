import os

def stl_to_gltf(binary_stl_path, out_path, is_binary):
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
          "POSITION" : 1
        },
        "indices" : 0
      } ]
    }
  ],

  "buffers" : [
    {
      %s
      "byteLength" : %d
    }
  ],
  "bufferViews" : [
    {
      "buffer" : 0,
      "byteOffset" : 0,
      "byteLength" : %d,
      "target" : 34963
    },
    {
      "buffer" : 0,
      "byteOffset" : %d,
      "byteLength" : %d,
      "target" : 34962
    }
  ],
  "accessors" : [
    {
      "bufferView" : 0,
      "byteOffset" : 0,
      "componentType" : 5123,
      "count" : %d,
      "type" : "SCALAR",
      "max" : [ %d ],
      "min" : [ 0 ]
    },
    {
      "bufferView" : 1,
      "byteOffset" : 0,
      "componentType" : 5126,
      "count" : %d,
      "type" : "VEC3",
      "min" : [%f, %f, %f],
      "max" : [%f, %f, %f]
    }
  ],

  "asset" : {
    "version" : "2.0"
  }
}
'''

    header_bytes = 80
    long_int_bytes = 4
    unsigned_int_bytes = 2
    float_bytes = 4
    vec3_bytes = 4 * 3
    spacer_bytes = 2
    num_vertices_in_face = 3

    f = open(path_to_stl, "rb")

    if not is_binary:
        out_bin = os.path.join(out_path, "out.bin")
    else:
        out_bin = out_path

    f.seek(header_bytes) # skip 80 bytes headers

    num_faces_bytes = f.read(long_int_bytes)

    number_faces = int.from_bytes(num_faces_bytes, byteorder='little')

    number_vertices = number_faces * num_vertices_in_face # each faces has 3 vertices

    if number_vertices > 65535:
        print("too many vertices {} would not work, exiting".format(number_vertices))
        sys.exit(1)

    vertices_bytelength = number_vertices * vec3_bytes # each vec3 has 3 floats, each float is 4 bytes
    unpadded_indices_bytelength = number_vertices * unsigned_int_bytes
    indices_bytelength = (unpadded_indices_bytelength + 3) & ~3

    # the vec3_bytes is for normal
    stl_assume_bytes = header_bytes + long_int_bytes + number_faces * (vec3_bytes + spacer_bytes) + vertices_bytelength
    assert stl_assume_bytes == os.path.getsize(path_to_stl), "stl is not binary or ill formatted"

    out_bin_bytelength = indices_bytelength + vertices_bytelength


    minx, maxx = [9999999, -9999999]
    miny, maxy = [9999999, -9999999]
    minz, maxz = [9999999, -9999999]

    with open(out_bin, "wb") as o:
        for i in range(number_vertices): # TODO: padding spaces for vertices
            o.write(struct.pack('<H', i));

        for i in range(indices_bytelength - unpadded_indices_bytelength):
            o.write(b' ')


        for i in range(number_faces):
            f.seek(vec3_bytes, 1) # skip the normals
            for i in range(num_vertices_in_face): # 3 vertices for each face
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

    assert os.path.getsize(out_bin) == out_bin_bytelength

    if is_binary:
        out_bin_uir = ""
    else:
        out_bin_uir = '"uri": "out.bin",'

    gltf2 = gltf2 % ( out_bin_uir,

                #buffer
                out_bin_bytelength,

                # bufferViews[0]
                indices_bytelength,

                # bufferViews[1]
                indices_bytelength,
                vertices_bytelength,

                # accessors[0]
                number_vertices,
                number_vertices - 1,

                # accessors[1]
                number_vertices,
                minx, miny, minz,
                maxx, maxy, maxz)

    if is_binary:

        gltf2 = gltf2.replace(" ", "")
        gltf2 = gltf2.replace("\n", "")

        glb_out = bytearray()

        scene = bytearray(gltf2.encode())

        scene_len = len(scene)
        padded_scene_len = (scene_len + 3) & ~3
        body_offset = padded_scene_len + 12 + 8

        file_len = body_offset + out_bin_bytelength + 8

        # 12-byte header
        glb_out.extend(struct.pack('<I', 0x46546C67)) # magic number for glTF
        glb_out.extend(struct.pack('<I', 2))
        glb_out.extend(struct.pack('<I', file_len))

        # chunk 0
        glb_out.extend(struct.pack('<I', padded_scene_len))
        glb_out.extend(struct.pack('<I', 0x4E4F534A)) # magic number for JSON
        glb_out.extend(scene)

        while len(glb_out) < body_offset:
            glb_out.extend(b' ')

        # chunk 1
        glb_out.extend(struct.pack('<I', out_bin_bytelength))
        glb_out.extend(struct.pack('<I', 0x004E4942)) # magin number for BIN

        with open(out_bin, "rb") as out:
            b = out.read()

        glb_out.extend(b)

        with open(out_bin, "wb") as out:
            out.write(glb_out)

        # with open(out_bin, "rb") as out:
            # b = out.read()
            # print(b)
    else:
        out_file = os.path.join(out_path, "out.gltf")
        o_gltf = open(out_file, "w")
        o_gltf.write(gltf2)


if __name__ == '__main__':
    import sys

    if len(sys.argv) < 3:
        print("use it like python3 stl_to_gltf.py /path/to/stl /path/to/gltf/folder")
        print("or          python3 stl_to_gltf.py /path/to/stl /path/to/glb/file -b")
        sys.exit(1)

    path_to_stl = sys.argv[1]
    out_path = sys.argv[2]
    if len(sys.argv) > 3:
        is_binary = True
    else:
        is_binary = False

    if out_path.lower().endswith(".glb"):
        print("Use binary mode since output file has glb extension")
        is_binary = True
    else:
        if is_binary:
            print("output file should have glb extension but not %s", out_path)

    if not os.path.exists(path_to_stl):
        print("stl file does not exists %s" % path_to_stl)

    if not is_binary:
        if not os.path.isdir(out_path):
            os.mkdir(out_path)

    stl_to_gltf(path_to_stl, out_path, is_binary)


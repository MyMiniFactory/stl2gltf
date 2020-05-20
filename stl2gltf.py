import struct


HEADER_BYTES = 80
UNSIGNED_LONG_INT_BYTES = 4
FLOAT_BYTES = 4
VEC3_BYTES = 4 * 3
SPACER_BYTES = 2
GLFT2_TEMPLATE = """{"scenes":[{"nodes":[0]}],"nodes":[{"mesh":0}],"meshes":[{"primitives":[{"attributes":{"POSITION":1},"indices":0}]}],"buffers":[{%s"byteLength":%d}],"bufferViews":[{"buffer":0,"byteOffset":0,"byteLength":%d,"target":34963},{"buffer":0,"byteOffset":%d,"byteLength":%d,"target":34962}],"accessors":[{"bufferView":0,"byteOffset":0,"componentType":5125,"count":%d,"type":"SCALAR","max":[%d],"min":[0]},{"bufferView":1,"byteOffset":0,"componentType":5126,"count":%d,"type":"VEC3","min":[%f,%f,%f],"max":[%f,%f,%f]}],"asset":{"version":"2.0"}}"""


def stl_to_gltf(stlio):
    """
    Converts an STL format to GLTF format
    @params: stlio -> file object for the stl file
    @returns: glbio -> file object for the gltf file
    """
    vertices = {}
    indices = []

    unpack_face = struct.Struct("<12fH").unpack
    face_bytes = FLOAT_BYTES * 12 + 2

    # start parse STL file

    stlio.seek(HEADER_BYTES) # skip 80 bytes headers

    num_faces_bytes = stlio.read(UNSIGNED_LONG_INT_BYTES)
    number_faces = struct.unpack("<I", num_faces_bytes)[0]

    # validate stl file
    stl_assume_bytes = HEADER_BYTES + UNSIGNED_LONG_INT_BYTES + number_faces * (VEC3_BYTES*3 + SPACER_BYTES + VEC3_BYTES)
    stlio.seek(0, 2) # for tell(), which gives the length of the file to that point
    assert stl_assume_bytes == stlio.tell(), "stl is not binary or ill formatted"
    stlio.seek(HEADER_BYTES + UNSIGNED_LONG_INT_BYTES)

    minx, maxx = [9999999, -9999999]
    miny, maxy = [9999999, -9999999]
    minz, maxz = [9999999, -9999999]

    vertices_length_counter = 0

    data = struct.unpack("<" + "12fH"*number_faces, stlio.read())
    len_data = len(data)

    for i in range(0, len_data, 13):
        for j in range(3, 12, 3):
            x, y, z = data[i+j:i+j+3]

            x = int(x*100000)/100000
            y = int(y*100000)/100000
            z = int(z*100000)/100000

            tuple_xyz = (x, y, z);

            try:
                indices.append(vertices[tuple_xyz])
            except KeyError:
                vertices[tuple_xyz] = vertices_length_counter
                vertices_length_counter += 1
                indices.append(vertices[tuple_xyz])
            
            if x < minx: minx = x
            if x > maxx: maxx = x
            if y < miny: miny = y
            if y > maxy: maxy = y
            if z < minz: minz = z
            if z > maxz: maxz = z

    # finish parse stl file

    number_vertices = len(vertices)
    number_indices = len(indices)
    vertices_bytelength = number_vertices * VEC3_BYTES # each vec3 has 3 floats, each float is 4 bytes
    unpadded_indices_bytelength = number_vertices * UNSIGNED_LONG_INT_BYTES
    unpadded_indices_bytelength = number_indices * UNSIGNED_LONG_INT_BYTES
    indices_bytelength = (unpadded_indices_bytelength + 3) & ~3
    out_bin_bytelength = vertices_bytelength + indices_bytelength
    out_bin_uir = ""

    GLFT2 = GLFT2_TEMPLATE % ( 
        out_bin_uir,
        #buffer
        out_bin_bytelength,

        # bufferViews[0]
        indices_bytelength,

        # bufferViews[1]
        indices_bytelength,
        vertices_bytelength,

        # accessors[0]
        number_indices,
        number_vertices - 1,

        # accessors[1]
        number_vertices,
        minx, miny, minz,
        maxx, maxy, maxz
    )

    glbio = bytearray()

    scene = bytearray(GLFT2.encode())

    scene_len = len(scene)
    padded_scene_len = (scene_len + 3) & ~3
    body_offset = padded_scene_len + 12 + 8

    file_len = body_offset + out_bin_bytelength + 8

    # 12-byte header
    glbio.extend(struct.pack('<I', 0x46546C67)) # magic number for glTF
    glbio.extend(struct.pack('<I', 2))
    glbio.extend(struct.pack('<I', file_len))

    # chunk 0
    glbio.extend(struct.pack('<I', padded_scene_len))
    glbio.extend(struct.pack('<I', 0x4E4F534A)) # magic number for JSON
    glbio.extend(scene)

    while len(glbio) < body_offset:
        glbio.extend(b' ')

    # chunk 1
    glbio.extend(struct.pack('<I', out_bin_bytelength))
    glbio.extend(struct.pack('<I', 0x004E4942)) # magin number for BIN

    glbio.extend(struct.pack('<%dI' % len(indices), *indices))

    for i in range(indices_bytelength - unpadded_indices_bytelength):
        glbio.extend(b' ')

    vertices = dict((v, k) for k,v in vertices.items())

    vertices = [vertices[i] for i in range(number_vertices)]
    flatten = lambda l: [item for sublist in l for item in sublist]

    glbio.extend(struct.pack('%df' % number_vertices*3, *flatten(vertices))) # magin number for BIN

    return glbio



if __name__ == '__main__':
    import sys

    if len(sys.argv) < 2:
        print("use it like python3 stl_to_gltf.py /path/to/stl /path/to/glbfile")
        sys.exit(1)

    path_to_stl = sys.argv[1]
    out_path = sys.argv[2]

    try:
        with open(path_to_stl, 'rb') as stlio:
            glbio = stl_to_gltf(stlio)

            with open(out_path, "wb") as out:
                out.write(glbio)
    except FileNotFoundError:
        print("stl file does not exists %s" % path_to_stl)

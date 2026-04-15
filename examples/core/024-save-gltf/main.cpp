/*****************************************************************************
 * VCLib                                                                     *
 * Visual Computing Library                                                  *
 *                                                                           *
 * Copyright(C) 2021-2026                                                    *
 * Visual Computing Lab                                                      *
 * ISTI - Italian National Research Council                                  *
 *                                                                           *
 * All rights reserved.                                                      *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the Mozilla Public License Version 2.0 as published *
 * by the Mozilla Foundation; either version 2 of the License, or            *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
 * Mozilla Public License Version 2.0                                        *
 * (https://www.mozilla.org/en-US/MPL/2.0/) for more details.                *
 ****************************************************************************/

#include <vclib/io.h>
#include <vclib/mesh.h>
#include <vclib/meshes.h>
#include <vclib/space.h>
#include <tiny_gltf.h>
#include <vector>

#include <iostream>

void trimeshVertexToTinygltfVertex(const vcl::trimesh::Vertex<double, false>& vertex, std::vector<double>& verticesRaw, std::vector<double>& normalsRaw);
//tinygltf::Primitive trimeshVertexToTinygltfVertex(const vcl::trimesh::Vertex<double, false>& vertex, std::vector<tinygltf::Accessor>& accessors, std::vector<tinygltf::BufferView>& bufferViews, std::vector<tinygltf::Buffer>& buffers);
std::vector<unsigned char> doubleVectorBigEndianToFloatBytesVectorLittleEndian(std::vector<double>& vec);

int main()
{
    //TODO tri mesh (material index solo dei vertici)
    vcl::LoadSettings loadSettings;
    vcl::MeshInfo info;
    loadSettings.loadTextureImages = true;
    auto bunnyMesh =
        vcl::loadMesh<vcl::TriMesh>(VCLIB_EXAMPLE_MESHES_PATH "/bunny.obj", info, loadSettings);

    tinygltf::Model model;
    tinygltf::Scene scene;
    tinygltf::Node node;
    tinygltf::Mesh mesh;
    std::vector<tinygltf::Accessor> accessors;
    std::vector<tinygltf::BufferView> bufferViews;
    std::vector<tinygltf::Buffer> buffers; //TODO buffers: vertici, normali, indici, texcoords, tangenti
    std::vector<double> verticesRaw{};
    std::vector<double> normalsRaw{};
    //std::vector<unsigned int> vertexIndicesRaw{};

    verticesRaw.reserve(3 * bunnyMesh.vertexCount());
    normalsRaw.reserve(3 * bunnyMesh.vertexCount());

    // 1) mesh
    // 1.1) vertices
    auto vIt = bunnyMesh.vertexBegin();
    auto vItEnd = bunnyMesh.vertexEnd();

    while (vIt != vItEnd) {
        tinygltf::Primitive primitive;
        primitive.attributes["POSITION"] = 0;
        primitive.attributes["NORMAL"] = 1;

        trimeshVertexToTinygltfVertex(*vIt, verticesRaw, normalsRaw);

        mesh.primitives.push_back(primitive);
    }

    //TODO 1.2) faces
    //TODO conversione facce

    model.meshes.push_back(mesh);

    // 2) buffer
    tinygltf::Buffer positionsBuffer{}, normalsBuffer{};
    positionsBuffer.data = doubleVectorBigEndianToFloatBytesVectorLittleEndian(verticesRaw);
    normalsBuffer.data = doubleVectorBigEndianToFloatBytesVectorLittleEndian(normalsRaw);
    //TODO la conversione di data in uri e' fatta da tinygltf?

    buffers.push_back(positionsBuffer);
    buffers.push_back(normalsBuffer);

    // 3) buffer views
    tinygltf::BufferView positionsBufferView{}, normalsBufferView{};

    positionsBufferView.buffer = 0;
    positionsBufferView.byteLength = positionsBuffer.data.size();

    normalsBufferView.buffer = 1;
    normalsBufferView.byteLength = normalsBuffer.data.size();

    bufferViews.push_back(positionsBufferView);
    bufferViews.push_back(normalsBufferView);

    //4) accessors
    tinygltf::Accessor positionsAccessor{}, normalsAccessor{};

    positionsAccessor.bufferView = 0;
    positionsAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT; // gltf FLOAT - 32bit
    positionsAccessor.type = TINYGLTF_TYPE_VEC3;
    positionsAccessor.count = positionsBufferView.byteLength / (4 * 3); // count = bytes / (float_bytes * vec3_elem_count)

    normalsAccessor.bufferView = 1;
    normalsAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT; // gltf FLOAT - 32bit
    normalsAccessor.type = TINYGLTF_TYPE_VEC3;
    normalsAccessor.count = normalsBufferView.byteLength / (4 * 3); // count = bytes / (float_bytes * vec3_elem_count

    accessors.push_back(positionsAccessor);
    accessors.push_back(normalsAccessor);

    // 5) node
    node.mesh = 0;
    model.nodes.push_back(node);

    // 6) default scene
    scene.nodes.push_back(0);
    model.scenes.push_back(scene);
    model.defaultScene = 0;



    //TODO restante:
    //TODO materiali
    //TODO nome
    //TODO bounding box





    /* funzioni tiny gltf per il salvataggio
    loader.WriteGltfSceneToFile(&model, output_filename);

       // Embedd buffers and images
#ifndef TINYGLTF_NO_STB_IMAGE_WRITE
    loader.WriteGltfSceneToFile(&model, embedded_filename, true, true);
#endif
    */

    return 0;
}

void trimeshVertexToTinygltfVertex(const vcl::trimesh::Vertex<double, false>& vertex, std::vector<double>& verticesRaw, std::vector<double>& normalsRaw) {
    verticesRaw.push_back(vertex.position().x());
    verticesRaw.push_back(vertex.position().y());
    verticesRaw.push_back(vertex.position().z());

    //TODO indicizzazione dei vertici?
    //...

    normalsRaw.push_back(vertex.normal().x());
    normalsRaw.push_back(vertex.normal().y());
    normalsRaw.push_back(vertex.normal().z());

    //TODO get optional tangent
    //TODO get optional material index
    //TODO get optional tex coords

    //TODO optional color
}

std::vector<unsigned char> doubleVectorBigEndianToFloatBytesVectorLittleEndian(std::vector<double>& vec) {
    std::vector<unsigned char> data;
    data.reserve(0); //TODO

    //TODO converti da double a float
    //TODO rappresentazione little endian

    return data;
}

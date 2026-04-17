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

#include <cstdint>

int main()
{
    std::cout << "Loading mesh..." << std::endl;

    //TODO tri mesh (material index solo dei vertici)
    vcl::LoadSettings loadSettings;
    vcl::MeshInfo info;
    loadSettings.loadTextureImages = true;
    auto bunnyMesh = vcl::loadMesh<vcl::TriMesh>(VCLIB_EXAMPLE_MESHES_PATH "/bunny.obj", info, loadSettings);
    vcl::updateBoundingBox(bunnyMesh);

    std::cout << "Converting mesh..." << std::endl;

    tinygltf::Model model;

    //TODO pull dalla main branch

    //TODO materials, tex coord, material

    model.asset.version = "2.0";
    model.asset.generator = "vclib-tinygltf-exporter";

    // mesh
    tinygltf::Mesh mesh;

    //TODO separazione delle primitive in base al materiale

    // triangles
    tinygltf::Primitive primitive;
    primitive.attributes["POSITION"] = 0;
    primitive.attributes["NORMAL"] = 1; //TODO only if info.hasPerVertexNormal()
    primitive.indices = 2;
    primitive.mode = 4; // gltf TRIANGLES
    //TODO Tex coords
    //TODO material

    mesh.primitives.push_back(primitive);
    model.meshes.push_back(mesh);

    //TODO materials
    //guarda loadGltfPrimitiveMaterial

    // buffer
    tinygltf::Buffer positionsBuffer{}, normalsBuffer{}, indicesBuffer{};

    // vertices
    std::vector<float> verticesRaw(3 * bunnyMesh.vertexCount());
    vcl::vertexPositionsToBuffer(bunnyMesh, verticesRaw.data());
    std::vector<unsigned char> verticesBufferData(
            reinterpret_cast<unsigned char*>(verticesRaw.data()),
            reinterpret_cast<unsigned char*>(verticesRaw.data()) + verticesRaw.size() * sizeof(float)
        );
    positionsBuffer.data = verticesBufferData;

    // normals
    std::vector<float> normalsRaw(3 * bunnyMesh.vertexCount());
    vcl::vertexNormalsToBuffer(bunnyMesh, normalsRaw.data());
    std::vector<unsigned char> normalsBufferData(
        reinterpret_cast<unsigned char*>(normalsRaw.data()),
        reinterpret_cast<unsigned char*>(normalsRaw.data()) + normalsRaw.size() * sizeof(float)
        );
    normalsBuffer.data = normalsBufferData;

    // indices
    std::vector<uint32_t> vertexIndicesRaw{};
    vertexIndicesRaw.reserve(3 * bunnyMesh.faceCount()); // They are triangle faces
    // indices of vertices that do not consider deleted vertices
    std::vector<uint32_t> vIndices = bunnyMesh.vertexCompactIndices();

    for (const vcl::TriMesh::Face& f : bunnyMesh.faces()) {
        for (const vcl::TriMesh::Vertex* v : f.vertices()) {
            vertexIndicesRaw.emplace_back(vIndices[bunnyMesh.index(v)]);
        }
    }

    std::vector<unsigned char> indicesBufferData(
        reinterpret_cast<unsigned char*>(vertexIndicesRaw.data()),
        reinterpret_cast<unsigned char*>(vertexIndicesRaw.data()) + vertexIndicesRaw.size() * sizeof(uint32_t)
        );
    indicesBuffer.data = indicesBufferData;

    model.buffers.push_back(positionsBuffer);
    model.buffers.push_back(normalsBuffer);
    model.buffers.push_back(indicesBuffer);

    // buffer views
    tinygltf::BufferView positionsBufferView{}, normalsBufferView{}, indicesBufferView{};

    positionsBufferView.buffer = 0;
    positionsBufferView.byteLength = positionsBuffer.data.size();

    normalsBufferView.buffer = 1;
    normalsBufferView.byteLength = normalsBuffer.data.size();

    indicesBufferView.buffer = 2;
    indicesBufferView.byteLength = indicesBuffer.data.size();
    indicesBufferView.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

    model.bufferViews.push_back(positionsBufferView);
    model.bufferViews.push_back(normalsBufferView);
    model.bufferViews.push_back(indicesBufferView);

    // accessors
    tinygltf::Accessor positionsAccessor{}, normalsAccessor{}, indicesAccessor{};

    positionsAccessor.bufferView = 0;
    positionsAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT; // gltf FLOAT - 32bit
    positionsAccessor.type = TINYGLTF_TYPE_VEC3;
    positionsAccessor.count = positionsBufferView.byteLength / (4 * 3); // count = bytes / (float_bytes * vec3_elem_count)
    auto bBox = bunnyMesh.boundingBox();
    positionsAccessor.maxValues = std::vector<double>{bBox.max().x(), bBox.max().y(), bBox.max().z()};
    positionsAccessor.minValues = std::vector<double>{bBox.min().x(), bBox.min().y(), bBox.min().z()};

    normalsAccessor.bufferView = 1;
    normalsAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT; // gltf FLOAT - 32bit
    normalsAccessor.type = TINYGLTF_TYPE_VEC3;
    normalsAccessor.count = normalsBufferView.byteLength / (4 * 3); // count = bytes / (float_bytes * vec3_elem_count)

    indicesAccessor.bufferView = 2;
    indicesAccessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT; // gltf UNSIGNED_INT - 32bit
    indicesAccessor.type = TINYGLTF_TYPE_SCALAR;
    indicesAccessor.count = indicesBufferView.byteLength / 4; // count = bytes / uint_bytes

    model.accessors.push_back(positionsAccessor);
    model.accessors.push_back(normalsAccessor);
    model.accessors.push_back(indicesAccessor);

    // node
    tinygltf::Node node;
    node.mesh = 0;
    node.matrix = std::vector<double>(bunnyMesh.transformMatrix().data(), bunnyMesh.transformMatrix().data() + bunnyMesh.transformMatrix().size());
    model.nodes.push_back(node);

    // default scene
    tinygltf::Scene scene;
    scene.nodes.push_back(0);
    model.scenes.push_back(scene);
    model.defaultScene = 0;



    //TODO restante:
    //TODO materiali
    //TODO nome



    //TODO faces to triangles: triangulatedFaceVertexIndicesToBuffer

    /*
#ifndef TINYGLTF_NO_STB_IMAGE_WRITE
    loader.WriteGltfSceneToFile(&model, embedded_filename, true, true);
#endif
    */

    std::cout << "Exporting to gltf..." << std::endl;

    tinygltf::TinyGLTF gltf;
    bool success = gltf.WriteGltfSceneToFile(&model, VCLIB_EXAMPLE_MESHES_PATH "/gltf/bunny_export_gltf.gltf",
          true,   // embedImages
          true,   // embedBuffers
          true,   // pretty print
          false); // write binary

    if (success)
        std::cout << "Export successful" << std::endl;
    else
        std::cout << "Export failed" << std::endl;

    return 0;
}
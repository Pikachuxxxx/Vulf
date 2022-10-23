#pragma once

#include "utils/mesh.h"

#include <iostream>

#include <tinyobj/tiny_obj_loader.h>

#include <string>

static void LoadObjModel(const std::string& path, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices) {

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    VK_LOG("Loading mode from : ", path, " .........");

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
        throw std::runtime_error(warn + err);
    }

    std::unordered_map<Vertex, uint16_t> uniqueVertices{};

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            // Tex coords
            //if (!attrib.texcoords.empty())
            //    vertex.texCoord = (glm::vec2(attrib.texcoords[2 * index.texcoord_index + 0], 1.0f - attrib.texcoords[2 * index.texcoord_index + 1]));
            //else
                vertex.texCoord = glm::vec2(0.0f, 0.0f);

            vertex.position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            // Normals
            //if (!attrib.normals.empty())
            //    vertex.normal = glm::vec3(attrib.normals[3 * index.normal_index + 0], attrib.normals[3 * index.normal_index + 1], attrib.normals[3 * index.normal_index + 2]);
            //else
                vertex.normal = glm::vec3(0.0f);

            // vertex colors
            glm::vec3 vert_color = glm::vec3(0.0f);
            //if (shape.mesh.material_ids[0] >= 0) {
            //    tinyobj::material_t* mp = &materials[shape.mesh.material_ids[0]];
            //    vert_color = glm::vec3(mp->diffuse[0], mp->diffuse[1], mp->diffuse[2]);
            //}
            vertex.color = vert_color;

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint16_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }

    VK_LOG("Model Loaded successfully!");
}

// TODO: Add indices to this
static Mesh LoadObjModel(const std::string& path)
{

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    VK_LOG("Loading mode from : ", path, " .........");

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
        throw std::runtime_error(warn + err);
    }

    // std::unordered_map<Vertex, uint16_t> uniqueVertices{};
    Mesh mesh;
    uint32_t offset = 0;

    for (const auto& shape : shapes)
    {
        uint32_t part_offset = offset;
        for (const auto& index : shape.mesh.indices)
        {
            Vertex vertex{};

            vertex.position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            ++offset;

            // Tex coords
            if (!attrib.texcoords.empty())
               vertex.texCoord = (glm::vec2(attrib.texcoords[2 * index.texcoord_index + 0], attrib.texcoords[2 * index.texcoord_index + 1]));
            else
                vertex.texCoord = glm::vec2(0.0f, 0.0f);

            // Normals
            if (!attrib.normals.empty())
               vertex.normal = glm::vec3(attrib.normals[3 * index.normal_index + 0], attrib.normals[3 * index.normal_index + 1], attrib.normals[3 * index.normal_index + 2]);
            else
                vertex.normal = glm::vec3(0.0f);

            // vertex colors
            glm::vec3 vert_color = glm::vec3(0.0f);
            if (shape.mesh.material_ids[0] >= 0) {
               tinyobj::material_t* mp = &materials[shape.mesh.material_ids[0]];
               vert_color = glm::vec3(mp->diffuse[0], mp->diffuse[1], mp->diffuse[2]);
            }
            vertex.color = vert_color;

            mesh.vertices.push_back(vertex);
        }
        uint32_t part_vertex_count = offset - part_offset;
        if( 0 < part_vertex_count )
            mesh.parts.push_back( { part_offset, part_vertex_count } );
    }
    VK_LOG("Model Loaded successfully!");
    return mesh;
}

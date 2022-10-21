#pragma once

#include "vertex.h"

struct Mesh
{
    std::vector<Vertex> vertices;
    // TODO: Modify parts and support indices
    // std::vector<uint16_t> indices;

    struct Part
    {
        uint32_t  VertexOffset;
        uint32_t  VertexCount;
    };

    std::vector<Part> parts;
};

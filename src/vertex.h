#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <vulkan/vulkan.h>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescription();
};

static const std::vector<Vertex> rainbowTriangleVertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{-0.5f,  0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    {{0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}}
};

static const std::vector<uint16_t> rainbowTriangleIndices = {
    0, 1, 2, 2, 3, 1
};

static const std::vector<Vertex> whiteQuadVertices = {
    {{-0.25f, -0.25f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{-0.25f,  0.25f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{ 0.25f, -0.25f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    {{ 0.25f,  0.25f, 0.0f}, {1.0f, 1.0f, 1.0f}}
};

static const std::vector<uint16_t> whiteQuadIndices = {
    0, 1, 2, 2, 3, 1
};

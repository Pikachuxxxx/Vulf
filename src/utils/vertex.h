#pragma once

// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <vector>
#include <array>
#include <vulkan/vulkan.h>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescription();

    bool operator==(const Vertex& other) const {
        return position == other.position && color == other.color && texCoord == other.texCoord;
    }
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

static const std::vector<Vertex> rainbowTriangleVertices = {
    {{-0.50f,  0.50f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 0.00f, -0.50f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{ 0.50f,  0.50f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}
};

static const std::vector<uint16_t> rainbowTriangleIndices = {
    0, 1, 2, 3, 0
};

static const std::vector<Vertex> whiteQuadVertices = {
    {{-0.25f, -0.25f, 0.5f}, {1.0f, 0.84f, 0.22f}, {0.0f, 0.0f}},
    {{0.25f,  -0.25f, 0.5f}, {1.0f, 0.84f, 0.22f}, {1.0f, 0.0f}},
    {{0.25f,   0.25f, 0.5f}, {1.0f, 0.84f, 0.22f}, {1.0f, 1.0f}},
    {{-0.25f,  0.25f, 0.5f}, {1.0f, 0.84f, 0.22f}, {0.0f, 1.0f}}
};

static const std::vector<uint16_t> whiteQuadIndices = {
    0, 1, 2, 2, 3, 0
};

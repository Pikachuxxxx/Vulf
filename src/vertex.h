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
	{{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 0.0f}},
	{{-0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}}
};
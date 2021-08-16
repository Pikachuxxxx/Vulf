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

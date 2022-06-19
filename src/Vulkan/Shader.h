#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

enum ShaderType
{
    VERTEX_SHADER = VK_SHADER_STAGE_VERTEX_BIT,
    // GEOMETRY_SHADER,
    // TESSELATION_SHADER,
    FRAGMENT_SHADER = VK_SHADER_STAGE_FRAGMENT_BIT,
    // COMPUTE_SHADER
    TASK_SHADER = VK_SHADER_STAGE_TASK_BIT_NV,
    MESH_SHADER = VK_SHADER_STAGE_MESH_BIT_NV
};

class Shader
{
public:
    Shader() = default;
    void CreateShader(const std::string& path, ShaderType type);
    void DestroyModule();
    const VkShaderModule& GetModule() { return m_Module; }
    const VkPipelineShaderStageCreateInfo& GetShaderStageInfo() { return m_ShaderStageInfo; }
    std::string GetShaderTypeString();
private:
    VkShaderModule m_Module;
    ShaderType m_ShaderType;
    VkPipelineShaderStageCreateInfo m_ShaderStageInfo;
private:
    std::vector<char> ReadShaderByteCode(const std::string& filePath);
};

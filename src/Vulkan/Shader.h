#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

enum class ShaderType
{
    VERTEX_SHADER,
    // GEOMETRY_SHADER,
    // TESSELATION_SHADER,
    FRAGMENT_SHADER,
    // COMPUTE_SHADER
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

#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

enum ShaderType
{
    VERTEX_SHADER = VK_SHADER_STAGE_VERTEX_BIT,
    FRAGMENT_SHADER = VK_SHADER_STAGE_FRAGMENT_BIT,
    //GEOMETRY_SHADER = VK_SHADER_STAGE_GEOMETRY_BIT,
    COMPUTE_SHADER = VK_SHADER_STAGE_COMPUTE_BIT,
    TASK_SHADER = VK_SHADER_STAGE_TASK_BIT_NV,
    MESH_SHADER = VK_SHADER_STAGE_MESH_BIT_NV,
    TESSELATION_CONTROL_SHADER = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
    TESSELATION_EVALUATION_SHADER = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT
};

class Shader
{
public:
    Shader() = default;
    void Init(const std::string& path, ShaderType type);
    void Destroy();


    const VkShaderModule& get_handle() { return m_Module; }
    const VkPipelineShaderStageCreateInfo& get_shader_stage_info() { return m_ShaderStageInfo; }
    std::string get_shader_type_string();

private:
    VkShaderModule                  m_Module;
    ShaderType                      m_ShaderType;
    VkPipelineShaderStageCreateInfo m_ShaderStageInfo;
private:
    std::vector<char> read_shader_byte_code(const std::string& filePath);
};

#include "VKShader.h"

#include <fstream>

#include "VKDevice.h"
#include "../utils/VulkanCheckResult.h"

void VKShader::CreateShader(const std::string& path, ShaderType type)
{
    m_ShaderType = type;
    VkShaderModuleCreateInfo shaderCI{};
    shaderCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    std::vector<char> byteCode = ReadShaderByteCode(path);
    shaderCI.codeSize = byteCode.size();
    shaderCI.pCode = reinterpret_cast<uint32_t*>(byteCode.data());

    if(VK_CALL(vkCreateShaderModule(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), &shaderCI, nullptr, &m_Module)))
        throw std::runtime_error("Cannot Create shader module!");
    else VK_LOG(GetShaderTypeString(), " shader module created!");

    // Create the pipeline shader shate create info
    m_ShaderStageInfo = {};
    m_ShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    switch (type) {
        case ShaderType::VERTEX_SHADER :
            m_ShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            break;
        case ShaderType::FRAGMENT_SHADER :
            m_ShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            break;
    }
    m_ShaderStageInfo.module = m_Module;
    m_ShaderStageInfo.pName = "main";
}

void VKShader::DestroyModule()
{
    vkDestroyShaderModule(VKLogicalDevice::GetDeviceManager()->GetLogicalDevice(), m_Module, nullptr);
}

std::string VKShader::GetShaderTypeString()
{
    switch (m_ShaderType) {
        case ShaderType::VERTEX_SHADER :
            return "Vertex";
            break;
        case ShaderType::FRAGMENT_SHADER :
            return "Fragment";
            break;
    }
}

std::vector<char> VKShader::ReadShaderByteCode(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);

    if(!file.is_open())
        throw std::runtime_error("Cannot open the shader byte code file");

    size_t fileSize = file.tellg();
    file.seekg(0);
    std::vector<char> buffer(fileSize);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

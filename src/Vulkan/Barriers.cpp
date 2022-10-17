#include "Barriers.h"

#include "Device.h"
#include "../utils/VulkanCheckResult.h"

std::vector<MemoryBarrierInfo> MemoryBarrier::s_Transitions;
std::vector<VkMemoryBarrier> MemoryBarrier::s_MemBarriers;

void MemoryBarrier::register_for_transtion(MemoryBarrierInfo transitionInfo)
{
    s_Transitions.push_back(transitionInfo);

    VkMemoryBarrier memBarrier{};
    memBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memBarrier.pNext = nullptr;
    memBarrier.srcAccessMask = transitionInfo.currentAccess;
    memBarrier.dstAccessMask = transitionInfo.newAccess;

    s_MemBarriers.push_back(memBarrier);
}

void MemoryBarrier::start_transition(VkPipelineStageFlags srcStages, VkPipelineStageFlags dstStages)
{
    VkCommandBuffer cmdBuf = Device::Get()->begin_single_time_cmd_buffer();
    {
        // Here last 2 in buffer last but 2 is for image barriers
        vkCmdPipelineBarrier(cmdBuf, srcStages, dstStages, 0, static_cast<uint32_t>(s_MemBarriers.size()), &s_MemBarriers[0], 0, nullptr, 0, nullptr);
    }
    Device::Get()->flush_cmd_buffer(cmdBuf, Device::Get()->get_graphics_queue());

    clear();
}

void MemoryBarrier::insert_barrier(MemoryBarrierInfo transitionInfo, VkPipelineStageFlags srcStages, VkPipelineStageFlags dstStages)
{
    register_for_transtion(transitionInfo);
    start_transition(srcStages, dstStages);
    clear();
}

void MemoryBarrier::clear()
{
    s_MemBarriers.clear();
    s_Transitions.clear();
}

//------------------------------------------------------------------------------

#pragma once

#include <vulkan/vulkan.h>

#include <vector>

struct MemoryBarrierInfo
{
    VkAccessFlags       currentAccess;
    VkAccessFlags       newAccess;

    MemoryBarrierInfo(VkAccessFlags currentAccess, VkAccessFlags newAccess) : currentAccess(currentAccess), newAccess(newAccess) { }
};

class MemoryBarrier
{
public:

    static std::vector<MemoryBarrierInfo> s_Transitions;
    static std::vector<VkMemoryBarrier> s_MemBarriers;

    static void register_for_transtion(MemoryBarrierInfo transitionInfo);
    static void start_transition(VkPipelineStageFlags srcStages, VkPipelineStageFlags dstStages);
    static void insert_barrier(MemoryBarrierInfo transitionInfo, VkPipelineStageFlags srcStages, VkPipelineStageFlags dstStages);

    static void clear();
};

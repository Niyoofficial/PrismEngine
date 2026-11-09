#include "VulkanRenderCommandQueue.h"

#include "VulkanRenderCommandList.h"
#include "VulkanRenderDevice.h"

namespace Prism::Render::Vulkan
{
VulkanRenderCommandQueue::VulkanRenderCommandQueue(VkQueue queue, const uint32_t queueFamilyIndex) :
	m_queue(queue), m_queueFamilyIndex(queueFamilyIndex)
{
	const auto& device = VulkanRenderDevice::Get();

	constexpr VkSemaphoreTypeCreateInfo timelineCreateInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
		.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
		.initialValue = 0,
	};

	const VkSemaphoreCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = &timelineCreateInfo,
		.flags = 0,
	};

	PE_ASSERT(vkCreateSemaphore(device.GetDevice(), &createInfo, nullptr, &m_timelineSemaphore) == VK_SUCCESS);
}

VulkanRenderCommandQueue::~VulkanRenderCommandQueue()
{
	const VkDevice logicalDevice = VulkanRenderDevice::Get().GetDevice();
	if (m_timelineSemaphore != VK_NULL_HANDLE)
	{
		vkDestroySemaphore(logicalDevice, m_timelineSemaphore, nullptr);
	}
}

void VulkanRenderCommandQueue::SetMarker(glm::float3 color, std::wstring string) {}

void VulkanRenderCommandQueue::BeginEvent(glm::float3 color, std::wstring string) {}

void VulkanRenderCommandQueue::EndEvent() {}

uint64_t VulkanRenderCommandQueue::GetFenceValue() { return m_fenceValue; }

void VulkanRenderCommandQueue::IncreaseFenceValue() { ++m_fenceValue; }

void VulkanRenderCommandQueue::SignalFence(uint64_t fenceValue)
{
	const VkTimelineSemaphoreSubmitInfo timelineSemaphoreSubmitInfo{
		.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
		.waitSemaphoreValueCount = 0,
		.signalSemaphoreValueCount = 1,
		.pSignalSemaphoreValues = &fenceValue,
	};

	const VkSubmitInfo submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = &timelineSemaphoreSubmitInfo,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &m_timelineSemaphore,
	};

	{
		std::scoped_lock lock(m_submitMutex);

		PE_ASSERT(vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE) == VK_SUCCESS);
	}
}

uint64_t VulkanRenderCommandQueue::IncreaseAndSignalFence()
{
	IncreaseFenceValue();
	SignalFence(m_fenceValue);

	return GetFenceValue();
}

uint64_t VulkanRenderCommandQueue::GetCompletedFenceValue() const
{
	uint64_t completedValue = 0;
	VkDevice logicalDevice = VulkanRenderDevice::Get().GetDevice();

	PE_ASSERT(vkGetSemaphoreCounterValue(logicalDevice, m_timelineSemaphore, &completedValue) == VK_SUCCESS);

	return completedValue;
}

void VulkanRenderCommandQueue::WaitForFenceToComplete(uint64_t fenceValue)
{
	if (GetCompletedFenceValue() < fenceValue)
	{
		const VkSemaphoreWaitInfo waitInfo{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.flags = 0,
			.semaphoreCount = 1,
			.pSemaphores = &m_timelineSemaphore,
			.pValues = &fenceValue,
		};
		VkDevice logicalDevice = VulkanRenderDevice::Get().GetDevice();

		PE_ASSERT(vkWaitSemaphores(logicalDevice, &waitInfo, UINT64_MAX) == VK_SUCCESS);
	}

	ExecuteGPUCompletionEvents();
}

void VulkanRenderCommandQueue::SubmitImmediate(const Ref<RenderCommandList>& renderCommandList)
{
	auto* cmd = dynamic_cast<VulkanRenderCommandList*>(renderCommandList.Raw());
	PE_ASSERT(cmd);

	cmd->Finalize();

	const auto& device = VulkanRenderDevice::Get();

	const VkCommandBuffer vkCmd = cmd->GetVkCommandBuffer();

	const VkCommandBufferSubmitInfo commandBufferInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.commandBuffer = vkCmd,
	};

	const VkSubmitInfo2 submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &commandBufferInfo,
	};

	constexpr VkFenceCreateInfo fenceInfo{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
	};

	VkFence fence = VK_NULL_HANDLE;

	PE_ASSERT(vkCreateFence(device.GetDevice(), &fenceInfo, nullptr, &fence) == VK_SUCCESS);

	{
		std::scoped_lock lock(m_submitMutex);

		PE_ASSERT(vkQueueSubmit2(m_queue, 1, &submitInfo, fence) == VK_SUCCESS);
	}

	PE_ASSERT(vkWaitForFences(device.GetDevice(), 1, &fence, VK_TRUE, UINT64_MAX) == VK_SUCCESS);

	vkDestroyFence(device.GetDevice(), fence, nullptr);
}

void VulkanRenderCommandQueue::Execute(RenderCommandList* cmdList)
{
	const auto* vkCmdList = dynamic_cast<VulkanRenderCommandList*>(cmdList);

	PE_ASSERT(vkCmdList);

	VkCommandBuffer vkCommandBuffer = vkCmdList->GetVkCommandBuffer();

	const VkSubmitInfo submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &vkCommandBuffer,
	};

	std::scoped_lock lock(m_submitMutex);

	PE_ASSERT(vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE) == VK_SUCCESS);
}
} // namespace Prism::Render::Vulkan

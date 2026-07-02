#include "VulkanRenderCommandQueue.h"

#include "VulkanRenderCommandList.h"
#include "VulkanRenderDevice.h"

Prism::Render::Vulkan::VulkanRenderCommandQueue::VulkanRenderCommandQueue(VkQueue queue, const uint32_t queueFamilyIndex) :
    m_queue(queue), m_queueFamilyIndex(queueFamilyIndex)
{
	const auto& device = VulkanRenderDevice::Get();

	constexpr VkSemaphoreTypeCreateInfo timelineCreateInfo{
	    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
	    .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
	};

	const VkSemaphoreCreateInfo createInfo{
	    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	    .pNext = &timelineCreateInfo,
	    .flags = 0,
	};

	PE_ASSERT(vkCreateSemaphore(device.GetDevice(), &createInfo, nullptr, &m_timelineSemaphore) == VK_SUCCESS);
}

Prism::Render::Vulkan::VulkanRenderCommandQueue::~VulkanRenderCommandQueue()
{
	const VkDevice logicalDevice = VulkanRenderDevice::Get().GetDevice();
	if (m_timelineSemaphore != VK_NULL_HANDLE)
	{
		vkDestroySemaphore(logicalDevice, m_timelineSemaphore, nullptr);
	}
}

void Prism::Render::Vulkan::VulkanRenderCommandQueue::SetMarker(glm::float3 color, std::wstring string) {}

void Prism::Render::Vulkan::VulkanRenderCommandQueue::BeginEvent(glm::float3 color, std::wstring string) {}

void Prism::Render::Vulkan::VulkanRenderCommandQueue::EndEvent() {}

uint64_t Prism::Render::Vulkan::VulkanRenderCommandQueue::GetFenceValue() { return m_fenceValue; }

void Prism::Render::Vulkan::VulkanRenderCommandQueue::IncreaseFenceValue() { ++m_fenceValue; }

void Prism::Render::Vulkan::VulkanRenderCommandQueue::SignalFence(uint64_t fenceValue)
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

	PE_ASSERT(vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE) == VK_SUCCESS);
}

uint64_t Prism::Render::Vulkan::VulkanRenderCommandQueue::IncreaseAndSignalFence()
{
	IncreaseFenceValue();
	SignalFence(m_fenceValue);

	return GetFenceValue();
}

uint64_t Prism::Render::Vulkan::VulkanRenderCommandQueue::GetCompletedFenceValue() const
{
	uint64_t completedValue = 0;
	VkDevice logicalDevice = VulkanRenderDevice::Get().GetDevice();

	PE_ASSERT(vkGetSemaphoreCounterValue(logicalDevice, m_timelineSemaphore, &completedValue) == VK_SUCCESS);

	return completedValue;
}

void Prism::Render::Vulkan::VulkanRenderCommandQueue::WaitForFenceToComplete(uint64_t fenceValue)
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

void Prism::Render::Vulkan::VulkanRenderCommandQueue::Execute(RenderCommandList* cmdList)
{
	const auto* vkCmdList = dynamic_cast<VulkanRenderCommandList*>(cmdList);

	VkCommandBuffer vkCommandBuffer = vkCmdList->GetVkCommandBuffer();

	PE_ASSERT(vkEndCommandBuffer(vkCommandBuffer) == VK_SUCCESS);

	IncreaseFenceValue();

	const uint64_t signalValue = m_fenceValue;

	const VkTimelineSemaphoreSubmitInfo timelineInfo{
	    .sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
	    .signalSemaphoreValueCount = 1,
	    .pSignalSemaphoreValues = &signalValue,
	};

	const VkSubmitInfo submitInfo{
	    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
	    .pNext = &timelineInfo,
	    .commandBufferCount = 1,
	    .pCommandBuffers = &vkCommandBuffer,
	    .signalSemaphoreCount = 1,
	    .pSignalSemaphores = &m_timelineSemaphore,
	};

	PE_ASSERT(vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE) == VK_SUCCESS);
}

#pragma once

#include <vulkan/vulkan_core.h>
#include "Prism/Render/RenderCommandQueue.h"

namespace Prism::Render::Vulkan
{
class VulkanRenderCommandQueue : public RenderCommandQueue
{
public:
	VulkanRenderCommandQueue(VkQueue queue, uint32_t queueFamilyIndex);
	~VulkanRenderCommandQueue() override;

	void SetMarker(glm::float3 color, std::wstring string) override;
	void BeginEvent(glm::float3 color, std::wstring string) override;
	void EndEvent() override;

	[[nodiscard]] uint64_t GetFenceValue() override;
	void IncreaseFenceValue() override;
	void SignalFence(uint64_t fenceValue) override;
	[[nodiscard]] uint64_t IncreaseAndSignalFence() override;
	[[nodiscard]] uint64_t GetCompletedFenceValue() const override;

	void WaitForFenceToComplete(uint64_t fenceValue) override;

	void SubmitImmediate(const Ref<RenderCommandList>& renderCommandList);

	[[nodiscard]] VkQueue GetQueue() const { return m_queue; }

	[[nodiscard]] std::mutex& GetSubmitMutex() { return m_submitMutex; }

private:
	void Execute(RenderCommandList* cmdList) override;

	VkQueue m_queue = VK_NULL_HANDLE;
	uint32_t m_queueFamilyIndex = 0;

	uint64_t m_fenceValue = 0;
	VkSemaphore m_timelineSemaphore = VK_NULL_HANDLE;

	VkSemaphore m_signalSemaphore = VK_NULL_HANDLE;

	std::mutex m_submitMutex;
};
} // namespace Prism::Render::Vulkan

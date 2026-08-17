#include "VulkanBufferView.h"
#include "VulkanBuffer.h"
#include "VulkanRenderDevice.h"

Prism::Render::Vulkan::VulkanBufferView::VulkanBufferView(const BufferViewDesc& desc, Buffer* buffer) : m_viewDesc(desc)
{
	PE_ASSERT(buffer, "Passed buffer is invalid");

	m_owningBuffer = buffer;

	const auto* vkBuffer = dynamic_cast<VulkanBuffer*>(m_owningBuffer.Raw());

	m_descriptorBufferInfo.buffer = vkBuffer->GetVkBuffer();
	m_descriptorBufferInfo.offset = m_viewDesc.offset;
	m_descriptorBufferInfo.range = m_viewDesc.size;

	switch (desc.type)
	{
	case BufferViewType::CBV:
		m_descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		break;
	case BufferViewType::SRV:
	case BufferViewType::UAV:
		m_descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		break;
	default:
		PE_ASSERT(false, "Unsupported buffer view type");
		break;
	}
}

Prism::Render::Vulkan::VulkanBufferView::~VulkanBufferView()
{
	if (m_bindlessIndex != UINT32_MAX)
	{
		VulkanRenderDevice::Get().GetBindlessManager().FreeResource(m_bindlessIndex);
	}
}

uint32_t Prism::Render::Vulkan::VulkanBufferView::GetBindlessIndex()
{
	if (m_bindlessIndex != UINT32_MAX)
	{
		return m_bindlessIndex;
	}

	auto& device = VulkanRenderDevice::Get();
	auto& bindless = device.GetBindlessManager();

	m_bindlessIndex = bindless.AllocateResource();
	bindless.WriteStorageBuffer(device.GetDevice(), m_bindlessIndex, m_descriptorBufferInfo.buffer, m_descriptorBufferInfo.offset,
	                            m_descriptorBufferInfo.range);

	return m_bindlessIndex;
}

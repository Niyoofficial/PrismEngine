#include "VulkanBufferView.h"
#include "VulkanBuffer.h"

Prism::Render::Vulkan::VulkanBufferView::VulkanBufferView(const BufferViewDesc& desc, Buffer* buffer) : m_viewDesc(desc)
{
	PE_ASSERT(buffer, "Passed buffer is invalid");

	m_owningBuffer = buffer;

	const auto* vkBuffer = static_cast<VulkanBuffer*>(m_owningBuffer.Raw());

	m_descriptorBufferInfo.buffer = vkBuffer->GetVkBuffer();
	m_descriptorBufferInfo.offset = m_viewDesc.offset;
	m_descriptorBufferInfo.range = m_viewDesc.size;

	switch (desc.type)
	{
	case BufferViewType::CBV:
		m_descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		break;

	case BufferViewType::SRV:
		m_descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		break;

	case BufferViewType::UAV:
		m_descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		break;

	default:
		PE_ASSERT(false, "Unsupported buffer view type");
		break;
	}
}

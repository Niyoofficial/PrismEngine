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
		VulkanRenderDevice::Get().GetBindlessManager()->FreeResource(m_bindlessIndex);
	}
}

uint32_t Prism::Render::Vulkan::VulkanBufferView::GetBindlessIndex() const
{
	PE_ASSERT(m_bindlessIndex != UINT32_MAX, "BufferView was not registered in bindless heap");

	return m_bindlessIndex;
}

void Prism::Render::Vulkan::VulkanBufferView::RegisterBindless()
{
	if (m_viewDesc.type != BufferViewType::CBV && m_viewDesc.type != BufferViewType::SRV &&
	    m_viewDesc.type != BufferViewType::UAV)
	{
		return;
	}

	const auto& device = VulkanRenderDevice::Get();
	auto* bindless = device.GetBindlessManager();

	m_bindlessIndex = bindless->AllocateResource();
	switch (m_descriptorType)
	{
	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		bindless->WriteUniformBuffer(device.GetDevice(), m_bindlessIndex, m_descriptorBufferInfo.buffer,
		                             m_descriptorBufferInfo.offset, m_descriptorBufferInfo.range);
		break;
	case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		bindless->WriteStorageBuffer(device.GetDevice(), m_bindlessIndex, m_descriptorBufferInfo.buffer,
		                             m_descriptorBufferInfo.offset, m_descriptorBufferInfo.range);
		break;
	default:
		PE_ASSERT_NO_ENTRY("Unsupported Vulkan texture view type");
		break;
	}
}

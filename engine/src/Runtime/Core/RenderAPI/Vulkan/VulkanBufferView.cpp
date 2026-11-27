#include "VulkanBufferView.h"
#include "VulkanBuffer.h"
#include "VulkanRenderDevice.h"

namespace Prism::Render::Vulkan
{
VulkanBufferView::VulkanBufferView(const BufferViewDesc& desc, Buffer* buffer) : m_viewDesc(desc)
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

VulkanBufferView::~VulkanBufferView()
{
	if (m_bindlessIndex != UINT32_MAX)
	{
		VulkanRenderDevice::Get().GetBindlessManager()->FreeResource(m_bindlessIndex);
	}
}

uint32_t VulkanBufferView::GetBindlessIndex() const
{
	PE_ASSERT(m_bindlessIndex != UINT32_MAX, "BufferView was not registered in bindless heap");

	return m_bindlessIndex;
}

void VulkanBufferView::RegisterBindless()
{
	if (m_viewDesc.type != BufferViewType::CBV && m_viewDesc.type != BufferViewType::SRV &&
		m_viewDesc.type != BufferViewType::UAV)
	{
		return;
	}

	// dynamic views must not own a permanent bindless descriptor, their descriptors are created for individual ring allocation
	if (IsViewOfDynamicResource())
	{
		return;
	}

	const auto& device = VulkanRenderDevice::Get();
	auto* bindless = device.GetBindlessManager();

	auto* vulkanBuffer = dynamic_cast<VulkanBuffer*>(m_owningBuffer.Raw());
	PE_ASSERT(vulkanBuffer, "Invalid Vulkan Buffer");

	const VkBuffer buffer = vulkanBuffer->GetVkBuffer();

	const VkBufferDeviceAddressInfo addressInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = buffer,
	};

	const VkDeviceAddress bufferAddress = vkGetBufferDeviceAddress(device.GetDevice(), &addressInfo);

	PE_ASSERT(bufferAddress != 0, "Failed to get buffer device address");

	const VkDeviceAddress descriptorAddress = bufferAddress + m_descriptorBufferInfo.offset;

	m_bindlessIndex = bindless->AllocateResource();
	switch (m_descriptorType)
	{
	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		bindless->WriteUniformBuffer(device.GetDevice(), m_bindlessIndex, descriptorAddress, m_descriptorBufferInfo.range);
		break;
	case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		bindless->WriteStorageBuffer(device.GetDevice(), m_bindlessIndex, descriptorAddress, m_descriptorBufferInfo.range);
		break;
	default:
		PE_ASSERT_NO_ENTRY("Unsupported Vulkan buffer view type");
		break;
	}
}

uint32_t VulkanBufferView::BuildDynamicDescriptor(const DynamicBufferAllocator::Allocation& allocation) const
{
	PE_ASSERT(IsViewOfDynamicResource(), "BufferView is not dynamic");
	PE_ASSERT(allocation.IsValid(), "Invalid dynamic allocation");

	const auto& device = VulkanRenderDevice::Get();
	auto* bindless = device.GetBindlessManager();

	const VkDeviceAddress ringBufferAddress = device.GetDynamicBufferDeviceAddress(allocation.ringBufferID);

	PE_ASSERT(ringBufferAddress != 0, "Failed to get dynamic buffer device address");

	const VkDeviceAddress descriptorAddress = ringBufferAddress + allocation.gpuRingAllocation.offset + m_viewDesc.offset;

	const uint32_t bindlessIndex = bindless->AllocateResource();

	PE_ASSERT(bindlessIndex != UINT32_MAX, "Failed to allocate bindless descriptor");

	switch (m_descriptorType)
	{
	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		bindless->WriteUniformBuffer(device.GetDevice(), bindlessIndex, descriptorAddress, m_viewDesc.size);
		break;

	case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		bindless->WriteStorageBuffer(device.GetDevice(), bindlessIndex, descriptorAddress, m_viewDesc.size);
		break;

	default:
		PE_ASSERT_NO_ENTRY("Unsupported Vulkan buffer descriptor type");
		return UINT32_MAX;
	}

	return bindlessIndex;
}
} // namespace Prism::Render::Vulkan

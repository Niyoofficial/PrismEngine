#include "VulkanBindlessManager.h"

#include <array>
#include <limits>
#include "Prism/Base/Assert.h"
#include "VulkanRenderDevice.h"
#include "VulkanUtils.h"

// TODO
// use volk library
namespace
{
PFN_vkWriteResourceDescriptorsEXT vkWriteResourceDescriptorsEXTFunction = nullptr;
PFN_vkWriteSamplerDescriptorsEXT vkWriteSamplerDescriptorsEXTFunction = nullptr;
PFN_vkCmdBindResourceHeapEXT vkCmdBindResourceHeapEXTFunction = nullptr;
PFN_vkCmdBindSamplerHeapEXT vkCmdBindSamplerHeapEXTFunction = nullptr;
} // namespace

namespace Prism::Render::Vulkan
{
VulkanBindlessManager::VulkanBindlessManager()
{
	auto& renderDevice = VulkanRenderDevice::Get();

	const VkDevice device = renderDevice.GetDevice();
	const VkPhysicalDevice physicalDevice = renderDevice.GetPhysicalDevice();

	vkWriteResourceDescriptorsEXTFunction =
		reinterpret_cast<PFN_vkWriteResourceDescriptorsEXT>(vkGetDeviceProcAddr(device, "vkWriteResourceDescriptorsEXT"));
	PE_ASSERT(vkWriteResourceDescriptorsEXTFunction != nullptr);

	vkWriteSamplerDescriptorsEXTFunction =
		reinterpret_cast<PFN_vkWriteSamplerDescriptorsEXT>(vkGetDeviceProcAddr(device, "vkWriteSamplerDescriptorsEXT"));
	PE_ASSERT(vkWriteSamplerDescriptorsEXTFunction != nullptr);

	vkCmdBindResourceHeapEXTFunction =
		reinterpret_cast<PFN_vkCmdBindResourceHeapEXT>(vkGetDeviceProcAddr(device, "vkCmdBindResourceHeapEXT"));
	PE_ASSERT(vkCmdBindResourceHeapEXTFunction != nullptr);

	vkCmdBindSamplerHeapEXTFunction =
		reinterpret_cast<PFN_vkCmdBindSamplerHeapEXT>(vkGetDeviceProcAddr(device, "vkCmdBindSamplerHeapEXT"));
	PE_ASSERT(vkCmdBindSamplerHeapEXTFunction != nullptr);

	VkPhysicalDeviceDescriptorHeapPropertiesEXT heapProperties{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_HEAP_PROPERTIES_EXT,
	};

	VkPhysicalDeviceProperties2 properties{
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
		.pNext = &heapProperties,
	};

	vkGetPhysicalDeviceProperties2(physicalDevice, &properties);

	PE_ASSERT(heapProperties.maxResourceHeapSize > 0);
	PE_ASSERT(heapProperties.maxSamplerHeapSize > 0);

	PE_ASSERT(heapProperties.resourceHeapAlignment > 0);
	PE_ASSERT(heapProperties.samplerHeapAlignment > 0);

	PE_ASSERT(heapProperties.imageDescriptorSize > 0);
	PE_ASSERT(heapProperties.bufferDescriptorSize > 0);
	PE_ASSERT(heapProperties.samplerDescriptorSize > 0);

	PE_ASSERT(heapProperties.imageDescriptorAlignment > 0);
	PE_ASSERT(heapProperties.bufferDescriptorAlignment > 0);
	PE_ASSERT(heapProperties.samplerDescriptorAlignment > 0);

	PE_ASSERT(heapProperties.minResourceHeapReservedRange > 0);
	PE_ASSERT(heapProperties.minSamplerHeapReservedRange > 0);

	m_resourceHeapAlignment = heapProperties.resourceHeapAlignment;
	m_samplerHeapAlignment = heapProperties.samplerHeapAlignment;

	// one resource slot must be suitable for both image and buffer descriptors
	const VkDeviceSize resourceDescriptorAlignment =
		GetCombinedAlignment(heapProperties.imageDescriptorAlignment, heapProperties.bufferDescriptorAlignment);

	m_resourceDescriptorSize =
		AlignUp(std::max(heapProperties.imageDescriptorSize, heapProperties.bufferDescriptorSize), resourceDescriptorAlignment);

	m_samplerDescriptorSize = AlignUp(heapProperties.samplerDescriptorSize, heapProperties.samplerDescriptorAlignment);

	PE_ASSERT(MaxBindlessDescriptors <= std::numeric_limits<VkDeviceSize>::max() / m_resourceDescriptorSize);

	PE_ASSERT(SamplerCount <= std::numeric_limits<VkDeviceSize>::max() / m_samplerDescriptorSize);

	const VkDeviceSize resourceApplicationCapacity = static_cast<VkDeviceSize>(MaxBindlessDescriptors) * m_resourceDescriptorSize;

	const VkDeviceSize samplerApplicationCapacity = static_cast<VkDeviceSize>(SamplerCount) * m_samplerDescriptorSize;

	const VkDeviceSize resourceTotalSize =
		AlignUp(resourceApplicationCapacity, m_resourceHeapAlignment) + heapProperties.minResourceHeapReservedRange;

	const VkDeviceSize samplerTotalSize =
		AlignUp(samplerApplicationCapacity, m_samplerHeapAlignment) + heapProperties.minSamplerHeapReservedRange;

	PE_ASSERT(resourceTotalSize <= heapProperties.maxResourceHeapSize, "Resource descriptor heap exceeds maxResourceHeapSize");

	PE_ASSERT(samplerTotalSize <= heapProperties.maxSamplerHeapSize, "Sampler descriptor heap exceeds maxSamplerHeapSize");

	const bool resourceHeapCreated =
		m_resourceHeap.Create(resourceApplicationCapacity, m_resourceHeapAlignment, m_resourceHeapAlignment,
							  heapProperties.minResourceHeapReservedRange, L"ResourceDescriptorHeapBuffer");

	PE_ASSERT(resourceHeapCreated, "Failed to create resource descriptor heap");

	const bool samplerHeapCreated =
		m_samplerHeap.Create(samplerApplicationCapacity, m_samplerHeapAlignment, m_samplerHeapAlignment,
							 heapProperties.minSamplerHeapReservedRange, L"SamplerDescriptorHeapBuffer");

	PE_ASSERT(samplerHeapCreated, "Failed to create sampler descriptor heap");

	// initialize the resource heap memory
	std::memset(m_resourceHeap.GetMapped(), 0, static_cast<size_t>(resourceApplicationCapacity));

	// initialize the sampler heap memory
	std::memset(m_samplerHeap.GetMapped(), 0, static_cast<size_t>(samplerApplicationCapacity));

	// write the default sampler descriptors
	const float maxAnisotropy = std::min(8.0f, properties.properties.limits.maxSamplerAnisotropy);
	CreateSamplers(device, maxAnisotropy);
}

VulkanBindlessManager::~VulkanBindlessManager()
{
	m_samplerHeap.Destroy();
	m_resourceHeap.Destroy();
}

uint32_t VulkanBindlessManager::AllocateResource()
{
	auto& list = m_resourceFreeList;

	std::lock_guard lock(list.mutex);

	if (!list.freeIndices.empty())
	{
		const uint32_t index = list.freeIndices.back();
		list.freeIndices.pop_back();
		return index;
	}

	PE_ASSERT(list.nextIndex < MaxBindlessDescriptors, "Bindless resource heap exhausted");

	return list.nextIndex++;
}

void VulkanBindlessManager::FreeResource(uint32_t index)
{
	PE_ASSERT(index < MaxBindlessDescriptors);

	auto& list = m_resourceFreeList;

	std::lock_guard lock(list.mutex);

	// avoid accidentally inserting the same index more than once
	PE_ASSERT(index < list.nextIndex);

	PE_ASSERT(std::find(list.freeIndices.begin(), list.freeIndices.end(), index) == list.freeIndices.end(),
			  "Bindless resource index was already freed");

	list.freeIndices.push_back(index);
}

void VulkanBindlessManager::WriteSampledImage(VkDevice device, uint32_t index, const VkImageViewCreateInfo& viewInfo,
											  VkImageLayout layout)
{
	PE_ASSERT(index < MaxBindlessDescriptors);

	VkImageDescriptorInfoEXT imageInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_DESCRIPTOR_INFO_EXT,
		.pView = &viewInfo,
		.layout = layout,
	};

	const VkResourceDescriptorInfoEXT resourceInfo{
		.sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT,
		.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		.data =
			{
				.pImage = &imageInfo,
			},
	};

	const VkHostAddressRangeEXT descriptorRange = MakeHostRange(
		m_resourceHeap.GetMapped(), static_cast<VkDeviceSize>(index) * m_resourceDescriptorSize, m_resourceDescriptorSize);

	const VkResult result = vkWriteResourceDescriptorsEXTFunction(device, 1, &resourceInfo, &descriptorRange);

	PE_ASSERT(result == VK_SUCCESS, "Failed to write sampled image descriptor");
}

void VulkanBindlessManager::WriteStorageImage(VkDevice device, uint32_t index, const VkImageViewCreateInfo& viewInfo,
											  VkImageLayout layout)
{
	PE_ASSERT(index < MaxBindlessDescriptors);

	VkImageDescriptorInfoEXT imageInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_DESCRIPTOR_INFO_EXT,
		.pView = &viewInfo,
		.layout = layout,
	};

	const VkResourceDescriptorInfoEXT resourceInfo{
		.sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT,
		.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		.data =
			{
				.pImage = &imageInfo,
			},
	};

	const VkHostAddressRangeEXT descriptorRange = MakeHostRange(
		m_resourceHeap.GetMapped(), static_cast<VkDeviceSize>(index) * m_resourceDescriptorSize, m_resourceDescriptorSize);

	const VkResult result = vkWriteResourceDescriptorsEXTFunction(device, 1, &resourceInfo, &descriptorRange);

	PE_ASSERT(result == VK_SUCCESS, "Failed to write storage image descriptor");
}

void VulkanBindlessManager::WriteUniformBuffer(VkDevice device, uint32_t index, VkDeviceAddress address, VkDeviceSize range)
{
	PE_ASSERT(index < MaxBindlessDescriptors);
	PE_ASSERT(address != 0);
	PE_ASSERT(range > 0);

	const VkDeviceAddressRangeEXT addressRange{
		.address = address,
		.size = range,
	};

	const VkResourceDescriptorInfoEXT resourceInfo{
		.sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT,
		.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.data =
			{
				.pAddressRange = &addressRange,
			},
	};

	const VkHostAddressRangeEXT descriptorRange = MakeHostRange(
		m_resourceHeap.GetMapped(), static_cast<VkDeviceSize>(index) * m_resourceDescriptorSize, m_resourceDescriptorSize);

	const VkResult result = vkWriteResourceDescriptorsEXTFunction(device, 1, &resourceInfo, &descriptorRange);

	PE_ASSERT(result == VK_SUCCESS, "Failed to write uniform buffer descriptor");
}

void VulkanBindlessManager::WriteStorageBuffer(VkDevice device, uint32_t index, VkDeviceAddress address, VkDeviceSize range)
{
	PE_ASSERT(index < MaxBindlessDescriptors);
	PE_ASSERT(address != 0);
	PE_ASSERT(range > 0);

	const VkDeviceAddressRangeEXT addressRange{
		.address = address,
		.size = range,
	};

	const VkResourceDescriptorInfoEXT resourceInfo{
		.sType = VK_STRUCTURE_TYPE_RESOURCE_DESCRIPTOR_INFO_EXT,
		.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		.data =
			{
				.pAddressRange = &addressRange,
			},
	};

	const VkHostAddressRangeEXT descriptorRange = MakeHostRange(
		m_resourceHeap.GetMapped(), static_cast<VkDeviceSize>(index) * m_resourceDescriptorSize, m_resourceDescriptorSize);

	const VkResult result = vkWriteResourceDescriptorsEXTFunction(device, 1, &resourceInfo, &descriptorRange);

	PE_ASSERT(result == VK_SUCCESS, "Failed to write storage buffer descriptor");
}

void VulkanBindlessManager::WriteSampler(const VkDevice device, const uint32_t index, const VkSamplerCreateInfo& samplerInfo)
{
	PE_ASSERT(index < SamplerCount);

	const VkHostAddressRangeEXT descriptorRange = MakeHostRange(
		m_samplerHeap.GetMapped(), static_cast<VkDeviceSize>(index) * m_samplerDescriptorSize, m_samplerDescriptorSize);

	const VkResult result = vkWriteSamplerDescriptorsEXTFunction(device, 1, &samplerInfo, &descriptorRange);

	PE_ASSERT(result == VK_SUCCESS, "Failed to write sampler descriptor");
}

void VulkanBindlessManager::BindHeaps(VkCommandBuffer commandBuffer) const
{
	PE_ASSERT(commandBuffer != VK_NULL_HANDLE);

	const VkBindHeapInfoEXT resourceHeapInfo = m_resourceHeap.GetBindHeapInfo();
	const VkBindHeapInfoEXT samplerHeapInfo = m_samplerHeap.GetBindHeapInfo();

	vkCmdBindResourceHeapEXTFunction(commandBuffer, &resourceHeapInfo);
	vkCmdBindSamplerHeapEXTFunction(commandBuffer, &samplerHeapInfo);
}

VkDeviceAddress VulkanBindlessManager::GetResourceDescriptorAddress(const uint32_t index) const
{
	PE_ASSERT(index < MaxBindlessDescriptors);

	return m_resourceHeap.GetAddress() + static_cast<VkDeviceAddress>(index) * m_resourceDescriptorSize;
}

void VulkanBindlessManager::CreateSamplers(const VkDevice device, const float maxAnisotropy)
{
	std::array<VkSamplerCreateInfo, SamplerCount> samplers{};

	const auto makeSampler = [](VkFilter filter, VkSamplerMipmapMode mipmapMode, VkSamplerAddressMode addressMode,
								const bool anisotropy, const float maxAnisotropy, const bool compare)
	{
		return VkSamplerCreateInfo{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter = filter,
			.minFilter = filter,
			.mipmapMode = mipmapMode,
			.addressModeU = addressMode,
			.addressModeV = addressMode,
			.addressModeW = addressMode,
			.mipLodBias = 0.0f,
			.anisotropyEnable = anisotropy ? VK_TRUE : VK_FALSE,
			.maxAnisotropy = anisotropy ? maxAnisotropy : 1.0f,
			.compareEnable = compare ? VK_TRUE : VK_FALSE,
			.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
			.minLod = 0.0f,
			.maxLod = VK_LOD_CLAMP_NONE,
			.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
			.unnormalizedCoordinates = VK_FALSE,
		};
	};

	// 0 - s_pointWrap
	samplers[0] =
		makeSampler(VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT, false, 1.0f, false);

	// 1 - s_pointClamp
	samplers[1] =
		makeSampler(VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, false, 1.0f, false);

	// 2 - s_linearWrap
	samplers[2] =
		makeSampler(VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, false, 1.0f, false);

	// 3 - s_linearClamp
	samplers[3] =
		makeSampler(VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, false, 1.0f, false);

	// 4 - s_anisotropicWrap
	samplers[4] =
		makeSampler(VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, true, maxAnisotropy, false);

	// 5 - s_anisotropicClamp
	samplers[5] = makeSampler(VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, true,
							  maxAnisotropy, false);

	// 6 - s_shadow
	samplers[6] =
		makeSampler(VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, false, 1.0f, true);

	for (uint32_t i = 0; i < SamplerCount; ++i)
	{
		WriteSampler(device, i, samplers[i]);
	}
}
} // namespace Prism::Render::Vulkan

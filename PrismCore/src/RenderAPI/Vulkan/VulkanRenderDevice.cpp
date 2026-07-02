#include "VulkanRenderDevice.h"

#include <SDL3/SDL_vulkan.h>
#include <iostream>
#include "VulkanBuffer.h"
#include "VulkanBufferView.h"
#include "VulkanRenderCommandQueue.h"

constexpr std::string_view applicationName = "PrismEngine";
constexpr uint32_t applicationVersion = VK_MAKE_VERSION(1, 0, 0);
constexpr uint32_t applicationVulkanApiVersion = VK_API_VERSION_1_3;
constexpr std::array<const char*, 1> applicationValidationLayers{
    "VK_LAYER_KHRONOS_validation",
};
constexpr std::array<const char*, 1> applicationDeviceExtensions{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		std::cerr << "[Vulkan Validation Layer]: " << pCallbackData->pMessage << std::endl << std::endl;
	}
	return VK_FALSE;
}

Prism::Render::Vulkan::VulkanRenderDevice& Prism::Render::Vulkan::VulkanRenderDevice::Get()
{
	return dynamic_cast<VulkanRenderDevice&>(RenderDevice::Get());
}

Prism::Render::Vulkan::VulkanRenderDevice* Prism::Render::Vulkan::VulkanRenderDevice::TryGet()
{
	return dynamic_cast<VulkanRenderDevice*>(RenderDevice::TryGet());
}

Prism::Render::Vulkan::VulkanRenderDevice::VulkanRenderDevice(const RenderDeviceParams& params) : RenderDevice(params)
{
	CreateVulkanInstance(params);

	if (params.enableDebugLayer)
	{
		CreateDebugMessenger();
	}

	PickPhysicalDevice();

	CreateLogicalDevice();

	CreateAllocator();

	CreateRenderCommandQueue();

	InitDeviceSubsystems();
}

Prism::Render::Vulkan::VulkanRenderDevice::~VulkanRenderDevice()
{
	m_commandQueue.reset();

	if (m_allocator)
	{
		vmaDestroyAllocator(m_allocator);
		m_allocator = VK_NULL_HANDLE;
	}

	if (m_device)
	{
		vkDestroyDevice(m_device, nullptr);
		m_device = VK_NULL_HANDLE;
	}

	if (m_debugMessenger)
	{
		const auto vkDestroyDebugUtilsMessengerEXTFunction = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
		    vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT"));

		if (vkDestroyDebugUtilsMessengerEXTFunction)
		{
			vkDestroyDebugUtilsMessengerEXTFunction(m_instance, m_debugMessenger, nullptr);
		}

		m_debugMessenger = VK_NULL_HANDLE;
	}

	if (m_instance)
	{
		vkDestroyInstance(m_instance, nullptr);
		m_instance = VK_NULL_HANDLE;
	}
}

Prism::Ref<Prism::Render::Buffer> Prism::Render::Vulkan::VulkanRenderDevice::CreateBuffer(const BufferDesc& desc)
{
	return Ref<VulkanBuffer>::Create(this, desc);
}

Prism::Render::RenderCommandQueue* Prism::Render::Vulkan::VulkanRenderDevice::GetRenderCommandQueue() const
{
	return m_commandQueue.get();
}

Prism::Ref<Prism::Render::BufferView> Prism::Render::Vulkan::VulkanRenderDevice::CreateBufferView_Impl(const BufferViewDesc& desc,
                                                                                                       Buffer* buffer)
{
	return Ref<VulkanBufferView>::Create(desc, buffer);
}

Prism::Ref<Prism::Render::TextureView>
Prism::Render::Vulkan::VulkanRenderDevice::CreateTextureView_Impl(const TextureViewDesc& desc, Texture* texture)
{}

void Prism::Render::Vulkan::VulkanRenderDevice::CreateVulkanInstance(const RenderDeviceParams& params)
{
	VkApplicationInfo applicationInfo{
	    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
	    .pApplicationName = applicationName.data(),
	    .applicationVersion = applicationVersion,
	    .pEngineName = applicationName.data(),
	    .engineVersion = applicationVersion,
	    .apiVersion = applicationVulkanApiVersion,
	};

	uint32_t extensionCount = 0;

	const char* const* sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);

	std::vector extensions(sdlExtensions, sdlExtensions + extensionCount);

	if (params.enableDebugLayer)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		PE_ASSERT(AreValidationLayerSupported());
	}

	const VkInstanceCreateInfo createInfo{
	    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
	    .pApplicationInfo = &applicationInfo,
	    .enabledLayerCount = params.enableDebugLayer ? static_cast<uint32_t>(applicationValidationLayers.size()) : 0u,
	    .ppEnabledLayerNames = params.enableDebugLayer ? applicationValidationLayers.data() : nullptr,
	    .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
	    .ppEnabledExtensionNames = extensions.data(),
	};

	PE_ASSERT(vkCreateInstance(&createInfo, nullptr, &m_instance) == VK_SUCCESS);
}

void Prism::Render::Vulkan::VulkanRenderDevice::CreateDebugMessenger()
{
	constexpr VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{
	    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
	    .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
	    .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
	        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
	    .pfnUserCallback = DebugCallback,
	    .pUserData = nullptr,
	};

	const auto vkCreateDebugUtilsMessengerEXTFunction =
	    reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT"));

	PE_ASSERT(vkCreateDebugUtilsMessengerEXTFunction);

	PE_ASSERT(vkCreateDebugUtilsMessengerEXTFunction(m_instance, &debugCreateInfo, nullptr, &m_debugMessenger) == VK_SUCCESS);
}

bool Prism::Render::Vulkan::VulkanRenderDevice::AreValidationLayerSupported() const
{
	uint32_t count = 0;
	vkEnumerateInstanceLayerProperties(&count, nullptr);

	std::vector<VkLayerProperties> layers(count);
	vkEnumerateInstanceLayerProperties(&count, layers.data());

	for (const auto* wantedValidationLayer : applicationValidationLayers)
	{
		bool found = false;

		for (const auto& layer : layers)
		{
			if (strcmp(layer.layerName, wantedValidationLayer) == 0)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			return false;
		}
	}

	return true;
}

bool Prism::Render::Vulkan::VulkanRenderDevice::AreDeviceExtensionSupported(const VkPhysicalDevice physicalDevice) const
{
	uint32_t count = 0;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);

	std::vector<VkExtensionProperties> extensions(count);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, extensions.data());

	for (const auto* wanted : applicationDeviceExtensions)
	{
		bool found = false;

		for (const auto& extension : extensions)
		{
			if (strcmp(extension.extensionName, wanted) == 0)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			return false;
		}
	}

	return true;
}

bool Prism::Render::Vulkan::VulkanRenderDevice::IsPhysicalDeviceSuitable(const VkPhysicalDevice physicalDevice) const
{
	if (!AreDeviceExtensionSupported(physicalDevice))
	{
		return false;
	}

	if (FindGraphicsQueueFamilyIndex(physicalDevice) == UINT32_MAX)
	{
		return false;
	}

	VkPhysicalDeviceVulkan13Features features13{
	    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
	};

	VkPhysicalDeviceFeatures2 features{
	    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
	    .pNext = &features13,
	};

	vkGetPhysicalDeviceFeatures2(physicalDevice, &features);

	return features13.dynamicRendering && features13.synchronization2;
}

void Prism::Render::Vulkan::VulkanRenderDevice::PickPhysicalDevice()
{
	uint32_t count = 0;
	vkEnumeratePhysicalDevices(m_instance, &count, nullptr);

	PE_ASSERT(count > 0);

	std::vector<VkPhysicalDevice> devices(count);
	vkEnumeratePhysicalDevices(m_instance, &count, devices.data());

	for (const auto device : devices)
	{
		if (IsPhysicalDeviceSuitable(device))
		{
			m_physicalDevice = device;
			m_graphicsQueueFamilyIndex = FindGraphicsQueueFamilyIndex(device);
			return;
		}
	}

	PE_ASSERT(false, "No suitable Vulkan device found");
}

void Prism::Render::Vulkan::VulkanRenderDevice::CreateLogicalDevice()
{
	constexpr float queuePriority = 1.f;

	VkDeviceQueueCreateInfo queueCreateInfo{
	    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
	    .queueFamilyIndex = m_graphicsQueueFamilyIndex,
	    .queueCount = 1,
	    .pQueuePriorities = &queuePriority,
	};

	VkPhysicalDeviceVulkan13Features features13{
	    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
	    .synchronization2 = VK_TRUE,
	    .dynamicRendering = VK_TRUE,
	};

	VkPhysicalDeviceFeatures2 features2{
	    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
	    .pNext = &features13,
	};

	VkDeviceCreateInfo createInfo{
	    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
	    .pNext = &features2,
	    .queueCreateInfoCount = 1,
	    .pQueueCreateInfos = &queueCreateInfo,
	    .enabledExtensionCount = static_cast<uint32_t>(applicationDeviceExtensions.size()),
	    .ppEnabledExtensionNames = applicationDeviceExtensions.data(),
	};

	PE_ASSERT(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) == VK_SUCCESS);

	VkQueue graphicsQueue{};
	vkGetDeviceQueue(m_device, m_graphicsQueueFamilyIndex, 0, &graphicsQueue);

	CreateRenderCommandQueue(graphicsQueue, m_graphicsQueueFamilyIndex);
}

void Prism::Render::Vulkan::VulkanRenderDevice::CreateAllocator()
{
	const VmaAllocatorCreateInfo allocatorInfo{
	    .physicalDevice = m_physicalDevice,
	    .device = m_device,
	    .instance = m_instance,
	    .vulkanApiVersion = applicationVulkanApiVersion,
	};

	PE_ASSERT(vmaCreateAllocator(&allocatorInfo, &m_allocator) == VK_SUCCESS);
}

void Prism::Render::Vulkan::VulkanRenderDevice::CreateRenderCommandQueue(VkQueue queue, uint32_t queueFamilyIndex)
{
	m_commandQueue = std::make_unique<VulkanRenderCommandQueue>(queue, queueFamilyIndex);
}

uint32_t Prism::Render::Vulkan::VulkanRenderDevice::FindGraphicsQueueFamilyIndex(const VkPhysicalDevice physicalDevice) const
{
	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilyProperties(count);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, queueFamilyProperties.data());

	for (uint32_t i = 0; i < count; i++)
	{
		if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && queueFamilyProperties[i].queueCount > 0)
		{
			return i;
		}
	}

	PE_ASSERT(false, "Failed to find a graphics queue family index for the Vulkan device");

	return UINT32_MAX;
}

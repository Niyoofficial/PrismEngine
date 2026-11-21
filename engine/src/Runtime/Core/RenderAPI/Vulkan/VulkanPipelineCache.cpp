#include "VulkanPipelineCache.h"

#include "VulkanPipelineLayoutCache.h"
#include "VulkanRenderDevice.h"
#include "VulkanShaderCompiler.h"
#include "VulkanTypeConversions.h"

namespace
{
struct StageDescriptorMappingData
{
	std::vector<VkDescriptorSetAndBindingMappingEXT> mappings;

	VkShaderDescriptorSetAndBindingMappingInfoEXT info{
		.sType = VK_STRUCTURE_TYPE_SHADER_DESCRIPTOR_SET_AND_BINDING_MAPPING_INFO_EXT,
		.pNext = nullptr,
		.mappingCount = 0,
		.pMappings = nullptr,
	};

	const void* GetPNext() const { return mappings.empty() ? nullptr : &info; }
};

StageDescriptorMappingData BuildStageDescriptorMappings(const Prism::Render::Vulkan::VulkanShaderReflection& reflection,
														const Prism::Render::Vulkan::VulkanBindlessManager& bindlessManager)
{
	StageDescriptorMappingData result{};

	const uint32_t samplerStride = static_cast<uint32_t>(bindlessManager.GetSamplerDescriptorSize());
	const uint32_t resourceStride = static_cast<uint32_t>(bindlessManager.GetResourceDescriptorSize());

	const uint32_t setCount = reflection.GetDescriptorSetCount();
	for (uint32_t s = 0; s < setCount; ++s)
	{
		const SpvReflectDescriptorSet& set = reflection.GetDescriptorSet(s);
		for (uint32_t b = 0; b < set.binding_count; ++b)
		{
			const SpvReflectDescriptorBinding* binding = set.bindings[b];
			if (!binding)
			{
				continue;
			}

			const uint32_t count = std::max(1u, binding->count);
			const bool isSampler = binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER ||
				binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

			VkDescriptorSetAndBindingMappingEXT mapping{
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_AND_BINDING_MAPPING_EXT,
				.pNext = nullptr,
				.descriptorSet = binding->set,
				.firstBinding = binding->binding,
				.bindingCount = count,
				.resourceMask = VK_SPIRV_RESOURCE_TYPE_ALL_EXT,
				.source = VK_DESCRIPTOR_MAPPING_SOURCE_HEAP_WITH_CONSTANT_OFFSET_EXT,
				.sourceData = {},
			};

			if (isSampler)
			{
				mapping.sourceData.constantOffset = VkDescriptorMappingSourceConstantOffsetEXT{
					.heapOffset = binding->binding * samplerStride,
					.heapArrayStride = samplerStride,
					.pEmbeddedSampler = nullptr,
				};
			}
			else
			{
				mapping.sourceData.constantOffset = VkDescriptorMappingSourceConstantOffsetEXT{
					.heapOffset = binding->binding * resourceStride,
					.heapArrayStride = resourceStride,
					.pEmbeddedSampler = nullptr,
				};
			}

			result.mappings.push_back(mapping);
		}
	}

	result.info.sType = VK_STRUCTURE_TYPE_SHADER_DESCRIPTOR_SET_AND_BINDING_MAPPING_INFO_EXT;
	result.info.pNext = nullptr;
	result.info.mappingCount = static_cast<uint32_t>(result.mappings.size());
	result.info.pMappings = result.mappings.empty() ? nullptr : result.mappings.data();

	return result;
}
} // namespace

namespace Prism::Render::Vulkan
{
VulkanPipelineCache::VulkanPipelineCache()
{
	const auto device = VulkanRenderDevice::Get().GetDevice();

	VkPipelineCacheCreateInfo pipelineCacheCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.initialDataSize = 0,
		.pInitialData = nullptr,
	};

	PE_ASSERT(vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &m_pipelineCache) == VK_SUCCESS);
}

VulkanPipelineCache::~VulkanPipelineCache()
{
	const auto device = VulkanRenderDevice::Get().GetDevice();

	for (auto& [pipeline] : m_graphicsPipelines | std::views::values)
	{
		vkDestroyPipeline(device, pipeline, nullptr);
	}

	for (auto& [pipeline, layout] : m_computePipelines | std::views::values)
	{
		vkDestroyPipeline(device, pipeline, nullptr);
	}

	vkDestroyPipelineCache(device, m_pipelineCache, nullptr);
}

VkPipeline VulkanPipelineCache::GetOrCreatePipeline(const GraphicsPipelineStateDesc& desc,
													const std::vector<Ref<TextureView>>& rtvs, TextureView* dsv)
{
	const auto& renderDevice = VulkanRenderDevice::Get();
	const auto device = renderDevice.GetDevice();
	const auto* bindlessManager = renderDevice.GetBindlessManager();
	auto* compiler = renderDevice.GetVulkanShaderCompiler();

	uint64_t hash = HashPipelineStateDesc(desc, rtvs, dsv);

	if (m_graphicsPipelines.contains(hash))
	{
		return m_graphicsPipelines.at(hash).pipeline;
	}

	const auto& vs = compiler->GetOrCreateShader(desc.vs);
	const auto& ps = compiler->GetOrCreateShader(desc.ps);

	const VkShaderModuleCreateInfo vertexModuleCreateInfo{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = vs.spirv.size() * sizeof(uint32_t),
		.pCode = vs.spirv.data(),
	};

	VkShaderModule vertexShaderModule = VK_NULL_HANDLE;

	PE_ASSERT(vkCreateShaderModule(device, &vertexModuleCreateInfo, nullptr, &vertexShaderModule) == VK_SUCCESS);

	const VkShaderModuleCreateInfo fragmentModuleCreateInfo{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = ps.spirv.size() * sizeof(uint32_t),
		.pCode = ps.spirv.data(),
	};

	VkShaderModule fragmentShaderModule = VK_NULL_HANDLE;

	PE_ASSERT(vkCreateShaderModule(device, &fragmentModuleCreateInfo, nullptr, &fragmentShaderModule) == VK_SUCCESS);

	const auto vsMapping = BuildStageDescriptorMappings(vs.reflection, *bindlessManager);
	const auto psMapping = BuildStageDescriptorMappings(ps.reflection, *bindlessManager);

	const auto vsEntryName = WStringToString(desc.vs.entryName);

	VkPipelineShaderStageCreateInfo vertexStageInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = vsMapping.GetPNext(),
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = vertexShaderModule,
		.pName = vsEntryName.c_str(),
	};

	const auto psEntryName = WStringToString(desc.ps.entryName);

	VkPipelineShaderStageCreateInfo fragmentStageInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = psMapping.GetPNext(),
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = fragmentShaderModule,
		.pName = psEntryName.c_str(),
	};

	VkPipelineShaderStageCreateInfo shaderStagesInfo[] = {vertexStageInfo, fragmentStageInfo};

	const VertexAttributeList layout = vs.reflection.GetVertexAttributeLayout();

	auto vertexInputState = BuildVertexInputState(layout, vs.reflection);
	auto inputAssemblyState = BuildInputAssemblyState(desc);
	auto viewportState = BuildViewportState();
	auto rasterizerState = BuildPipelineRasterizationState(desc);
	auto multisampleState = BuildMultisampleState(desc);
	auto depthState = BuildDepthStencilState(desc);
	auto blendState = BuildColorBlendState(desc, static_cast<uint32_t>(rtvs.size()));
	auto dynamicState = BuildDynamicState(desc);
	auto renderingInfo = BuildRenderingInfo(rtvs, dsv);

	VkPipelineCreateFlags2CreateInfo flags2CreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_CREATE_FLAGS_2_CREATE_INFO,
		.pNext = &renderingInfo.createInfo,
		.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT,
	};

	VkGraphicsPipelineCreateInfo pipelineCreateInfo{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = &flags2CreateInfo,
		.stageCount = 2,
		.pStages = shaderStagesInfo,
		.pVertexInputState = &vertexInputState.createInfo,
		.pInputAssemblyState = &inputAssemblyState,
		.pViewportState = &viewportState,
		.pRasterizationState = &rasterizerState,
		.pMultisampleState = &multisampleState,
		.pDepthStencilState = &depthState,
		.pColorBlendState = &blendState.createInfo,
		.pDynamicState = &dynamicState.createInfo,
		.layout = VK_NULL_HANDLE,
		.renderPass = VK_NULL_HANDLE,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1,
	};

	VkPipeline pipeline = VK_NULL_HANDLE;

	PE_ASSERT(vkCreateGraphicsPipelines(device, m_pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline) == VK_SUCCESS);

	vkDestroyShaderModule(device, vertexShaderModule, nullptr);
	vkDestroyShaderModule(device, fragmentShaderModule, nullptr);

	m_graphicsPipelines.emplace(hash, GraphicsPipeline{.pipeline = pipeline});

	return pipeline;
}

VkPipeline VulkanPipelineCache::GetOrCreatePipeline(const ComputePipelineStateDesc& desc)
{
	const auto& renderDevice = VulkanRenderDevice::Get();
	const auto device = renderDevice.GetDevice();
	const auto* bindlessManager = renderDevice.GetBindlessManager();
	auto* compiler = renderDevice.GetVulkanShaderCompiler();

	const auto& cs = compiler->GetOrCreateShader(desc.cs);

	const uint64_t hash = HashPipelineStateDesc(desc);

	if (m_computePipelines.contains(hash))
	{
		return m_computePipelines.at(hash).pipeline;
	}

	const VkShaderModuleCreateInfo shaderModuleInfo{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = cs.spirv.size() * sizeof(uint32_t),
		.pCode = cs.spirv.data(),
	};

	VkShaderModule shaderModule;

	PE_ASSERT(vkCreateShaderModule(device, &shaderModuleInfo, nullptr, &shaderModule) == VK_SUCCESS);

	std::array shaderReflection{&cs.reflection};

	const auto layout = renderDevice.GetPipelineLayoutCache()->GetOrCreatePipelineLayout(shaderReflection);

	const auto csMapping = BuildStageDescriptorMappings(cs.reflection, *bindlessManager);

	const auto csEntryName = WStringToString(desc.cs.entryName);

	const VkPipelineShaderStageCreateInfo stage{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = csMapping.GetPNext(),
		.stage = VK_SHADER_STAGE_COMPUTE_BIT,
		.module = shaderModule,
		.pName = csEntryName.c_str(),
	};

	constexpr VkPipelineCreateFlags2CreateInfo flags2CreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_CREATE_FLAGS_2_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT,
	};

	const VkComputePipelineCreateInfo info{
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		.pNext = &flags2CreateInfo,
		.stage = stage,
		.layout = VK_NULL_HANDLE,
	};

	VkPipeline pipeline;

	PE_ASSERT(vkCreateComputePipelines(device, m_pipelineCache, 1, &info, nullptr, &pipeline) == VK_SUCCESS);

	vkDestroyShaderModule(device, shaderModule, nullptr);

	m_computePipelines.emplace(hash, ComputePipeline{.pipeline = pipeline, .layout = layout});

	return pipeline;
}

uint64_t VulkanPipelineCache::HashPipelineStateDesc(const GraphicsPipelineStateDesc& desc,
													const std::vector<Ref<TextureView>>& rtvs, const TextureView* dsv) const
{
	uint64_t hash = std::hash<GraphicsPipelineStateDesc>{}(desc);

	auto* compiler = VulkanRenderDevice::Get().GetShaderCompiler();

	hash ^= compiler->GetShaderCodeHash(desc.vs);
	hash ^= compiler->GetShaderCodeHash(desc.ps);

	for (size_t i = 0; i < rtvs.size(); ++i)
	{
		const auto& rtv = rtvs[i];

		if (!rtv)
		{
			continue;
		}

		hash ^= std::hash<TextureViewDesc>{}(rtv->GetViewDesc());
	}

	if (dsv)
	{
		hash ^= std::hash<TextureViewDesc>{}(dsv->GetViewDesc());
	}

	return hash;
}

uint64_t VulkanPipelineCache::HashPipelineStateDesc(const ComputePipelineStateDesc& desc) const
{
	return VulkanRenderDevice::Get().GetShaderCompiler()->GetShaderCodeHash(desc.cs);
}

VertexInputState VulkanPipelineCache::BuildVertexInputState(const VertexAttributeList& vertexAttributeList,
															const VulkanShaderReflection& reflection)
{
	VertexInputState vertexInputState;

	vertexInputState.binding = {
		.binding = 0,
		.stride = static_cast<uint32_t>(GetVertexSize(vertexAttributeList)),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
	};

	uint32_t offset = 0;

	const auto& shaderInputs = reflection.GetVertexInputs();
	for (const auto vertexAttribute : vertexAttributeList)
	{
		const auto it = std::ranges::find_if(shaderInputs, [vertexAttribute](const VulkanShaderVertexInput& input)
											 { return input.attribute == vertexAttribute; });

		if (it == shaderInputs.end())
		{
			offset += GetVertexAttributeSize(vertexAttribute);
			continue;
		}

		vertexInputState.attributes.push_back({
			.location = it->location,
			.binding = 0,
			.format = it->format,
			.offset = offset,
		});

		offset += GetVertexAttributeSize(vertexAttribute);
	}

	vertexInputState.createInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &vertexInputState.binding,
		.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputState.attributes.size()),
		.pVertexAttributeDescriptions = vertexInputState.attributes.data(),
	};

	return vertexInputState;
}

VkPipelineInputAssemblyStateCreateInfo VulkanPipelineCache::BuildInputAssemblyState(const GraphicsPipelineStateDesc& desc)
{
	return {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = GetVkPrimitiveTopology(desc.primitiveTopologyType),
		.primitiveRestartEnable = VK_FALSE,
	};
}

VkPipelineViewportStateCreateInfo VulkanPipelineCache::BuildViewportState()
{
	return {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.scissorCount = 1,
	};
}

VkPipelineRasterizationStateCreateInfo VulkanPipelineCache::BuildPipelineRasterizationState(const GraphicsPipelineStateDesc& desc)
{
	return {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = GetVkPolygonMode(desc.rasterizerState.fillMode),
		.cullMode = GetVkCullModeFlags(desc.rasterizerState.cullMode),
		.frontFace = desc.rasterizerState.frontCounterClockwise ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE,
		.depthBiasEnable = desc.rasterizerState.depthBias != 0,
		.depthBiasConstantFactor = static_cast<float>(desc.rasterizerState.depthBias),
		.depthBiasClamp = 0.0f,
		.depthBiasSlopeFactor = 0.0f,
		.lineWidth = 1.0f,
	};
}

VkPipelineMultisampleStateCreateInfo VulkanPipelineCache::BuildMultisampleState(const GraphicsPipelineStateDesc& desc)
{
	return {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = GetVkSampleCountFlagBits(desc.sampleDesc.count),
		.sampleShadingEnable = VK_FALSE,
		.minSampleShading = 1.0f,
		.pSampleMask = &desc.sampleMask,
		.alphaToCoverageEnable = VK_FALSE,
		.alphaToOneEnable = VK_FALSE,
	};
}

DynamicState VulkanPipelineCache::BuildDynamicState(const GraphicsPipelineStateDesc& desc)
{
	DynamicState state{};

	state.states = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};

	if (desc.vertexStrideMode == VertexStrideMode::Dynamic)
	{
		state.states.push_back(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
	}

	state.createInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = static_cast<uint32_t>(state.states.size()),
		.pDynamicStates = state.states.data(),
	};

	return state;
}

VkPipelineDepthStencilStateCreateInfo VulkanPipelineCache::BuildDepthStencilState(const GraphicsPipelineStateDesc& desc)
{
	const auto& depthStencilState = desc.depthStencilState;

	return {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.depthTestEnable = depthStencilState.depthEnable,
		.depthWriteEnable = depthStencilState.depthWriteEnable,
		.depthCompareOp = GetVkCompareOp(depthStencilState.depthFunc),
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = depthStencilState.stencilEnable,
		.front = GetVkStencilOpState(depthStencilState.frontFace, depthStencilState.stencilReadMask,
									 depthStencilState.stencilWriteMask),
		.back = GetVkStencilOpState(depthStencilState.backFace, depthStencilState.stencilReadMask,
									depthStencilState.stencilWriteMask),
		.minDepthBounds = 0.0f,
		.maxDepthBounds = 1.0f,
	};
}

ColorBlendState VulkanPipelineCache::BuildColorBlendState(const GraphicsPipelineStateDesc& desc,
														  const uint32_t colorAttachmentCount)
{
	ColorBlendState state;

	state.attachments.reserve(colorAttachmentCount);

	for (uint32_t i = 0; i < colorAttachmentCount; i++)
	{
		const RenderTargetBlendDesc* blend = nullptr;

		if (std::holds_alternative<RenderTargetBlendDesc>(desc.blendState.renderTargetBlendDesc))
		{
			blend = &std::get<RenderTargetBlendDesc>(desc.blendState.renderTargetBlendDesc);
		}
		else
		{
			blend = &std::get<RenderTargetBlendDescSeparate>(desc.blendState.renderTargetBlendDesc)[i];
		}

		state.attachments.push_back({
			.blendEnable = blend->blendEnable,
			.srcColorBlendFactor = GetVkBlendFactor(blend->srcBlend),
			.dstColorBlendFactor = GetVkBlendFactor(blend->destBlend),
			.colorBlendOp = GetVkBlendOp(blend->blendOperation),
			.srcAlphaBlendFactor = GetVkBlendFactor(blend->srcBlendAlpha),
			.dstAlphaBlendFactor = GetVkBlendFactor(blend->destBlendAlpha),
			.alphaBlendOp = GetVkBlendOp(blend->blendOperationAlpha),
			.colorWriteMask = GetVkColorComponentFlags(blend->renderTargetWriteMask),
		});
	}

	state.createInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.attachmentCount = static_cast<uint32_t>(state.attachments.size()),
		.pAttachments = state.attachments.data(),
	};

	return state;
}

RenderingInfo VulkanPipelineCache::BuildRenderingInfo(const std::vector<Ref<TextureView>>& rtvs, TextureView* dsv)
{
	RenderingInfo info;

	for (const auto& rtv : rtvs)
	{
		if (!rtv)
		{
			continue;
		}

		info.colorFormats.push_back(GetVkFormat(rtv->GetTexture()->GetTextureDesc().format));
	}

	VkFormat depthFormat = VK_FORMAT_UNDEFINED;

	if (dsv)
	{
		depthFormat = GetVkFormat(dsv->GetViewDesc().format);
	}

	VkFormat stencilFormat = VK_FORMAT_UNDEFINED;

	switch (depthFormat)
	{
	case VK_FORMAT_D24_UNORM_S8_UINT:
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
		stencilFormat = depthFormat;
		break;
	default:
		break;
	}

	info.createInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		.colorAttachmentCount = static_cast<uint32_t>(info.colorFormats.size()),
		.pColorAttachmentFormats = info.colorFormats.data(),
		.depthAttachmentFormat = depthFormat,
		.stencilAttachmentFormat = stencilFormat,
	};

	return info;
}
} // namespace Prism::Render::Vulkan

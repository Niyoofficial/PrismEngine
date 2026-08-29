#include "VulkanPipelineCache.h"

#include "VulkanPipelineLayoutCache.h"
#include "VulkanRenderDevice.h"
#include "VulkanShaderCompiler.h"
#include "VulkanTypeConversions.h"

Prism::Render::Vulkan::VulkanPipelineCache::VulkanPipelineCache()
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

Prism::Render::Vulkan::VulkanPipelineCache::~VulkanPipelineCache()
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

VkPipeline Prism::Render::Vulkan::VulkanPipelineCache::GetOrCreatePipeline(const GraphicsPipelineStateDesc& desc,
                                                                           const std::vector<Ref<TextureView>>& rtvs,
                                                                           TextureView* dsv)
{
	// TODO
	// remove this copy
	GraphicsPipelineStateDesc localDesc = desc;

	PE_RENDER_LOG(Info, "PIPELINE VS: file='{}' entry='{}' type={}", WStringToString(localDesc.vs.filepath),
	              WStringToString(localDesc.vs.entryName), static_cast<int>(localDesc.vs.shaderType));

	PE_RENDER_LOG(Info, "PIPELINE PS: file='{}' entry='{}' type={}", WStringToString(localDesc.ps.filepath),
	              WStringToString(localDesc.ps.entryName), static_cast<int>(localDesc.ps.shaderType));

	auto* compiler = VulkanRenderDevice::Get().GetVulkanShaderCompiler();

	compiler->GetOrCreateShader(localDesc.ps);
	compiler->GetOrCreateShader(localDesc.vs);

	uint64_t hash = HashPipelineStateDesc(localDesc, rtvs, dsv);

	if (m_graphicsPipelines.contains(hash))
	{
		return m_graphicsPipelines.at(hash).pipeline;
	}

	const auto& vs = compiler->GetOrCreateShader(localDesc.vs);
	const auto& ps = compiler->GetOrCreateShader(localDesc.ps);

	const auto device = VulkanRenderDevice::Get().GetDevice();

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

	const auto vsEntryName = WStringToString(localDesc.vs.entryName);

	VkPipelineShaderStageCreateInfo vertexStageInfo{
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
	    .stage = VK_SHADER_STAGE_VERTEX_BIT,
	    .module = vertexShaderModule,
	    .pName = vsEntryName.c_str(),
	};

	const auto psEntryName = WStringToString(localDesc.ps.entryName);

	VkPipelineShaderStageCreateInfo fragmentStageInfo{
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
	    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
	    .module = fragmentShaderModule,
	    .pName = psEntryName.c_str(),
	};

	VkPipelineShaderStageCreateInfo shaderStagesInfo[] = {vertexStageInfo, fragmentStageInfo};

	std::array shaderReflections{&vs.reflection, &ps.reflection};

	auto pipelineLayout = VulkanRenderDevice::Get().GetPipelineLayoutCache()->GetOrCreatePipelineLayout(shaderReflections);

	PE_ASSERT(pipelineLayout != VK_NULL_HANDLE);

	// TODO
	// probably should be moved to GraphicsPipelineStateDesc?
	const VertexAttributeList layout = {
	    VertexAttribute::Position, VertexAttribute::Normal,    VertexAttribute::TexCoord,
	    VertexAttribute::Tangent,  VertexAttribute::Bitangent,
	};

	auto vertexInputState = BuildVertexInputState(layout);
	auto inputAssemblyState = BuildInputAssemblyState(localDesc);
	auto viewportState = BuildViewportState();
	auto rasterizerState = BuildPipelineRasterizationState(localDesc);
	auto multisampleState = BuildMultisampleState(localDesc);
	auto depthState = BuildDepthStencilState(localDesc);
	auto blendState = BuildColorBlendState(localDesc, static_cast<uint32_t>(rtvs.size()));
	auto dynamicState = BuildDynamicState();
	auto renderingInfo = BuildRenderingInfo(rtvs, dsv);

	VkGraphicsPipelineCreateInfo pipelineCreateInfo{
	    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
	    .pNext = &renderingInfo.createInfo,
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
	    .layout = pipelineLayout,
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

VkPipeline Prism::Render::Vulkan::VulkanPipelineCache::GetOrCreatePipeline(const ComputePipelineStateDesc& desc)
{
	uint64_t hash = HashPipelineStateDesc(desc);

	if (m_computePipelines.contains(hash))
	{
		return m_computePipelines.at(hash).pipeline;
	}

	auto* compiler = VulkanRenderDevice::Get().GetVulkanShaderCompiler();

	const auto& cs = compiler->GetOrCreateShader(desc.cs);

	const auto device = VulkanRenderDevice::Get().GetDevice();

	const VkShaderModuleCreateInfo shaderModuleInfo{
	    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
	    .codeSize = cs.spirv.size() * sizeof(uint32_t),
	    .pCode = cs.spirv.data(),
	};

	VkShaderModule shaderModule;

	PE_ASSERT(vkCreateShaderModule(device, &shaderModuleInfo, nullptr, &shaderModule) == VK_SUCCESS);

	std::array shaderReflection{&cs.reflection};

	const auto layout = VulkanRenderDevice::Get().GetPipelineLayoutCache()->GetOrCreatePipelineLayout(shaderReflection);

	const auto csEntryName = WStringToString(desc.cs.entryName);

	const VkPipelineShaderStageCreateInfo stage{
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
	    .stage = VK_SHADER_STAGE_COMPUTE_BIT,
	    .module = shaderModule,
	    .pName = csEntryName.c_str(),
	};

	const VkComputePipelineCreateInfo info{
	    .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
	    .stage = stage,
	    .layout = layout,
	};

	VkPipeline pipeline;

	PE_ASSERT(vkCreateComputePipelines(device, m_pipelineCache, 1, &info, nullptr, &pipeline) == VK_SUCCESS);

	vkDestroyShaderModule(device, shaderModule, nullptr);

	m_computePipelines.emplace(hash, ComputePipeline{.pipeline = pipeline, .layout = layout});

	return pipeline;
}

uint64_t Prism::Render::Vulkan::VulkanPipelineCache::HashPipelineStateDesc(const GraphicsPipelineStateDesc& desc,
                                                                           const std::vector<Ref<TextureView>>& rtvs,
                                                                           const TextureView* dsv) const
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

uint64_t Prism::Render::Vulkan::VulkanPipelineCache::HashPipelineStateDesc(const ComputePipelineStateDesc& desc) const
{
	return VulkanRenderDevice::Get().GetShaderCompiler()->GetShaderCodeHash(desc.cs);
}

Prism::Render::Vulkan::VertexInputState
Prism::Render::Vulkan::VulkanPipelineCache::BuildVertexInputState(const VertexAttributeList& vertexAttributeList)
{
	VertexInputState vertexInputState;

	vertexInputState.binding = {
	    .binding = 0,
	    .stride = static_cast<uint32_t>(GetVertexSize(vertexAttributeList)),
	    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
	};

	uint32_t offset = 0;

	for (auto vertexAttribute : vertexAttributeList)
	{
		VkVertexInputAttributeDescription desc{
		    .location = GetVertexLocation(vertexAttribute),
		    .binding = 0,
		    .format = GetVkFormat(vertexAttribute),
		    .offset = offset,
		};

		vertexInputState.attributes.push_back(desc);

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

VkPipelineInputAssemblyStateCreateInfo
Prism::Render::Vulkan::VulkanPipelineCache::BuildInputAssemblyState(const GraphicsPipelineStateDesc& desc)
{
	return {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
	    .topology = GetVkPrimitiveTopology(desc.primitiveTopologyType),
	    .primitiveRestartEnable = VK_FALSE,
	};
}

VkPipelineViewportStateCreateInfo Prism::Render::Vulkan::VulkanPipelineCache::BuildViewportState()
{
	return {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
	    .viewportCount = 1,
	    .scissorCount = 1,
	};
}

VkPipelineRasterizationStateCreateInfo
Prism::Render::Vulkan::VulkanPipelineCache::BuildPipelineRasterizationState(const GraphicsPipelineStateDesc& desc)
{
	return {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
	    .depthClampEnable = VK_FALSE,
	    .rasterizerDiscardEnable = VK_FALSE,
	    .polygonMode = GetVkPolygonMode(desc.rasterizerState.fillMode),
	    .cullMode = GetVkCullModeFlags(desc.rasterizerState.cullMode),
	    // TODO
	    // do we need to support VK_FRONT_FACE_CLOCKWISE?
	    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
	    .depthBiasEnable = desc.rasterizerState.depthBias != 0,
	    .depthBiasConstantFactor = static_cast<float>(desc.rasterizerState.depthBias),
	    .depthBiasClamp = 0.0f,
	    .depthBiasSlopeFactor = 0.0f,
	    .lineWidth = 1.0f,
	};
}

VkPipelineMultisampleStateCreateInfo
Prism::Render::Vulkan::VulkanPipelineCache::BuildMultisampleState(const GraphicsPipelineStateDesc& desc)
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

Prism::Render::Vulkan::DynamicState Prism::Render::Vulkan::VulkanPipelineCache::BuildDynamicState()
{
	DynamicState state{};

	state.states = {
	    VK_DYNAMIC_STATE_VIEWPORT,
	    VK_DYNAMIC_STATE_SCISSOR,
	};

	state.createInfo = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
	    .dynamicStateCount = static_cast<uint32_t>(state.states.size()),
	    .pDynamicStates = state.states.data(),
	};

	return state;
}

VkPipelineDepthStencilStateCreateInfo
Prism::Render::Vulkan::VulkanPipelineCache::BuildDepthStencilState(const GraphicsPipelineStateDesc& desc)
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

Prism::Render::Vulkan::ColorBlendState
Prism::Render::Vulkan::VulkanPipelineCache::BuildColorBlendState(const GraphicsPipelineStateDesc& desc,
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

Prism::Render::Vulkan::RenderingInfo
Prism::Render::Vulkan::VulkanPipelineCache::BuildRenderingInfo(const std::vector<Ref<TextureView>>& rtvs, TextureView* dsv)
{
	RenderingInfo info;

	for (auto& rtv : rtvs)
	{
		if (!rtv)
		{
			continue;
		}

		info.colorFormats.push_back(GetVkFormat(rtv->GetTexture()->GetTextureDesc().format));
	}

	info.createInfo = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
	    .colorAttachmentCount = static_cast<uint32_t>(info.colorFormats.size()),
	    .pColorAttachmentFormats = info.colorFormats.data(),
	    .depthAttachmentFormat = dsv ? GetVkFormat(dsv->GetTexture()->GetTextureDesc().format) : VK_FORMAT_UNDEFINED,
	    .stencilAttachmentFormat = dsv ? GetVkFormat(dsv->GetTexture()->GetTextureDesc().format) : VK_FORMAT_UNDEFINED,
	};

	return info;
}

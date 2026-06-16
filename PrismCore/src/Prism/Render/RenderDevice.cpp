#include "pcpch.h"
#include "RenderDevice.h"

#include "Prism/Render/RenderCommandQueue.h"
#include "Prism/Render/RenderConstants.h"
#include "Prism/Render/RenderResourceCreation.h"


namespace Prism::Render
{
void RenderDevice::Create(RenderDeviceParams params)
{
	Private::CreateRenderDevice(params);
}

void RenderDevice::Destroy()
{
	StaticPointerSingleton<RenderDevice>::Destroy();
}

void RenderDevice::TryDestroy()
{
	StaticPointerSingleton<RenderDevice>::TryDestroy();
}

RenderDevice& RenderDevice::Get()
{
	return StaticPointerSingleton<RenderDevice>::Get();
}

RenderDevice* RenderDevice::TryGet()
{
	return StaticPointerSingleton<RenderDevice>::TryGet();
}

RenderDevice::RenderDevice(RenderDeviceParams params)
	: m_renderResourceViewCache(this)
{
}

void RenderDevice::InitDeviceSubsystems()
{
	m_shaderCompiler.reset(Private::CreateShaderCompiler());
}

void RenderDevice::BeginRenderFrame()
{
}

void RenderDevice::EndRenderFrame()
{
	// Add a new CPU prepared frame
	m_cpuPreparedFrames.emplace(GetRenderCommandQueue()->GetLastSubmittedCmdListFenceValue());

	GetRenderCommandQueue()->ExecuteGPUCompletionEvents();

	TransferEndFramePreservedObjectsToReleaseQueue();
	ReleaseStaleResources();

	// Pop any completed CPU frames
	while (!m_cpuPreparedFrames.empty() && GetRenderCommandQueue()->GetCompletedFenceValue() >= m_cpuPreparedFrames.front())
		m_cpuPreparedFrames.pop();

	// If there is too many CPU prepared frames, wait for them to be completed by the GPU and pop them
	while (m_cpuPreparedFrames.size() >= Constants::MAX_FRAMES_IN_FLIGHT)
	{
		GetRenderCommandQueue()->WaitForFenceToComplete(m_cpuPreparedFrames.front());
		m_cpuPreparedFrames.pop();
	}
}

void RenderDevice::NotifyResourceDestruction(Buffer* resource)
{
	m_renderResourceViewCache.NotifyResourceDestruction(resource);
}

void RenderDevice::NotifyResourceDestruction(Texture* resource)
{
	m_renderResourceViewCache.NotifyResourceDestruction(resource);
}

Ref<RenderContext> RenderDevice::AllocateContext(std::wstring debugName)
{
	auto context = Ref<RenderContext>::Create(debugName);
	std::wstring eventName = L"RenderContext";
	if (!debugName.empty())
		eventName += L"_" + debugName;

	context->BeginEvent(eventName, {});

	return context;
}

uint64_t RenderDevice::SubmitContext(Ref<RenderContext>& context)
{
	context->EndEvent();
	return GetRenderCommandQueue()->Submit(context);
}

Ref<Buffer> RenderDevice::CreateBuffer(const BufferDesc& desc, RawData initData)
{
	auto buffer = CreateBuffer(desc);

	if (initData.data && initData.sizeInBytes > 0)
	{
		if (desc.usage == ResourceUsage::Dynamic || desc.usage == ResourceUsage::Staging)
		{
			void* address = buffer->Map(CPUAccess::Write);
			memcpy_s(address, desc.size, initData.data, initData.sizeInBytes);
			buffer->Unmap();
		}
		else if (desc.usage == ResourceUsage::Default)
		{
			// TODO: Add copy context
			Ref context = AllocateContext(L"UpdateDefaultBuffer");

			context->UpdateBuffer(buffer, initData);

			SubmitContext(context);
			GetRenderCommandQueue()->Flush(CommandQueueFlushType::WaitForCompletion);
		}
		else
		{
			PE_ASSERT_NO_ENTRY();
		}
	}

	return buffer;
}

Ref<Texture> RenderDevice::CreateTexture(const TextureDesc& desc, BarrierLayout initLayout, RawData initData)
{
	auto texture = CreateTexture(desc, initLayout);

	if (initData.data && initData.sizeInBytes > 0)
	{
		// TODO: Add copy context
		auto context = AllocateContext(L"UpdateTextureWithInitData");
		context->UpdateTexture(texture, initData, 0);

		SubmitContext(context);
		GetRenderCommandQueue()->Flush(CommandQueueFlushType::WaitForCompletion);
	}

	return texture;
}

Ref<Texture> RenderDevice::CreateTexture(const TextureDesc& desc, const Ref<Buffer>& initDataBuffer, BarrierLayout initLayout)
{
	PE_ASSERT(initDataBuffer);
	PE_ASSERT(initDataBuffer->GetBufferDesc().size >= RenderDevice::Get().GetTotalSizeInBytes(desc));

	auto texture = CreateTexture(desc, initLayout);

	Ref uploadBuffer = initDataBuffer;
	if (initDataBuffer->GetBufferDesc().cpuAccess == CPUAccess::Read)
	{
		auto uploadBufferDesc = initDataBuffer->GetBufferDesc();
		uploadBufferDesc.bufferName = desc.textureName + L"_UploadBuffer";
		uploadBufferDesc.usage = ResourceUsage::Staging;
		uploadBufferDesc.cpuAccess = CPUAccess::Write;
		uploadBufferDesc.bindFlags = BindFlags::None;

		void* data = initDataBuffer->Map(CPUAccess::Read);
		uploadBuffer = Buffer::Create(uploadBufferDesc, { .data = data, .sizeInBytes = initDataBuffer->GetBufferDesc().size });
		initDataBuffer->Unmap();
	}

	// TODO: Add copy context
	auto context = AllocateContext(L"UpdateTextureWithInitDataBuffer");
	context->CopyBufferRegion(texture, { 0, 0, 0 }, 0, uploadBuffer, 0);

	SubmitContext(context);
	GetRenderCommandQueue()->Flush(CommandQueueFlushType::WaitForCompletion);

	return texture;
}

Ref<BufferView> RenderDevice::CreateBufferView(const BufferViewDesc& desc, const Ref<Buffer>& buffer)
{
	return m_renderResourceViewCache.GetOrCreateBufferView(desc, buffer);
}

Ref<TextureView> RenderDevice::CreateTextureView(const TextureViewDesc& desc, const Ref<Texture>& texture)
{
	return m_renderResourceViewCache.GetOrCreateTextureView(desc, texture);
}

void RenderDevice::SetBypassCommandRecording(bool bypass)
{
	m_bypassCommandRecording = bypass;
}

bool RenderDevice::GetBypassCommandRecording() const
{
	return m_bypassCommandRecording;
}

void RenderDevice::ReleaseStaleResources()
{
	m_releaseQueue.PurgeReleaseQueue(GetRenderCommandQueue()->GetCompletedFenceValue());
	GetRenderCommandQueue()->ReleaseStaleResources();
}

void RenderDevice::TransferEndFramePreservedObjectsToReleaseQueue()
{
	for (auto& res : m_endFramePreservedObjects.GetPreservedObjects())
		m_releaseQueue.AddResource(std::move(res), GetRenderCommandQueue()->GetLastSubmittedCmdListFenceValue());
	m_endFramePreservedObjects.GetPreservedObjects().clear();
}
}

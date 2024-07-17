#pragma once
#include "Prism-Core/Render/Buffer.h"
#include "Prism-Core/Render/GraphicsPipelineState.h"
#include "Prism-Core/Render/ReleaseQueue.h"


namespace Prism::Render
{
class Buffer;
class BufferView;
class Texture;
class TextureView;

struct DrawCommandDesc
{
	int64_t numVertices = 0;
	int64_t numInstances = 1;
	int64_t startVertexLocation = 0;
};

struct DrawIndexedCommandDesc
{
	int64_t numIndices = 0;
	int64_t numInstances = 1;
	int64_t startIndexLocation = 0;
	int64_t baseVertexLocation = 0;
};

enum class IndexBufferFormat
{
	Uint16,
	Uint32
};

class RenderContext : public RefCounted
{
public:
	virtual ~RenderContext() = default;

	static RenderContext* Create();


	virtual void Draw(DrawCommandDesc desc) = 0;
	virtual void DrawIndexed(DrawIndexedCommandDesc desc) = 0;

	virtual void SetPSO(GraphicsPipelineState* pso) = 0;

	void SetRenderTarget(TextureView* rtv, TextureView* dsv);
	virtual void SetRenderTargets(std::vector<TextureView*> rtvs, TextureView* dsv) = 0;
	void SetViewport(Viewport viewport);
	virtual void SetViewports(std::vector<Viewport> viewports) = 0;
	void SetScissor(Scissor scissor);
	virtual void SetScissors(std::vector<Scissor> scissors) = 0;

	virtual void SetVertexBuffer(Buffer* buffer, int64_t vertexSizeInBytes) = 0;
	virtual void SetIndexBuffer(Buffer* buffer, IndexBufferFormat format) = 0;

	virtual void SetTexture(TextureView* textureView, const std::wstring& paramName) = 0;
	virtual void SetCBuffer(BufferView* bufferView, const std::wstring& paramName) = 0;

	virtual void ClearRenderTargetView(TextureView* rtv, glm::float4* clearColor = nullptr) = 0;
	virtual void ClearDepthStencilView(TextureView* dsv, Flags<ClearFlags> flags, DepthStencilValue* clearValue = nullptr) = 0;

	virtual void Transition(StateTransitionDesc desc) = 0;

	virtual void UpdateBuffer(Buffer* buffer, BufferData data) = 0;

	virtual void CopyBufferRegion(Buffer* dest, int32_t destOffset, Buffer* src, int32_t srcOffset, int32_t numBytes) = 0;


	// Objects MUST be std::move'd into this function
	template<typename T>
	void SafeReleaseResource(T&& resource) requires !std::is_lvalue_reference_v<T>
	{
		struct PreservedResourceWrapper : public PreservedResourceWrapperBase
		{
		public:
			explicit PreservedResourceWrapper(T&& specificObject) requires !std::is_lvalue_reference_v<T>
				: m_resource(std::move(specificObject))
			{
			}

		private:
			T m_resource = nullptr;
		};

		m_preservedResources.emplace_back(new PreservedResourceWrapper{std::move(resource)});
	}


	// Called by RenderDevice before executing
	virtual void CloseContext() = 0;

protected:
	struct PreservedResourceWrapperBase {};
	std::vector<PreservedResourceWrapperBase*> m_preservedResources;
};
}

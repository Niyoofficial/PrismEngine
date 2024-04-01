#pragma once
#include "Prism-Core/Render/Buffer.h"
#include "RenderAPI/D3D12/D3D12Base.h"

namespace Prism::Render::D3D12
{
class D3D12Buffer : public Buffer
{
public:
	D3D12Buffer(const BufferDesc& desc, const std::vector<BufferInitData>& initData);
	D3D12Buffer(ID3D12Resource* resource, const std::wstring& name, ResourceUsage usage);

	virtual BufferDesc GetBufferDesc() const override;

	ID3D12Resource* GetD3D12Resource() const { return m_resource.Get(); }

private:
	BufferDesc m_originalDesc;

	ComPtr<ID3D12Resource> m_resource;
};
}

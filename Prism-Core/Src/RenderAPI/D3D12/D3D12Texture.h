#pragma once
#include "Prism-Core/Render/Texture.h"
#include "RenderAPI/D3D12/D3D12Base.h"

namespace Prism::Render::D3D12
{
class D3D12Texture : public Texture
{
public:
	D3D12Texture(const TextureDesc& desc, RawData initData, BarrierLayout initLayout);
	explicit D3D12Texture(std::wstring filepath, bool loadAsCubemap = false);
	D3D12Texture(ID3D12Resource* resource, const std::wstring& name, ResourceUsage usage, ClearValue optimizedClearValue,
				 bool isCubeTexture = false);

	virtual TextureDesc GetTextureDesc() const override;

	ID3D12Resource* GetD3D12Resource() const { return m_resource.Get(); }

private:
	TextureDesc m_originalDesc;
	ComPtr<ID3D12Resource> m_resource;

	bool m_isMapped = false;
};
}

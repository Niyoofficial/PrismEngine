#pragma once
#include "Prism-Core/Render/Texture.h"
#include "RenderAPI/D3D12/D3D12Base.h"

namespace Prism::Render::D3D12
{
class D3D12Texture : public Texture
{
public:
	explicit D3D12Texture(TextureDesc desc, const std::vector<TextureInitData>& initData = {});
	D3D12Texture(ID3D12Resource* resource, TextureDesc desc);

	ID3D12Resource* GetD3D12Resource() const { return m_resource.Get(); }

	virtual TextureDesc GetTextureDesc() const override;

private:
	TextureDesc m_originalDesc = {};
	ComPtr<ID3D12Resource> m_resource;
};
}

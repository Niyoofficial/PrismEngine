#include "pcpch.h"
#include "TextureView.h"

#include "Prism/Render/RenderResourceCreation.h"

namespace Prism::Render
{
bool TextureViewDesc::Is1D() const
{
	return dimension == ResourceDimension::Tex1D;
}

bool TextureViewDesc::Is2D() const
{
	return dimension == ResourceDimension::Tex2D;
}

bool TextureViewDesc::Is3D() const
{
	return dimension == ResourceDimension::Tex3D;
}

bool TextureViewDesc::IsArray() const
{
	return dimension != ResourceDimension::Tex3D && (subresourceRange.firstArraySlice != 1 || subresourceRange.numArraySlices > 1);
}

bool TextureViewDesc::IsCube() const
{
	return dimension == ResourceDimension::TexCube;
}

TextureView* TextureView::Create(const TextureViewDesc& desc, Texture* texture)
{
	return Private::CreateTextureView(desc, texture);
}

Texture* TextureView::GetTexture() const
{
	PE_ASSERT(m_owningTexture);
	return m_owningTexture;
}
}

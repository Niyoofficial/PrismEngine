﻿#include "pcpch.h"
#include "TextureView.h"

#include "Prism-Core/Render/RenderResourceCreation.h"

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
	return
		firstArrayOrDepthSlice != -1 &&
		arrayOrDepthSlicesCount > 1;
}

bool TextureViewDesc::IsCube() const
{
	return
		dimension == ResourceDimension::TexCube &&
		IsArray() &&
		arrayOrDepthSlicesCount >= 6 &&
		arrayOrDepthSlicesCount % 6 == 0;
}

TextureView* TextureView::Create(const TextureViewDesc& desc, Texture* texture)
{
	return Private::CreateTextureView(desc, texture);
}
}
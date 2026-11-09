#include "pcpch.h"
#include "RenderResourceView.h"

#include "Prism/Render/Buffer.h"
#include "Prism/Render/BufferView.h"
#include "Prism/Render/RenderTypes.h"
#include "Prism/Render/Texture.h"
#include "Prism/Render/TextureView.h"

namespace Prism::Render
{
bool RenderResourceView::IsViewOfDynamicResource() const
{
	if (GetResourceType() == ResourceType::Buffer)
	{
		auto* buffer = GetSubType<BufferView>()->GetBuffer();
		return buffer->GetBufferDesc().usage == ResourceUsage::Dynamic;
	}
	else if (GetResourceType() == ResourceType::Texture)
	{
		auto* texture = GetSubType<TextureView>()->GetTexture();
		return texture->GetTextureDesc().usage == ResourceUsage::Dynamic;
	}
	PE_ASSERT_NO_ENTRY();
	return false;
}
}

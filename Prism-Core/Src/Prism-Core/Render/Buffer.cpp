#include "pcpch.h"
#include "Buffer.h"
#include "RenderResourceCreation.h"

namespace Prism::Render
{
Buffer* Buffer::Create(const BufferDesc& desc, const BufferInitData& initData)
{
	return Buffer::Create(desc, std::vector{initData});
}

Buffer* Buffer::Create(const BufferDesc& desc, const std::vector<BufferInitData>& initData)
{
	return Private::CreateBuffer(desc, initData);
}
}

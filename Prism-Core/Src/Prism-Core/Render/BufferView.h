﻿#pragma once
#include "Prism-Core/Render/RenderResourceView.h"

namespace Prism::Render
{
struct BufferViewDesc
{
	int64_t offset = 0;
	int64_t size = 0;
};

class BufferView : public RenderResourceView
{
public:
	static BufferView* Create(const BufferViewDesc& desc, class Buffer* buffer);

	ResourceType GetResourceType() const override { return ResourceType::Buffer; }

	virtual Buffer* GetBuffer() const = 0;
};
}
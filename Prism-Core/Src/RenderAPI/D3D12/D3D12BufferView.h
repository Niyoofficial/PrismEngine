#pragma once
#include "Prism-Core/Render/BufferView.h"
#include "RenderAPI/D3D12/D3D12DescriptorHeapManager.h"

namespace Prism::Render::D3D12
{
class D3D12BufferView : public BufferView
{
public:
	D3D12BufferView(const BufferViewDesc& desc, Buffer* buffer);

	void BuildView();

	const CPUDescriptorHeapAllocation& GetDescriptor() const { return m_descriptor; }

private:
	BufferViewDesc m_viewDesc;
	CPUDescriptorHeapAllocation m_descriptor;
};
}

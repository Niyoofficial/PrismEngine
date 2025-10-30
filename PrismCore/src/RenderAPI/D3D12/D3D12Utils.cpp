#include "pcpch.h"
#include "D3D12Utils.h"

#include <Windows.h>

#include "Prism/Render/RenderTypes.h"
#include "Prism/Render/TextureView.h"

namespace Prism::Render::D3D12
{
std::wstring GetHResultMessage(HRESULT hr)
{
	LPWSTR messageRaw;
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		hr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPWSTR>(&messageRaw), 0,
		nullptr);

	std::wstring message = messageRaw;
	LocalFree(messageRaw);
	return message;
}

bool VerifyHResult(HRESULT hr)
{
	bool fail = FAILED(hr);
	if (fail)
		PE_CORE_LOG(Error, "{} - {}", hr, WStringToString(GetHResultMessage(hr)));

	return !fail;
}

bool ShouldD3D12ViewBeCreatedAsArray(const TextureViewDesc& viewDesc)
{
	return viewDesc.subresourceRange.firstArraySlice != 0 || viewDesc.subresourceRange.numArraySlices > 1;
}
}

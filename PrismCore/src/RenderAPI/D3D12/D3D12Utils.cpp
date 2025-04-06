#include "pcpch.h"
#include "D3D12Utils.h"

#include <Windows.h>

#include "Prism/Render/RenderTypes.h"
#include "Prism/Render/Shader.h"

namespace Prism::Render::D3D12
{
bool VerifyHResult(HRESULT hr)
{
	bool fail = FAILED(hr);
	if (fail)
	{
		LPWSTR errorString;
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			hr,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			reinterpret_cast<LPWSTR>(&errorString), 0,
			nullptr);

		PE_CORE_LOG(Error, "{} - {}", hr, WStringToString(errorString));

		LocalFree(errorString);
	}

	return !fail;
}
}

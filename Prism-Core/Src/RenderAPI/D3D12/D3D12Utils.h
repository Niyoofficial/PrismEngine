#pragma once
#include <dxgiformat.h>
#include <intsafe.h>


namespace Prism::Render
{
enum class ShaderType;
enum class TextureFormat;
}

namespace Prism::D3D12
{
// Returns whether the HRESULT is successful or not
bool VerifyHResult(HRESULT hr);
}

#define PE_ASSERT_HR(expr)							\
	do												\
	{												\
		if (!Prism::D3D12::VerifyHResult((expr)))	\
			PE_ASSERT_BREAK_INSTRUCTION;			\
	}												\
	while (0)

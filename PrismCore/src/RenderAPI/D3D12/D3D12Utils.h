#pragma once
#include <intsafe.h>


namespace Prism::Render
{
struct TextureViewDesc;
enum class ShaderType : uint8_t;
enum class TextureFormat;
}

namespace Prism::Render::D3D12
{
// Returns whether the HRESULT is successful or not
bool VerifyHResult(HRESULT hr);

bool ShouldD3D12ViewBeCreatedAsArray(const TextureViewDesc& viewDesc);
}

#ifdef PE_ENABLE_ASSERTS
	#define PE_ASSERT_HR(expr)									\
		do														\
		{														\
			if (!Prism::Render::D3D12::VerifyHResult((expr)))	\
				PE_ASSERT_BREAK_INSTRUCTION;					\
		}														\
		while (0)
#else
	#define PE_ASSERT_HR(expr) do { if (!(expr)) (void)0; } while (0)
#endif

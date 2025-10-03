#pragma once
#include "Prism/Render/Material.h"
#include "Prism/Render/Primitive.h"

namespace Prism::Render
{
enum class IndexBufferFormat;
class RenderContext;
class Primitive;

// A collection of primitives
class PrimitiveBatch : public RefCounted
{
public:
	struct PrimitiveWithMaterial
	{
		Ref<Primitive> primitive;
		Material material;
	};
public:
	explicit PrimitiveBatch(const std::wstring& primitiveBatchName);

	void AddPrimitive(int64_t vertexSize, IndexBufferFormat indexFormat,
					  void* vertexBuffer, int64_t vertexCount, void* indexBuffer, int64_t indexCount,
					  std::wstring primitiveBufferParamName, int64_t primitiveBufferSize,
					  const Material& material, Bounds3f bounds);
	const std::vector<PrimitiveWithMaterial>& GetPrimitives() const { return m_primitives; }
	Bounds3f GetBounds() const { return m_bounds; }

	void Draw(RenderContext* renderContext, void* cbufferData, int64_t dataSize, bool bBindMaterial = true);

private:
	std::wstring m_primitiveBatchName;

	std::vector<PrimitiveWithMaterial> m_primitives;
	Bounds3f m_bounds;
};
}

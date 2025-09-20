#pragma once

namespace Prism
{
namespace MeshLoading
{
class MeshAsset;
}

class VertexFactory;

struct RenderProxy
{
	
};

struct MeshRendererComponent
{
public:
	void SetPrimitive(MeshLoading::MeshAsset* mesh, int32_t primitiveIndex);

private:
	Ref<MeshLoading::MeshAsset> m_meshAsset;
	int32_t m_primitiveIndex = -1;
};
}

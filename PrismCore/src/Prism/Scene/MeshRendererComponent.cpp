#include "MeshRendererComponent.h"

namespace Prism
{
void MeshRendererComponent::SetPrimitive(MeshLoading::MeshAsset* mesh, int32_t primitiveIndex)
{
	PE_ASSERT(mesh && primitiveIndex > 0);

	m_meshAsset = mesh;
	m_primitiveIndex = primitiveIndex;
}
}

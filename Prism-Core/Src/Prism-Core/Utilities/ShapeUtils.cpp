#include "pcpch.h"
#include "ShapeUtils.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"


namespace Prism::ShapeUtils
{
void ProcessMesh(ShapeData& data, aiNode* node, aiMesh* mesh, const aiScene* scene)
{
	data.vertices.resize(mesh->mNumVertices);
	for (int32_t i = 0; i < (int32_t)mesh->mNumVertices; ++i)
	{
		if (mesh->HasPositions())
		{
			aiVector3D& position = mesh->mVertices[i];
			position *= node->mTransformation;

			data.vertices[i].position = { position.x, position.y, position.z };
		}

		if (mesh->HasNormals())
		{
			aiVector3D& normal = mesh->mNormals[i];
			data.vertices[i].normal = {normal.x, normal.y, normal.z};
		}

		if (mesh->HasTextureCoords(0))
		{
			aiVector3D& texCoords = mesh->mTextureCoords[0][i];
			data.vertices[i].texCoord = {texCoords.x, texCoords.y};
		}

		if (mesh->HasTangentsAndBitangents())
		{
			aiVector3D& tangent = mesh->mTangents[i];
			data.vertices[i].tangent = {tangent.x, tangent.y, tangent.z};

			aiVector3D& bitangent = mesh->mTangents[i];
			data.vertices[i].bitangent = {bitangent.x, bitangent.y, bitangent.z};
		}

		if (mesh->HasVertexColors(0))
		{
			aiColor4D& vertColor = mesh->mColors[0][i];
			data.vertices[i].vertexColor = {vertColor.r, vertColor.g, vertColor.b, vertColor.a};
		}
	}

	PE_ASSERT(mesh->HasFaces());

	for (int32_t i = 0; i < (int32_t)mesh->mNumFaces; ++i)
	{
		aiFace& face = mesh->mFaces[i];
		for (int32_t j = 0; j < (int32_t)face.mNumIndices; ++j)
			data.indices.push_back((int32_t)face.mIndices[j]);
	}

	// TODO: Load textures
}

void ProcessNode(ShapeData& data, aiNode* node, const aiScene* scene)
{
	for (int32_t i = 0; i < (int32_t)node->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		ProcessMesh(data, node, mesh, scene);
	}

	for (int32_t i = 0; i < (int32_t)node->mNumChildren; ++i)
		ProcessNode(data, node->mChildren[i], scene);
}

ShapeData LoadShapeFromFile(const std::wstring& filename)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(WStringToString(filename),
											 aiProcess_Triangulate |
											 aiProcess_ConvertToLeftHanded |
											 aiProcess_CalcTangentSpace |
											 aiProcess_FixInfacingNormals);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		PE_ASSERT(false, "{}", importer.GetErrorString());
		return {};
	}

	ShapeData data;
	ProcessNode(data, scene->mRootNode, scene);

	return data;
}
}

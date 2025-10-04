#include "pcpch.h"
#include "MeshLoading.h"

#include "assimp/DefaultLogger.hpp"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "Prism/Render/Texture.h"


DECLARE_LOG_CATEGORY(AssimpLog, "Assimp");

namespace Prism::MeshLoading
{
glm::float4x4 AssimpMatToPrismMat(const aiMatrix4x4& aiMat)
{
	glm::float4x4 mat;
	mat[0][0] = aiMat.a1;
	mat[0][1] = aiMat.a2;
	mat[0][2] = aiMat.a3;
	mat[0][3] = aiMat.a4;
	mat[1][0] = aiMat.b1;
	mat[1][1] = aiMat.b2;
	mat[1][2] = aiMat.b3;
	mat[1][3] = aiMat.b4;
	mat[2][0] = aiMat.c1;
	mat[2][1] = aiMat.c2;
	mat[2][2] = aiMat.c3;
	mat[2][3] = aiMat.c4;
	mat[3][0] = aiMat.d1;
	mat[3][1] = aiMat.d2;
	mat[3][2] = aiMat.d3;
	mat[3][3] = aiMat.d4;
	return mat;
}

class PrismLogger : public Assimp::Logger
{
public:
	static void Create(Log::LogVerbosity verbosity)
	{
		Assimp::DefaultLogger::set(new PrismLogger(verbosity));
	}

	explicit PrismLogger(Log::LogVerbosity verbosity)
		: m_verbosity(verbosity)
	{
	}

	bool attachStream(Assimp::LogStream* pStream, unsigned severity) override
	{
		return false;
	}
	bool detachStream(Assimp::LogStream* pStream, unsigned severity) override
	{
		return false;
	}

protected:
	void OnDebug(const char* message) override
	{
		if (m_verbosity > Log::LogVerbosity::Trace)
			return;

		PE_LOG(AssimpLog, Info, message);
	}
	void OnVerboseDebug(const char* message) override
	{
		if (m_verbosity > Log::LogVerbosity::Trace)
			return;

		PE_LOG(AssimpLog, Trace, message);
	}
	void OnInfo(const char* message) override
	{
		if (m_verbosity > Log::LogVerbosity::Info)
			return;

		PE_LOG(AssimpLog, Info, message);
	}
	void OnWarn(const char* message) override
	{
		if (m_verbosity > Log::LogVerbosity::Warn)
			return;

		PE_LOG(AssimpLog, Warn, message);
	}
	void OnError(const char* message) override
	{
		if (m_verbosity > Log::LogVerbosity::Error)
			return;

		PE_LOG(AssimpLog, Error, message);
	}

protected:
	Log::LogVerbosity m_verbosity;
};

void ProcessPrimitive(PrimitiveData& data, aiNode* node, aiMesh* mesh, const aiScene* scene,
					  const std::wstring& meshName, std::vector<Ref<Render::Texture>>& textures, glm::float4x4 transform)
{
	for (int32_t i = 0; i < (int32_t)mesh->mNumVertices; ++i)
	{
		VertexData vertex;
		if (mesh->HasPositions())
		{
			aiVector3D position = mesh->mVertices[i];
			//position *= node->mTransformation;

			vertex.position = {position.x, position.y, position.z};
		}

		if (mesh->HasNormals())
		{
			aiVector3D aiNormal = mesh->mNormals[i];
			glm::float4 normal = {aiNormal.x, aiNormal.y, aiNormal.z, 0.f};
			//normal = normal * transform;

			vertex.normal = glm::normalize(normal);
		}

		if (mesh->HasTextureCoords(0))
		{
			aiVector3D texCoords = mesh->mTextureCoords[0][i];
			vertex.texCoords = {texCoords.x, texCoords.y};
		}

		if (mesh->HasTangentsAndBitangents())
		{
			aiVector3D aiTangent = mesh->mTangents[i];
			glm::float3 tangent = {aiTangent.x, aiTangent.y, aiTangent.z};
			//tangent = tangent * transform;
			vertex.tangent = {tangent.x, tangent.y, tangent.z};

			aiVector3D aiBitangent = mesh->mTangents[i];
			glm::float3 bitangent = {aiBitangent.x, aiBitangent.y, aiBitangent.z};
			//bitangent = bitangent * transform;
			vertex.bitangent = {bitangent.x, bitangent.y, bitangent.z};
		}

		if (mesh->HasVertexColors(0))
		{
			aiColor4D vertColor = mesh->mColors[0][i];
			vertex.vertexColor = {vertColor.r, vertColor.g, vertColor.b, vertColor.a};
		}

		data.vertices.push_back(vertex);
		data.bounds += vertex.position;
	}

	PE_ASSERT(mesh->HasFaces());

	for (int32_t i = 0; i < (int32_t)mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		for (int32_t j = 0; j < (int32_t)face.mNumIndices; ++j)
			data.indices.push_back(face.mIndices[j]);
	}

	if (scene->HasMaterials())
	{
		std::array textureTypesToTry = {
			aiTextureType_BASE_COLOR,
			aiTextureType_NORMALS,
			aiTextureType_METALNESS,
			aiTextureType_DIFFUSE_ROUGHNESS
		};

		for (aiTextureType textureType : textureTypesToTry)
		{
			PE_ASSERT(scene->mNumMaterials > mesh->mMaterialIndex);
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			int32_t textureCount = (int32_t)material->GetTextureCount(textureType);

			if (textureCount > 1)
			{
				PE_CORE_LOG(Warn, "Texture count for texture type {} is {}, only first texture will be read",
				   aiTextureTypeToString(textureType), textureCount);
			}

			auto assimpTextureTypeToPrismTextureType =
				[](aiTextureType assimpType)
				{
					switch (assimpType)
					{
					case aiTextureType_BASE_COLOR:
						return TextureType::Albedo;
					case aiTextureType_NORMALS:
						return TextureType::Normals;
					case aiTextureType_METALNESS:
						return TextureType::Metallic;
					case aiTextureType_DIFFUSE_ROUGHNESS:
						return TextureType::Roughness;
					default:
						PE_ASSERT_NO_ENTRY();
						return TextureType::Albedo;
					}
				};

			if (textureCount > 0)
			{
				aiString path;
				material->GetTexture(textureType, 0, &path);
				std::wstring texturePath = L"textures/" + meshName + L"/" + StringToWString(path.C_Str());

				PE_ASSERT(!scene->GetEmbeddedTexture(path.C_Str()), "Embedded textures are not supported! {}", path.C_Str());

				// Check if this texture has already been loaded
				bool foundTexture = false;
				for (auto& texture : textures)
				{
					PE_ASSERT(texture);
					if (texture->GetTextureDesc().textureName == texturePath)
					{
						data.textures[assimpTextureTypeToPrismTextureType(textureType)] = texture;
						foundTexture = true;
						break;
					}
				}

				if (!foundTexture)
				{
					Ref<Render::Texture> texture = Render::Texture::Create(texturePath, false, false);
					data.textures[assimpTextureTypeToPrismTextureType(textureType)] = texture;
					textures.push_back(texture);
				}
			}
		}
	}
}

void ProcessNode(MeshData& data, aiNode* node, const aiScene* scene, const std::wstring& meshName, std::vector<Ref<Render::Texture>>& textures, glm::float4x4 parentTransform)
{
	glm::float4x4 nodeTransform = AssimpMatToPrismMat(node->mTransformation);
	glm::float4x4 transform = parentTransform * nodeTransform;

	for (int32_t i = 0; i < (int32_t)node->mNumMeshes; ++i)
	{
		PrimitiveData primitiveData;
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		ProcessPrimitive(primitiveData, node, mesh, scene, meshName, textures, transform);
		data.primitives.push_back(primitiveData);
		data.bounds += primitiveData.bounds;
	}

	for (int32_t i = 0; i < (int32_t)node->mNumChildren; ++i)
		ProcessNode(data, node->mChildren[i], scene, meshName, textures, transform);
}

void InitMeshLoading()
{
	PrismLogger::Create(Log::LogVerbosity::Warn);
}

MeshData LoadMeshFromFile(const std::wstring& filePath)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(WStringToString(filePath),
											 aiProcess_Triangulate |
											 aiProcess_ConvertToLeftHanded |
											 aiProcess_OptimizeMeshes |
											 aiProcess_ValidateDataStructure |
											 aiProcess_PreTransformVertices |
											 aiProcess_GlobalScale |
											 aiProcess_SortByPType |
											 aiProcess_JoinIdenticalVertices |
											 aiProcess_GenSmoothNormals |
											 aiProcess_CalcTangentSpace |
											 aiProcess_GenUVCoords);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		PE_ASSERT(false, "{}", importer.GetErrorString());
		return {};
	}

	std::wstring meshName = filePath.substr(filePath.find_last_of(L'/') + 1);
	meshName = meshName.substr(0, meshName.find_last_of(L'.'));

	std::vector<Ref<Render::Texture>> textures;

	MeshData data;
	ProcessNode(data, scene->mRootNode, scene, meshName, textures, glm::float4x4(1.f));

	for (auto& texture : textures)
	{
		PE_ASSERT(texture);
		texture->WaitForLoadFinish();
	}

	return data;
}

MeshNodeIterator::MeshNodeIterator(MeshNode node)
	: value(node)
{
}

MeshNode MeshNodeIterator::operator*() const
{
	return value;
}

void MeshNodeIterator::operator++()
{
	++value;
}

void MeshNodeIterator::operator--()
{
	--value;
}

MeshAsset::MeshAsset(const std::wstring& filePath)
{
	const aiScene* scene = m_importer.ReadFile(WStringToString(filePath).c_str(),
											   aiProcess_Triangulate |
											   aiProcess_ConvertToLeftHanded |
											   aiProcess_OptimizeMeshes |
											   aiProcess_ValidateDataStructure |
											   aiProcess_PreTransformVertices |
											   aiProcess_GlobalScale |
											   aiProcess_SortByPType |
											   aiProcess_JoinIdenticalVertices |
											   aiProcess_GenSmoothNormals |
											   aiProcess_CalcTangentSpace |
											   aiProcess_GenUVCoords);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		PE_ASSERT(false, "{}", m_importer.GetErrorString());

	std::function<void(aiNode*)> processNode =
		[this, &processNode, scene](aiNode* assimpNode)
		{
			m_nodes.emplace_back(assimpNode, (MeshNode)m_nodes.size() - 1);
			MeshNode currNode = (MeshNode)(m_nodes.size() - 1);
			for (int32_t i = 0; i < assimpNode->mNumMeshes; ++i)
				m_nodes.emplace_back(scene->mMeshes[assimpNode->mMeshes[i]], currNode);

			for (int32_t i = 0; i < assimpNode->mNumChildren; ++i)
				processNode(assimpNode->mChildren[i]);
		};

	processNode(scene->mRootNode);
}

MeshNode MeshAsset::GetRootNode() const
{
	return 0;
}

int32_t MeshAsset::GetNodeChildrenCount(MeshNode node) const
{
	if (std::holds_alternative<aiNode*>(m_nodes[node].assimpNode))
	{
		return
			std::get<aiNode*>(m_nodes[node].assimpNode)->mNumChildren +
			std::get<aiNode*>(m_nodes[node].assimpNode)->mNumMeshes;
	}
	return 0;
}

MeshNode MeshAsset::GetNodeChild(MeshNode node, int32_t index) const
{
	if (std::holds_alternative<aiNode*>(m_nodes[node].assimpNode))
	{
		aiNode* assimpNode = std::get<aiNode*>(m_nodes[node].assimpNode);
		std::variant<aiNode*, aiMesh*> assimpChild;
		if (index >= assimpNode->mNumChildren)
			assimpChild = m_importer.GetScene()->mMeshes[assimpNode->mMeshes[index - assimpNode->mNumChildren]];
		else
			assimpChild = assimpNode->mChildren[index];

		auto it = std::ranges::find_if(m_nodes, 
			[&assimpChild](auto info)
			{
				return info.assimpNode == assimpChild;
			});
		if (it != m_nodes.end())
			return (MeshNode)(it - m_nodes.begin());
	}

	return -1;
}

MeshNode MeshAsset::GetNodeParent(MeshNode node) const
{
	return m_nodes[node].parent;
}

bool MeshAsset::DoesNodeContainVertices(MeshNode node) const
{
	return std::holds_alternative<aiMesh*>(m_nodes[node].assimpNode);
}

int64_t MeshAsset::GetNodeVertexCount(MeshNode node) const
{
	if (DoesNodeContainVertices(node))
		return std::get<aiMesh*>(m_nodes[node].assimpNode)->mNumVertices;

	return 0;
}

std::wstring MeshAsset::GetNodeName(MeshNode node)
{
	if (std::holds_alternative<aiNode*>(m_nodes[node].assimpNode))
		return StringToWString(std::get<aiNode*>(m_nodes[node].assimpNode)->mName.C_Str());
	else if (std::holds_alternative<aiMesh*>(m_nodes[node].assimpNode))
		return StringToWString(std::get<aiMesh*>(m_nodes[node].assimpNode)->mName.C_Str());
	return {};
}

Bounds3f MeshAsset::GetBoundingBox(MeshNode node) const
{
	PE_ASSERT(DoesNodeContainVertices(node));

	aiAABB aabb = std::get<aiMesh*>(m_nodes[node].assimpNode)->mAABB;
	return {{aabb.mMin.x, aabb.mMin.y, aabb.mMin.z}, {aabb.mMax.x, aabb.mMax.y, aabb.mMax.z}};
}

glm::float3 MeshAsset::GetPosition(MeshNode node, int32_t vertexIndex) const
{
	PE_ASSERT(DoesNodeContainVertices(node));

	auto aiPos = std::get<aiMesh*>(m_nodes[node].assimpNode)->mVertices[vertexIndex];
	return {aiPos.x, aiPos.y, aiPos.z};
}

glm::float3 MeshAsset::GetNormal(MeshNode node, int32_t vertexIndex) const
{
	PE_ASSERT(DoesNodeContainVertices(node));

	auto aiNorm = std::get<aiMesh*>(m_nodes[node].assimpNode)->mNormals[vertexIndex];
	return {aiNorm.x, aiNorm.y, aiNorm.z};
}

glm::float2 MeshAsset::GetTexCoord(MeshNode node, int32_t vertexIndex, int32_t texCoordIndex) const
{
	PE_ASSERT(DoesNodeContainVertices(node));

	auto aiTexCoord = std::get<aiMesh*>(m_nodes[node].assimpNode)->mTextureCoords[texCoordIndex][vertexIndex];
	return {aiTexCoord.x, aiTexCoord.y};
}

glm::float3 MeshAsset::GetTangent(MeshNode node, int32_t vertexIndex) const
{
	PE_ASSERT(DoesNodeContainVertices(node));

	auto aiTan = std::get<aiMesh*>(m_nodes[node].assimpNode)->mTangents[vertexIndex];
	return { aiTan.x, aiTan.y, aiTan.z };
}

glm::float3 MeshAsset::GetBitangent(MeshNode node, int32_t vertexIndex) const
{
	PE_ASSERT(DoesNodeContainVertices(node));

	auto aiBitan = std::get<aiMesh*>(m_nodes[node].assimpNode)->mBitangents[vertexIndex];
	return {aiBitan.x, aiBitan.y, aiBitan.z};
}

glm::float4 MeshAsset::GetColor(MeshNode node, int32_t vertexIndex, int32_t colorIndex) const
{
	PE_ASSERT(DoesNodeContainVertices(node));

	auto aiColor = std::get<aiMesh*>(m_nodes[node].assimpNode)->mColors[colorIndex][vertexIndex];
	return {aiColor.r, aiColor.g, aiColor.b, aiColor.a};
}

std::vector<uint32_t> MeshAsset::GetIndices(MeshNode node) const
{
	PE_ASSERT(std::holds_alternative<aiMesh*>(m_nodes[node].assimpNode));
	aiMesh* mesh = std::get<aiMesh*>(m_nodes[node].assimpNode);
	PE_ASSERT(mesh->HasFaces());

	std::vector<uint32_t> indices;
	for (int32_t i = 0; i < (int32_t)mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		for (int32_t j = 0; j < (int32_t)face.mNumIndices; ++j)
			indices.push_back(face.mIndices[j]);
	}

	return indices;
}

int64_t MeshAsset::GetIndexCount(MeshNode node) const
{
	PE_ASSERT(std::holds_alternative<aiMesh*>(m_nodes[node].assimpNode));
	aiMesh* mesh = std::get<aiMesh*>(m_nodes[node].assimpNode);
	PE_ASSERT(mesh->HasFaces());

	// We triangulate all the meshes on import, so we can assume 3 indices per face
	return (int64_t)mesh->mNumFaces * 3;
}

MeshNodeIterator MeshAsset::begin() const
{
	return 0;
}

MeshNodeIterator MeshAsset::end() const
{
	return (MeshNode)m_nodes.size();
}
}

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

void InitMeshLoading()
{
	PrismLogger::Create(Log::LogVerbosity::Warn);
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

static std::unordered_map<TextureType, Ref<Render::Texture>> LoadTexturesForMesh(const std::wstring& filePath, const aiScene* scene, aiMesh* assimpMesh, std::vector<Ref<Render::Texture>>& loadedTextures)
{
	if (!scene->HasMaterials())
		return {};

	std::unordered_map<TextureType, Ref<Render::Texture>> textures;

	std::array textureTypesToTry = {
		aiTextureType_BASE_COLOR,
		aiTextureType_NORMALS,
		aiTextureType_METALNESS,
		aiTextureType_DIFFUSE_ROUGHNESS
	};

	for (aiTextureType textureType : textureTypesToTry)
	{
		PE_ASSERT(scene->mNumMaterials > assimpMesh->mMaterialIndex);
		aiMaterial* material = scene->mMaterials[assimpMesh->mMaterialIndex];
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

			const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(path.C_Str());

			std::wstring texturePath = embeddedTexture
				? StringToWString(embeddedTexture->mFilename.C_Str())
				: (std::fs::path(filePath).parent_path() / path.C_Str()).generic_wstring();

			// Check if this texture has already been loaded
			bool foundTexture = false;
			for (auto& texture : loadedTextures)
			{
				PE_ASSERT(texture);
				if (texture->GetTextureDesc().textureName == texturePath)
				{
					textures[assimpTextureTypeToPrismTextureType(textureType)] = texture;
					foundTexture = true;
					break;
				}
			}

			if (!foundTexture)
			{
				Ref<Render::Texture> texture;
				if (embeddedTexture)
				{
					texture = Render::Texture::CreateFromMemory(texturePath, embeddedTexture->pcData,
						embeddedTexture->mHeight
						? embeddedTexture->mHeight * embeddedTexture->mWidth
						: embeddedTexture->mWidth, false, false);
				}
				else
				{
					texture = Render::Texture::CreateFromFile(texturePath, false, false);
				}

				textures[assimpTextureTypeToPrismTextureType(textureType)] = texture;
				loadedTextures.push_back(texture);
			}
		}
	}

	return textures;
}

MeshAsset::MeshAsset(const std::wstring& filePath)
	: m_filePath(filePath)
{
	const aiScene* scene = m_importer.ReadFile(WStringToString(filePath).c_str(),
											   aiProcess_Triangulate |
											   aiProcess_ConvertToLeftHanded |
											   //aiProcess_OptimizeMeshes |
											   aiProcess_ValidateDataStructure |
											   aiProcess_PreTransformVertices |
											   aiProcess_GlobalScale |
											   aiProcess_SortByPType |
											   //aiProcess_JoinIdenticalVertices |
											   aiProcess_GenSmoothNormals |
											   aiProcess_CalcTangentSpace |
											   aiProcess_GenUVCoords |
											   aiProcess_GenBoundingBoxes
	);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		PE_ASSERT(false, "{}", m_importer.GetErrorString());

	std::vector<Ref<Render::Texture>> loadedTextures;

	std::function<void(aiNode*)> processNode =
		[this, &processNode, scene, &loadedTextures, &filePath](aiNode* assimpNode)
		{
			m_nodes.emplace_back(assimpNode, (MeshNode)m_nodes.size() - 1);
			MeshNode currNode = (MeshNode)(m_nodes.size() - 1);
			for (int32_t i = 0; i < assimpNode->mNumMeshes; ++i)
			{
				aiMesh* assimpMesh = scene->mMeshes[assimpNode->mMeshes[i]];

				NodeInfo nodeInfo = {
					.assimpNode = assimpMesh,
					.parent = currNode,
					.textures = LoadTexturesForMesh(filePath, scene, assimpMesh, loadedTextures)
				};

				m_nodes.push_back(nodeInfo);
			}

			for (int32_t i = 0; i < assimpNode->mNumChildren; ++i)
				processNode(assimpNode->mChildren[i]);
		};

	processNode(scene->mRootNode);

	for (auto& texture : loadedTextures)
	{
		PE_ASSERT(texture);
		texture->WaitForLoadFinish();
	}
}

std::wstring MeshAsset::GetLoadedFilepath() const
{
	return m_filePath;
}

MeshNode MeshAsset::GetRootNode() const
{
	return 0;
}

int32_t MeshAsset::GetTotalNodeCount() const
{
	return m_nodes.size();
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

std::wstring MeshAsset::GetNodeName(MeshNode node) const
{
	if (std::holds_alternative<aiNode*>(m_nodes[node].assimpNode))
		return StringToWString(std::get<aiNode*>(m_nodes[node].assimpNode)->mName.C_Str());
	else if (std::holds_alternative<aiMesh*>(m_nodes[node].assimpNode))
		return StringToWString(std::get<aiMesh*>(m_nodes[node].assimpNode)->mName.C_Str());
	return {};
}

Render::Texture* MeshAsset::GetNodeTexture(MeshNode node, TextureType type)
{
	if (m_nodes[node].textures.contains(type))
		return m_nodes[node].textures.at(type);
	return nullptr;
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

	auto* assimpMesh = std::get<aiMesh*>(m_nodes[node].assimpNode);
	if (assimpMesh->mTextureCoords[texCoordIndex])
	{
		auto aiTexCoord = assimpMesh->mTextureCoords[texCoordIndex][vertexIndex];
		return {aiTexCoord.x, aiTexCoord.y};
	}
	return {};
}

glm::float3 MeshAsset::GetTangent(MeshNode node, int32_t vertexIndex) const
{
	PE_ASSERT(DoesNodeContainVertices(node));

	auto* assimpMesh = std::get<aiMesh*>(m_nodes[node].assimpNode);
	if (assimpMesh->mTangents)
	{
		auto aiTan = assimpMesh->mTangents[vertexIndex];
		return {aiTan.x, aiTan.y, aiTan.z};
	}
	return {};
}

glm::float3 MeshAsset::GetBitangent(MeshNode node, int32_t vertexIndex) const
{
	PE_ASSERT(DoesNodeContainVertices(node));

	auto* assimpMesh = std::get<aiMesh*>(m_nodes[node].assimpNode);
	if (assimpMesh->mBitangents)
	{
		auto aiBitan = assimpMesh->mBitangents[vertexIndex];
		return {aiBitan.x, aiBitan.y, aiBitan.z};
	}
	return {};
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

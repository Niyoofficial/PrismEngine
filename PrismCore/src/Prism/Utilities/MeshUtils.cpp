#include "pcpch.h"
#include "MeshUtils.h"

#include "assimp/DefaultLogger.hpp"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "Prism/Render/Texture.h"


DECLARE_LOG_CATEGORY(AssimpLog, "Assimp");

namespace Prism::MeshUtils
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
			vertex.texCoord = {texCoords.x, texCoords.y};
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
}

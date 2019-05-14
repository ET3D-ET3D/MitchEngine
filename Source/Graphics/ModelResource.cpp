#include "PCH.h"

#include "ModelResource.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <unordered_map>

#include "Logger.h"
#include "Renderer.h"
#include "Graphics/Texture.h"
#include "Game.h"
#include "Graphics/Material.h"
#include "Resource/ResourceCache.h"
#include "Graphics/Mesh.h"
#include "Scene/Node.h"
#include <stack>

ModelResource::ModelResource(const FilePath& path)
	: Resource(path)
{
	Load();
}

ModelResource::~ModelResource()
{
	Resource::~Resource();
}

void ModelResource::Load()
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(Path.FullPath.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
		return;
	}

	ProcessNode(scene->mRootNode, scene, RootNode);
}

std::vector<Moonlight::Mesh*> ModelResource::GetAllMeshes()
{
	std::vector<Moonlight::Mesh*> meshes;
	std::stack<Moonlight::Node*> nodes;
	nodes.push(&RootNode);
	while (!nodes.empty())
	{
		Moonlight::Node& child = *nodes.top();
		nodes.pop();
		for (Moonlight::Mesh* childMesh : child.Meshes)
		{
			meshes.push_back(childMesh);
		}
		for (Moonlight::Node& childNode : child.Nodes)
		{
			nodes.push(&childNode);
		}
	}
	return meshes;
}

void ModelResource::SetShader(Moonlight::Shader* shader)
{
	ModelShader = shader;
}

void ModelResource::ProcessNode(aiNode *node, const aiScene *scene, Moonlight::Node& parent)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		parent.Meshes.push_back(ProcessMesh(mesh, scene));
	}
	parent.Nodes.emplace_back();
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene, parent.Nodes.back());
	}
}

Moonlight::Mesh* ModelResource::ProcessMesh(aiMesh *mesh, const aiScene *scene)
{
	std::vector<Moonlight::Vertex> vertices;
	std::vector<unsigned int> indices;
	Moonlight::Material* newMaterial = new Moonlight::Material();
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Moonlight::Vertex vertex;
		DirectX::XMFLOAT3 vector;

		vertex.Position = { mesh->mVertices[i].x,  mesh->mVertices[i].y,  mesh->mVertices[i].z };

		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.Normal = vector;

		if (mesh->mTextureCoords[0])
		{
			DirectX::XMFLOAT2 vec;

			// A vertex can contain up to 8 different texture coordinates. We assume that we won't use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TextureCoord = vec;
		}
		else
		{
			vertex.TextureCoord = DirectX::XMFLOAT2(0.0f, 0.0f);
		}
		if (mesh->mTangents)
		{
			vector.x = mesh->mTangents[i].x;
			vector.y = mesh->mTangents[i].y;
			vector.z = mesh->mTangents[i].z;
			//vertex.Tangent = vector;
		}
		if (mesh->mBitangents)
		{
			vector.x = mesh->mBitangents[i].x;
			vector.y = mesh->mBitangents[i].y;
			vector.z = mesh->mBitangents[i].z;
			//vertex.Bitangent = vector;
		}
		vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];

		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

	LoadMaterialTextures(newMaterial, material, aiTextureType_DIFFUSE, Moonlight::TextureType::Diffuse);
	LoadMaterialTextures(newMaterial, material, aiTextureType_SPECULAR, Moonlight::TextureType::Specular);
	LoadMaterialTextures(newMaterial, material, aiTextureType_NORMALS, Moonlight::TextureType::Normal);
	LoadMaterialTextures(newMaterial, material, aiTextureType_HEIGHT, Moonlight::TextureType::Height);
	LoadMaterialTextures(newMaterial, material, aiTextureType_OPACITY, Moonlight::TextureType::Opacity);
	Moonlight::Mesh* output = new Moonlight::Mesh(vertices, indices, newMaterial);
	output->Name = std::string(mesh->mName.C_Str());
	return output;
}

void ModelResource::LoadMaterialTextures(Moonlight::Material* newMaterial, aiMaterial *mat, aiTextureType type, const Moonlight::TextureType& typeName)
{
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		std::string stdString = std::string(str.C_Str());
		if (stdString != ".")
		{
			std::string& texturePath = stdString;
			std::shared_ptr<Moonlight::Texture> texture;
			if (stdString.find(":") != std::string::npos)
			{
				texture = ResourceCache::GetInstance().Get<Moonlight::Texture>(FilePath(texturePath));
			}
			else
			{
				texture = ResourceCache::GetInstance().Get<Moonlight::Texture>(FilePath(Path.Directory + texturePath));
			}
			texture->Type = typeName;
			newMaterial->SetTexture(typeName, texture);
		}
	}
}

Moonlight::Shader* ModelResource::GetShader() const
{
	return ModelShader;
}

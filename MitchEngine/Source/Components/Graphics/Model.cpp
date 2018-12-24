#include "PCH.h"
#include "Model.h"
#include "Utility/Logger.h"

FbxManager* Model::g_pFbxSdkManager = nullptr;
Model::Model(std::string const &path, const std::string& shader, bool gamma /*= false*/) : EnableGammaCorrection(gamma)
{
	Load(path);
	ModelShader = new Shader((shader + ".vert"), (shader + ".frag"));
}

Model::~Model()
{
}

void Model::Draw(std::shared_ptr<DX::DeviceResources> deviceResources)
{
	for (unsigned int i = 0; i < Meshes.size(); i++)
	{
		Meshes[i].Draw(*ModelShader);
	}
}

void Model::Init()
{
}
//
void Model::Load(std::string const &path)
{
	if (g_pFbxSdkManager == nullptr)
	{
		g_pFbxSdkManager = FbxManager::Create();

		FbxIOSettings* pIOsettings = FbxIOSettings::Create(g_pFbxSdkManager, IOSROOT);
		g_pFbxSdkManager->SetIOSettings(pIOsettings);
	}

	FbxImporter* importer = FbxImporter::Create(g_pFbxSdkManager, "");
	FbxScene* scene = FbxScene::Create(g_pFbxSdkManager, "");

	bool success = importer->Initialize(path.c_str(), -1, g_pFbxSdkManager->GetIOSettings());

	if (!success)
	{
		Logger::GetInstance().Log(Logger::LogType::Error, "[Model] Failed to find " + path);
		return;
	}

	success = importer->Import(scene);
	if (!success)
	{
		Logger::GetInstance().Log(Logger::LogType::Error, "[Model] Failed to load scene" + path);
		return;
	}

	importer->Destroy();
	FbxNode* rootNode = scene->GetRootNode();
	if (rootNode)
	{
		for (int i = 0; i < rootNode->GetChildCount(); ++i)
		{
			FbxNode* child = rootNode->GetChild(i);

			if (!child->GetNodeAttribute())
			{
				continue;
			}

			FbxNodeAttribute::EType attributeType = child->GetNodeAttribute()->GetAttributeType();

			if (attributeType != FbxNodeAttribute::eMesh)
			{
				continue;
			}

			FbxMesh* mesh = static_cast<FbxMesh*>(child->GetNodeAttribute());
			Meshes.push_back(ProcessMesh(mesh, scene));
		}
	}
	//	Assimp::Importer importer;
	//	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	//	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	//	{
	//		std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
	//		return;
	//	}
	//
	//	Directory = path.substr(0, path.find_last_of('/'));
	//
	//	ProcessNode(scene->mRootNode, scene);
}
//
//void Model::ProcessNode(aiNode *node, const aiScene *scene)
//{
//	for (unsigned int i = 0; i < node->mNumMeshes; i++)
//	{
//		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
//		Meshes.push_back(ProcessMesh(mesh, scene));
//	}
//
//	for (unsigned int i = 0; i < node->mNumChildren; i++)
//	{
//		ProcessNode(node->mChildren[i], scene);
//	}
//}
//
Mesh Model::ProcessMesh(FbxMesh* mesh, const FbxScene* scene)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture*> textures;

	FbxVector4* verts = mesh->GetControlPoints();
	for (int i = 0; i < mesh->GetPolygonCount(); ++i)
	{
		int numVerts = mesh->GetPolygonSize(i);

		if (numVerts != 3)
		{
			Logger::GetInstance().Log(Logger::LogType::Error, "[Mesh] is not triangulated.");
		}

		for (int j = 0; j < numVerts; ++j)
		{
			Vertex vertex;
			glm::vec3 vector;

			vector.x = verts[j].mData[0];
			vector.y = verts[i].mData[1];
			vector.z = verts[i].mData[2];

			vertex.Position = vector;

			vertices.push_back(vertex);
		}
	}

	return Mesh(vertices, indices, textures);
	//	std::vector<Vertex> vertices;
	//	std::vector<unsigned int> indices;
	//	std::vector<Texture*> textures;
	//
	//	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	//	{
	//		Vertex vertex;
	//		glm::vec3 vector;
	//
	//		vector.x = mesh->mVertices[i].x;
	//		vector.y = mesh->mVertices[i].y;
	//		vector.z = mesh->mVertices[i].z;
	//		vertex.Position = vector;
	//
	//		if (mesh->HasNormals())
	//		{
	//			vector.x = mesh->mNormals[i].x;
	//			vector.y = mesh->mNormals[i].y;
	//			vector.z = mesh->mNormals[i].z;
	//			vertex.Normal = vector;
	//		}
	//
	//		{
	//			glm::vec2 vec;
	//
	//			// A vertex can contain up to 8 different texture coordinates. We assume that we won't use models where a vertex can have multiple texture coordinates so we always take the first set (0).
	//			vec.x = mesh->mTextureCoords[0][i].x;
	//			vec.y = mesh->mTextureCoords[0][i].y;
	//			vertex.TexCoords = vec;
	//		}
	//		if (mesh->mTangents)
	//		{
	//			vector.x = mesh->mTangents[i].x;
	//			vector.y = mesh->mTangents[i].y;
	//			vector.z = mesh->mTangents[i].z;
	//			vertex.Tangent = vector;
	//		}
	//		if (mesh->mBitangents)
	//		{
	//			vector.x = mesh->mBitangents[i].x;
	//			vector.y = mesh->mBitangents[i].y;
	//			vector.z = mesh->mBitangents[i].z;
	//			vertex.Bitangent = vector;
	//		}
	//		vertices.push_back(vertex);
	//	}
	//
	//	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	//	{
	//		aiFace face = mesh->mFaces[i];
	//
	//		for (unsigned int j = 0; j < face.mNumIndices; j++)
	//		{
	//			indices.push_back(face.mIndices[j]);
	//		}
	//	}
	//
	//	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	//
	//	LoadMaterialTextures(textures, material, aiTextureType_DIFFUSE, "texture_diffuse");
	//	LoadMaterialTextures(textures, material, aiTextureType_SPECULAR, "texture_specular");
	//	LoadMaterialTextures(textures, material, aiTextureType_NORMALS, "texture_normal");
	//	LoadMaterialTextures(textures, material, aiTextureType_HEIGHT, "texture_height");
	//
	//	return Mesh(vertices, indices, textures);
}
//
//void Model::LoadMaterialTextures(std::vector<Texture*>& textures, aiMaterial *mat, aiTextureType type, std::string typeName)
//{
//	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
//	{
//		aiString str;
//		mat->GetTexture(type, i, &str);
//
//		std::string fileName = str.C_Str();
//		//fileName = fileName.substr(fileName.find_last_of('\\')+1, fileName.size()-1);
//
//		if (fileName != ".")
//		{
//			Texture* texture = ResourceCache::GetInstance().Get<Texture>(Directory + "/" + fileName);
//			texture->type = typeName;
//			texture->path = str.C_Str();
//			textures.push_back(texture);
//		}
//	}
//}

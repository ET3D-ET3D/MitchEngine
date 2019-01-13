#include "FBXModel.h"

#include <glm.hpp>

#include <fbxsdk/utils/fbxgeometryconverter.h>
#include <unordered_map>

#include "Graphics/FBXModel.h"
#include "Logger.h"
#include "Renderer.h"

namespace Moonlight
{
	FbxManager* FBXModel::g_pFbxSdkManager = nullptr;

	FBXModel::FBXModel()
	{

	}

	FBXModel::FBXModel(std::string const &path)
		: Path(path)
	{

	}

	FBXModel::~FBXModel()
	{

	}

	void FBXModel::Load()
	{
		if (g_pFbxSdkManager == nullptr)
		{
			g_pFbxSdkManager = FbxManager::Create();

			FbxIOSettings* pIOsettings = FbxIOSettings::Create(g_pFbxSdkManager, IOSROOT);
			g_pFbxSdkManager->SetIOSettings(pIOsettings);
		}

		FbxImporter* importer = FbxImporter::Create(g_pFbxSdkManager, "");
		FbxScene* scene = FbxScene::Create(g_pFbxSdkManager, "");

		bool success = importer->Initialize(Path.c_str(), -1, g_pFbxSdkManager->GetIOSettings());

		if (!success)
		{
			Logger::GetInstance().Log(Logger::LogType::Error, "[Model] Failed to find " + Path);
			return;
		}

		success = importer->Import(scene);
		if (!success)
		{
			Logger::GetInstance().Log(Logger::LogType::Error, "[Model] Failed to load scene" + Path);
			return;
		}

		importer->Destroy();
		FbxNode* rootNode = scene->GetRootNode();
		FbxGeometryConverter* converter = new FbxGeometryConverter(g_pFbxSdkManager);
		if (rootNode)
		{
			int vertexCounter = 0;
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
				converter->Triangulate(child->GetNodeAttribute(), true);
				FbxMesh* mesh = static_cast<FbxMesh*>(child->GetNodeAttribute());
				Meshes.push_back(ProcessMesh(mesh, scene, vertexCounter, child));
				vertexCounter = 0;
			}
		}
		delete converter;
	}

	FBXModel* FBXModel::Load(const std::string& InFilePath)
	{
		FBXModel* model = new FBXModel(InFilePath);

		// TODO ASync
		{
			model->Load();
		}

		return model;
	}

	void FBXModel::Draw()
	{
		for (Mesh& mesh : Meshes)
		{
			mesh.Draw(*ModelShader);
		}
	}

	void FBXModel::SetShader(Shader* shader)
	{
		ModelShader = shader;
	}

	Mesh FBXModel::ProcessMesh(FbxMesh* mesh, const FbxScene* scene, int& vertexCounter, FbxNode* child)
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::vector<Texture*> textures;

		int triCount = mesh->GetPolygonCount();

		std::unordered_map<glm::vec3, unsigned int> mapping;
		for (int i = 0; i < triCount; ++i)
		{
			int numVerts = mesh->GetPolygonSize(i);

			if (numVerts != 3)
			{
				Logger::GetInstance().Log(Logger::LogType::Error, "[Mesh] is not triangulated.");
			}

			for (int j = 0; j < numVerts; ++j)
			{
				FbxVector4* verts = mesh->GetControlPoints();
				int controlPointIndex = mesh->GetPolygonVertex(i, j);
				FbxVector4 currControlPoint = verts[controlPointIndex];

				Vertex vertex;
				DirectX::XMFLOAT3 vector;

				vector.x = currControlPoint[0];
				vector.y = currControlPoint[1];
				vector.z = currControlPoint[2];

				vertex.Position = vector;
				vertex.Normal = ReadNormal(mesh, controlPointIndex, vertexCounter);

				for (int k = 0; k < 1; ++k)
				{
					// A vertex can contain up to 8 different texture coordinates. We assume that we won't use models where a vertex can have multiple texture coordinates so we always take the first set (0).
					//			vec.x = mesh->mTextureCoords[0][i].x;
					//			vec.y = mesh->mTextureCoords[0][i].y;
					//vertex.TexCoords = ReadUV(mesh, controlPointIndex, mesh->GetTextureUVIndex(i, j), k);
				}


				vertices.push_back(vertex);
				indices.push_back(vertexCounter);
				++vertexCounter;
			}

		}
		//FillMap(mapping, vertices);
		//MakeIndices(mapping, vertices, indices);

		LoadMaterial(child, mesh, textures);

		return Mesh(vertices, indices, textures);
	}

	glm::vec2 FBXModel::ReadUV(FbxMesh* mesh, int controlPointIndex, int textureUVIndex, int uvLayer)
	{
		glm::vec2 uv = glm::vec2(0, 0);
		if (uvLayer >= 2 || mesh->GetElementUVCount() <= uvLayer)
		{
			return uv;
		}

		FbxGeometryElementUV* vertexUV = mesh->GetElementUV(uvLayer);
		FbxLayerElement::EMappingMode mappingMode = vertexUV->GetMappingMode();
		FbxLayerElement::EReferenceMode refMode = vertexUV->GetReferenceMode();

		switch (mappingMode)
		{
		case fbxsdk::FbxLayerElement::eNone:
			break;
		case fbxsdk::FbxLayerElement::eByControlPoint:
			switch (refMode)
			{
			case fbxsdk::FbxLayerElement::eDirect:
				(uv)[0] = (float)vertexUV->GetDirectArray().GetAt(controlPointIndex)[0];
				(uv)[1] = (float)vertexUV->GetDirectArray().GetAt(controlPointIndex)[1];
				break;
			case fbxsdk::FbxLayerElement::eIndex:
				break;
			case fbxsdk::FbxLayerElement::eIndexToDirect:
			{
				int id = vertexUV->GetIndexArray().GetAt(controlPointIndex);
				(uv)[0] = (float)vertexUV->GetDirectArray().GetAt(id)[0];
				(uv)[1] = (float)vertexUV->GetDirectArray().GetAt(id)[1];
			}
			break;
			default:
				break;
			}
			break;
		case fbxsdk::FbxLayerElement::eByPolygonVertex:
			switch (refMode)
			{
			case fbxsdk::FbxLayerElement::eDirect:
			case fbxsdk::FbxLayerElement::eIndexToDirect:
				(uv)[0] = (float)vertexUV->GetDirectArray().GetAt(textureUVIndex)[0];
				(uv)[1] = (float)vertexUV->GetDirectArray().GetAt(textureUVIndex)[1];
				break;
			default:
				break;
			}
			break;
		case fbxsdk::FbxLayerElement::eByPolygon:
			break;
		case fbxsdk::FbxLayerElement::eByEdge:
			break;
		case fbxsdk::FbxLayerElement::eAllSame:
			break;
		default:
			break;
		}

		return uv;
	}

	DirectX::XMFLOAT3 FBXModel::ReadNormal(FbxMesh* mesh, int controlPointIndex, int vertextCounter)
	{
		DirectX::XMFLOAT3 normal;

		if (mesh->GetElementNormalCount() < 1)
		{
			throw std::exception("Invalid normals");
		}

		FbxGeometryElementNormal* vertexNormal = mesh->GetElementNormal(0);
		switch (vertexNormal->GetMappingMode())
		{
		case FbxGeometryElement::eByControlPoint:
		case FbxGeometryElement::eByPolygonVertex:
		{
			switch (vertexNormal->GetReferenceMode())
			{
			case FbxGeometryElement::eDirect:
			{
				normal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(controlPointIndex).mData[0]);
				normal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(controlPointIndex).mData[1]);
				normal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(controlPointIndex).mData[2]);
			}
			break;
			case FbxGeometryElement::eIndexToDirect:
			{
				int index = vertexNormal->GetIndexArray().GetAt(controlPointIndex);
				normal.x = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[0]);
				normal.y = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[1]);
				normal.z = static_cast<float>(vertexNormal->GetDirectArray().GetAt(index).mData[2]);
			}
			break;
			}
		}
		break;
		default:
			throw std::exception("Invalid reference");
			break;
		}
		return normal;
	}

	void FBXModel::LoadMaterial(FbxNode* node, FbxMesh* mesh, std::vector<Texture*>& materials)
	{
		unsigned int materialCount = node->GetMaterialCount();

		for (unsigned int i = 0; i < materialCount; ++i)
		{
			FbxSurfaceMaterial* surfaceMaterial = node->GetMaterial(i);

			LoadMaterialTextures(surfaceMaterial, materials);
		}

	}

	void FBXModel::LoadMaterialTextures(FbxSurfaceMaterial* material, std::vector<Texture*>& materials)
	{
		unsigned int textureIndex = 0;
		FbxProperty property;

		FBXSDK_FOR_EACH_TEXTURE(textureIndex)
		{
			property = material->FindProperty(FbxLayerElement::sTextureChannelNames[textureIndex]);
			if (property.IsValid())
			{
				unsigned int textureCount = property.GetSrcObjectCount<FbxTexture>();
				for (unsigned int i = 0; i < textureCount; ++i)
				{
					FbxLayeredTexture* layeredTexture = property.GetSrcObject<FbxLayeredTexture>(i);
					if (layeredTexture)
					{
						throw std::exception("Layered texture is unsupported atm");
					}
					else
					{
						FbxTexture* texture = property.GetSrcObject<FbxTexture>(i);
						if (texture)
						{
							std::string textureType = property.GetNameAsCStr();
							FbxFileTexture* fileTexture = FbxCast<FbxFileTexture>(texture);
							if (fileTexture)
							{
								std::string filename = fileTexture->GetFileName();

								Texture* loadedTexture = Renderer::GetInstance().GetResources().Get<Texture>(filename);
								loadedTexture->path = filename;
								loadedTexture->type = textureType;
								Logger::GetInstance().Log(Logger::LogType::Info, "Loaded Texture Type: " + loadedTexture->type);

								materials.push_back(loadedTexture);
							}
						}
					}
				}
			}
		}
	}

}
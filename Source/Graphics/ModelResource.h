#pragma once 
#include "Graphics/Texture.h"
#include "Graphics/Mesh.h"
#include "Resource/Resource.h"
#include "FilePath.h"
#include "Graphics/Mesh.h"
#include <string>
#include <vector>
#include "Graphics/Material.h"
#include "assimp/material.h"
#include "assimp/mesh.h"
#include "assimp/scene.h"
#include "Graphics/Shader.h"

class ModelResource
	: public Resource
{
	friend class RenderCore;
public:
	ModelResource();
	ModelResource(const FilePath& path);
	~ModelResource();

	void SetShader(Moonlight::Shader* shader);

	std::vector<Moonlight::Mesh*> Meshes;
private:
	void Load();

	void ProcessNode(aiNode *node, const aiScene *scene);

	Moonlight::Mesh* ProcessMesh(aiMesh *mesh, const aiScene *scene);

	void LoadMaterialTextures(Moonlight::Material* newMaterial, aiMaterial *mat, aiTextureType type, const Moonlight::TextureType& typeName);

	Moonlight::Shader* ModelShader = nullptr;
public:
	Moonlight::Shader* GetShader() const;
};

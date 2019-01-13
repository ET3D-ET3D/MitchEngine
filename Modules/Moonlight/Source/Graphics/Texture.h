#pragma once
#include <string>
#include "Dementia.h"
#include "Resource/Resource.h"

class Texture : public Resource
{
public:
	unsigned int Id;
	std::string Type;
	std::string Directory;

	int Width;
	int Height;

	Texture();
	~Texture();

	static Texture* Load(const std::string& InFilePath);

	// Textures should not be copied around in memory
	ME_NONCOPYABLE(Texture);
	int nrChannels;
	std::string type;
	std::string path;
};
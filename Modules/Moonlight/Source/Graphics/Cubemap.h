#pragma once
#include <string>
#include "Dementia.h"
#include "Resource/Resource.h"

class Cubemap
	: public Resource
{
public:
	unsigned int Id;
	std::string Type;
	std::string Directory;

	int Width;
	int Height;

	Cubemap(const Path& InFilePath);
	~Cubemap();

	// Textures should not be copied around in memory
	ME_NONCOPYABLE(Cubemap);
	int nrChannels;
};
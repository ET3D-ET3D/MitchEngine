#pragma once
#include <vector>
#include <map>
#include <string>
#include "Resource.h"
#include "Path.h"
#include "Singleton.h"
#include "ClassTypeId.h"
#include <memory>
#include "Logger.h"

class Resource;

class ResourceCache
{
	ResourceCache();
	~ResourceCache();

public:
	size_t GetCacheSize();

	template<class T, typename... Args>
	std::shared_ptr<T> Get(const Path& InFilePath, Args&& ... args);

	void TryToDestroy(Resource* resource);

private:

	std::map<std::string, std::shared_ptr<Resource>> ResourceStack;
	ME_SINGLETON_DEFINITION(ResourceCache)
};

template<class T, typename... Args>
std::shared_ptr<T> ResourceCache::Get(const Path& InFilePath, Args&& ... args)
{
	TypeId id = ClassTypeId<Resource>::GetTypeId<T>();
	auto I = ResourceStack.find(InFilePath.FullPath);
	if (I != ResourceStack.end())
	{
		std::shared_ptr<T> Res = std::dynamic_pointer_cast<T>(I->second);
		return Res;
	}
	if (!InFilePath.Exists)
	{
		YIKES("Failed to load resource: " + InFilePath.FullPath);
		return {};
	}

	std::shared_ptr<T> Res = std::make_shared<T>(InFilePath, std::forward<Args>(args)...);
	Res->Resources = this;
	ResourceStack[InFilePath.FullPath] = Res;
	return Res;
}

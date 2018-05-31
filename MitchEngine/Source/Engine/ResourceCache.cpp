#include "ResourceCache.h"

namespace MAN
{
	ResourceCache::ResourceCache()
	{
		Push();
	}

	ResourceCache::~ResourceCache()
	{
		Pop();
	}

	void ResourceCache::Push()
	{
		ResourceStack.push_back(std::map<std::string, Resource*>());
	}

	void ResourceCache::Pop()
	{
		auto& V = ResourceStack[ResourceStack.size() - 1];
		for (auto I = V.begin(); I != V.end(); ++I)
		{
			delete I->second;
		}
		ResourceStack.pop_back();
	}

	size_t ResourceCache::GetStackSize()
	{
		return ResourceStack.size();
	}
}
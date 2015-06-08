#pragma once
#include <vector>
#include <map>
#include <string>
#include "Resource.h"

namespace ma {
	class ResourceCache {
	public:
		ResourceCache();
		~ResourceCache();

		void Push();

		void Pop();

		size_t GetStackSize();

		template<class T>
		T* Get(const std::string& InFilePath);
	private:
		std::vector<std::map<std::string, ma::Resource*>> ResourceStack;
	};

	template<class T>
	T* ma::ResourceCache::Get(const std::string& InFilePath) {
		std::map<std::string, Resource*>::iterator I;
		for (int i = ResourceStack.size() - 1; i >= 0; i--)
		{
			I = ResourceStack[i].find(InFilePath);
			if (I != ResourceStack[i].end()) {
				T* Res = dynamic_cast<T*>(I->second);
				assert(Res);
				return Res;
			}
		}
		T* Res = T::Load(InFilePath);
		Res->Resources = this;
		ResourceStack[ResourceStack.size() - 1][InFilePath] = Res;
		return Res;
	}

}
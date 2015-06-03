#pragma once
#include "Component.h"
#include "ComponentTypeArray.h"
#include "ClassTypeId.h"

namespace ma {
	class ComponentFilter {
	public:
		ComponentFilter() = default;

		~ComponentFilter() = default;

		template<typename C>
		ComponentFilter& Requires() {
			//static_assert(std::is_base_of<BaseComponent, C>(), "C doesn't inherit from Component");
			CheckCapacity(RequiredComponentsList, C::GetTypeId());
			RequiredComponentsList[C::GetTypeId()] = true;
			return *this;
		}

		bool PassFilter(const ComponentTypeArray& InComponentTypeArray) const;

		void Clear();

	private:
		ComponentTypeArray RequiredComponentsList;

		ComponentTypeArray RequiresOneOfComponentsList;

		ComponentTypeArray ExcludeComponentsList;
	};


}
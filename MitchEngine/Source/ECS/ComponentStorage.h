#pragma once
#include <vector>
#include <memory>

#include "ECS/Entity.h"
#include "ECS/Component.h"
#include "ECS/ComponentTypeArray.h"

#include "Utility/Util.h"

class ComponentStorage
{
public:
	ComponentStorage(std::size_t InEntityAmount);
	~ComponentStorage();

	MAN_NONCOPYABLE(ComponentStorage)
	MAN_NONMOVABLE(ComponentStorage)

	void AddComponent(Entity& InEntity, BaseComponent* InComponent, TypeId InComponentTypeId);

	BaseComponent& GetComponent(const Entity& InEntity, TypeId InTypeId);

	ComponentTypeArray GetComponentTypes(const Entity& InEntity) const;

	void RemoveComponent(const Entity& InEntity, TypeId InTypeId);

	void RemoveAllComponents(Entity& InEntity);

	void Resize(std::size_t InAmount);
private:
	typedef std::vector<std::unique_ptr<BaseComponent> > ImplComponentArray;

	struct EntityComponents
	{
		EntityComponents() = default;

		EntityComponents(EntityComponents&& E) :
			Components(std::move(E.Components)),
			ComponentTypeList(std::move(E.ComponentTypeList))
		{
		}
		ImplComponentArray Components;

		ComponentTypeArray ComponentTypeList;
	};

	std::vector<EntityComponents> ComponentEntries;

	ImplComponentArray& GetComponents_Implementation(const Entity& E);

	const ImplComponentArray& GetComponents_Implementation(const Entity& E) const;
};
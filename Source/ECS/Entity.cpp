#include "PCH.h"
#include "Entity.h"
#include "Engine/World.h"
#include "ComponentDetail.h"
#include "Logger.h"

Entity::Entity()
{
}

Entity::Entity(World& inWorld, EntityID inId) :
	Id(inId),
	GameWorld(&inWorld)
{
}

Entity::~Entity()
{
}

const bool Entity::HasComponent(TypeId inComponentType) const
{
	ComponentTypeArray ar = GameWorld->EntityAttributes.Storage.GetComponentTypes(*this);

	return ar[inComponentType] == true;
}

void Entity::AddComponent(BaseComponent* inComponent, TypeId inComponentTypeId)
{
	GameWorld->EntityAttributes.Storage.AddComponent(*this, inComponent, inComponentTypeId);
	SetActive(true);
}

BaseComponent* Entity::AddComponentByName(const std::string& inComponent)
{
	ComponentRegistry& reg = getComponentRegistry();
	ComponentRegistry::iterator it = reg.find(inComponent);

	if (it == reg.end()) {
		Logger::GetInstance().Log(Logger::LogType::Warning, "Factory not found for component " + inComponent);
		return nullptr;
	}

	CreateComponentFunc func = it->second;
	SetActive(true);
	return func(*this);
}

const EntityID& Entity::GetId() const
{
	return Id;
}

BaseComponent& Entity::GetComponent(TypeId InTypeId) const
{
	return GameWorld->EntityAttributes.Storage.GetComponent(*this, InTypeId);
}

void Entity::SetActive(const bool InActive)
{
	GameWorld->ActivateEntity(*this, InActive);
}

bool Entity::operator==(const Entity& entity) const
{
	return Id == entity.Id && entity.GameWorld == GameWorld;
}

void Entity::RemoveComponent(TypeId InComponentTypeId)
{
	GameWorld->EntityAttributes.Storage.RemoveComponent(*this, InComponentTypeId);
}

std::vector<BaseComponent*> Entity::GetAllComponents()
{
	return GameWorld->EntityAttributes.Storage.GetAllComponents(*this);
}
#include "EntityIdPool.h"

using namespace ma;

EntityIdPool::EntityIdPool(std::size_t InPoolSize) :
DefaultPoolSize(InPoolSize),
NextId(0),
Entities(InPoolSize) {

}

Entity::ID EntityIdPool::Create() {
	Entity::ID Id;

	if (!FreeList.empty()) {
		Id = FreeList.back();
		FreeList.pop_back();
	}
	else {
		Id.Index = NextId++;
		Entities[Id.Index] = Id.Counter = 1;
	}
	return Id;
}

std::size_t EntityIdPool::GetSize() const {
	return Entities.size();
}

void EntityIdPool::Resize(std::size_t InAmount) {
	Entities.resize(InAmount);
}
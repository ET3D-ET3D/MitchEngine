// 2018 Mitchell Andrews
#pragma once
#include "ClassTypeId.h"
#include "EntityID.h"
#include "Dementia.h"

#include <nlohmann/json.hpp>

#define ME_REGISTER_COMPONENT(TYPE)                      \
	namespace details {                                  \
    namespace                                            \
    {                                                    \
        template<class T>                                \
        class ComponentRegistration;                     \
                                                         \
        template<>                                       \
        class ComponentRegistration<TYPE>                \
        {                                                \
            static const RegistryEntry<TYPE>& reg;       \
        };                                               \
                                                         \
        const RegistryEntry<TYPE>&                       \
            ComponentRegistration<TYPE>::reg =           \
                RegistryEntry<TYPE>::Instance(#TYPE);    \
    }}

// for convenience
using json = nlohmann::json;

class BaseComponent
{
	friend class ComponentStorage;
public:
	BaseComponent() = delete;
	BaseComponent(const char* CompName)
		: TypeName(CompName)
	{
		TypeName = TypeName.substr(TypeName.find(' ') + 1);
	}

	virtual ~BaseComponent() = default;

	virtual void Init() = 0;

	const std::string& GetName() const
	{
		return TypeName;
	}

	EntityID Parent;

#if ME_EDITOR
	virtual void OnEditorInspect() = 0;
	virtual void Serialize(json& outJson) = 0;
	virtual void Deserialize(const json& inJson) = 0;
#endif

private:
	std::string TypeName;
};

template<typename T>
class Component
	: public BaseComponent
{
public:
	Component()
		: BaseComponent(typeid(T).name())
	{
	}

	static TypeId GetTypeId()
	{
		return ClassTypeId<BaseComponent>::GetTypeId<T>();
	}

	// Each core must update each loop
	virtual void Update(float dt)
	{
	}

#if ME_EDITOR
	virtual void OnEditorInspect() override
	{
	}

	virtual void Serialize(json& outJson) override
	{
		outJson["Type"] = GetName();
	}

	virtual void Deserialize(const json& inJson) override
	{
	}

#endif
};

using ComponentArray = std::vector<std::reference_wrapper<BaseComponent>>;

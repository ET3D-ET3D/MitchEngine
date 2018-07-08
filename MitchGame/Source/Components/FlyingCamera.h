#pragma once
#include "Engine/Component.h"
#include <string>
#include <glm.hpp>

class FlyingCamera
	: public Component<FlyingCamera>
{
public:

	FlyingCamera();
	~FlyingCamera();

	// Separate init from construction code.
	virtual void Init() final;

	float FlyingSpeed = 3.5f;

	float LookSensitivity = .15f;
};
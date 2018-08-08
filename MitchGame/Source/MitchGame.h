#pragma once
#include "Game.h"
#include <glm.hpp>

class MitchGame
	: public Game
{
public:
	MitchGame();
	virtual ~MitchGame() override;

	virtual void Initialize() override;

	virtual void Update(float DeltaTime) override;

	virtual void End() override;

	Entity MainCamera;
	Entity SecondaryCamera;
	std::vector<Entity> Cubes;
	bool AddedPhysics = false;

	glm::vec2 PrevMouseScroll;
};

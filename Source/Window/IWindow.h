#pragma once
#include <string>
#include <Math/Vector2.h>

class IWindow
{
public:
	virtual bool ShouldClose() = 0;
	virtual void ParseMessageQueue() = 0;
	virtual void Swap() = 0;
	virtual Vector2 GetSize() const = 0;
	virtual Vector2 GetPosition() = 0;

	virtual bool IsFullscreen() = 0;

	virtual void Maximize() = 0;
	virtual void Minimize() = 0;
	virtual void ExitMaximize() = 0;
	virtual void SetTitle(const std::string& title) = 0;
	virtual void Exit() = 0;
};

#pragma once
#include <glm.hpp>

namespace Moonlight
{
	struct IDeviceNotify
	{
		virtual void OnDeviceLost() = 0;
		virtual void OnDeviceRestored() = 0;
	};

	class IDevice
	{
	public:
		IDevice() = default;
		~IDevice() = default;

		virtual void CreateDeviceIndependentResources() = 0;

		virtual void CreateDeviceResources() = 0;

		virtual void CreateWindowSizeDependentResources() = 0;

		virtual void Trim() = 0;

		virtual void SetLogicalSize(glm::vec2 vec2) = 0;

		virtual void WindowSizeChanged(const glm::vec2& NewSize) = 0;
	};
}
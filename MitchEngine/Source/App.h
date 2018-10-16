#pragma once

#include "Graphics/Common/DeviceResources.h"
#include "TestDirectX.h"
#include "Game.h"

// Main entry point for our app. Connects the app with the Windows shell and handles application lifecycle events.
ref class App : public Windows::ApplicationModel::Core::IFrameworkView
{
internal:
	App(std::unique_ptr<Game> ayy) : m_main(std::move(ayy)) {

	}
	App();
public:
	// IFrameworkView Methods.
	virtual void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView);
	virtual void SetWindow(Windows::UI::Core::CoreWindow^ window);
	virtual void Load(Platform::String^ entryPoint);
	virtual void Run();
	virtual void Uninitialize();

protected:
	// Application lifecycle event handlers.
	void OnActivated(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, Windows::ApplicationModel::Activation::IActivatedEventArgs^ args);
	void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ args);
	void OnResuming(Platform::Object^ sender, Platform::Object^ args);

	// Window event handlers.
	void OnWindowSizeChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ args);
	void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);
	void OnWindowClosed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CoreWindowEventArgs^ args);

	// DisplayInformation event handlers.
	void OnDpiChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
	void OnOrientationChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
	void OnDisplayContentsInvalidated(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);

private:
	std::shared_ptr<DX::DeviceResources> m_deviceResources;
	std::unique_ptr<Game> m_main;
	std::unique_ptr<TestDirectX> m_test;
	bool m_windowClosed;
	bool m_windowVisible;
};

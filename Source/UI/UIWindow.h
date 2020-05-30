#pragma once
#include <Ultralight/platform/FontLoader.h>
#include <Ultralight/String16.h>
#include "AppCore/Window.h"
#include "Window/IWindow.h"
#include "UI/RefCountedImpl.h"

class UIWindow
	: public ultralight::Window
	, public ultralight::RefCountedImpl<UIWindow>
{
	friend class OverlayImpl;
	friend class Window;
	friend class OverlayImpl;
public:
	UIWindow(IWindow* window, ultralight::OverlayManager* manager);
	virtual ~UIWindow() = default;


	virtual void set_listener(ultralight::WindowListener* listener) override;


	virtual ultralight::WindowListener* listener() override;


	virtual uint32_t width() const override;


	virtual uint32_t height() const override;


	virtual bool is_fullscreen() const override;


	virtual double scale() const override;


	virtual void SetTitle(const char* title) override;


	virtual void SetCursor(ultralight::Cursor cursor) override;


	virtual void Close() override;


	virtual int DeviceToPixels(int val) const override;

	void set_app_listener(ultralight::WindowListener* listener) {  }
	HWND hwnd();

	virtual int PixelsToDevice(int val) const override;
	REF_COUNTED_IMPL(UIWindow);

	virtual ultralight::OverlayManager* overlay_manager() const override;
protected:
	ultralight::OverlayManager* m_overlayManager = nullptr;
	IWindow* m_window;
};
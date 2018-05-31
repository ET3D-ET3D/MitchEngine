#include "Engine/Window.h"
#include "Engine/Input.h"
#include "Utility/Logger.h"
#include <assert.h>

using namespace MAN;

int Window::WINDOW_WIDTH = 960;
int Window::WINDOW_HEIGHT = 540;

Window::Window(std::string title, int width, int height)
{
	WINDOW_HEIGHT = height;
	WINDOW_WIDTH = width;

	// Init GLFW
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr); // Windowed
	glfwMakeContextCurrent(window);
	if (window == nullptr)
	{
		Logger::Get().Log(Logger::LogType::Error, "Failed to create window.");
		assert(0);
	}

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetKeyCallback(window, &Input::KeyCallback);
	glfwSetCursorPosCallback(window, &Input::MouseCallback);
}

Window::~Window()
{
}

bool Window::ShouldClose()
{
	return (glfwWindowShouldClose(window) == 1) ? true : false;
}
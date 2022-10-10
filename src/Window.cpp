#include <iostream>

#include "Window.h"

// NOTE(__LUNA__): Static members must be prefixed with class scope operator
void Window::resizeCallback(GLFWwindow *window, int width, int height)
{
	(void) width;
	(void) height;
	auto win = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
	win->m_resized = true;
}

Window::Window()
	: m_resized(false) {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_window = glfwCreateWindow(WIDTH, HEIGHT, "Wulkan", NULL, NULL);
		glfwSetWindowUserPointer(m_window, this);
		glfwSetFramebufferSizeCallback(m_window, resizeCallback);
	}

Window::~Window() {
	glfwDestroyWindow(m_window);
}

GLFWwindow *Window::getWindow() const
{
	return m_window;
}

bool Window::close() const
{
	return glfwWindowShouldClose(m_window);
}

void Window::setTitle(std::string &title)
{
	glfwSetWindowTitle(m_window, title.c_str());
}

// NOTE(__LUNA__): Test resized... fixed after bug
bool Window::resized() const
{
	return m_resized;
}

void Window::set_resized(bool resized)
{
	m_resized = resized;
}

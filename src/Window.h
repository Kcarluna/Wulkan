#pragma once
#include <GLFW/glfw3.h>

#define WIDTH 1024
#define HEIGHT 768

class Window {
public:
	Window();
	~Window();
	GLFWwindow *getWindow() const;
	bool close() const;
	void setTitle(std::string &title);
	bool resized() const;
	void set_resized(bool resized);
private:
	GLFWwindow *m_window;
	bool m_resized;
private:
	static void resizeCallback(GLFWwindow *window, int width, int height);
};

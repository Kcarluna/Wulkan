// NOTE(__LUNA__): Am I doing this input thing right?
#pragma once

#include "Window.h"

class Input {
public:
	void setWindow(GLFWwindow *window);

	float getYaw() const;
	float getPitch() const;
	float getFov() const;

	bool moveUp() const;
	bool moveLeft() const;
	bool moveDown() const;
	bool moveRight() const;
private:
	GLFWwindow *m_window;

	// NOTE(__LUNA__): 0.0f would be facing to the right | -90.0 to compensate
	float m_yaw = -90.0f;
	float m_pitch = 0.0f;
	float m_fov = 45.0f;

	bool m_up = false;
	bool m_left = false;
	bool m_down = false;
	bool m_right = false;
private:
	static void mouseCallback(GLFWwindow *window, double x, double y);
	static void scrollCallback(GLFWwindow *window, double x, double y);
	static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
};

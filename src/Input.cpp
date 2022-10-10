#include <iostream>

#include "Input.h"

// NOTE(__LUNA__): IDK if apple touchpad sux or wat but wtf famly
void Input::mouseCallback(GLFWwindow *window, double x, double y)
{
	auto input = reinterpret_cast<Input *>(glfwGetWindowUserPointer(window));

	int w, h;
	glfwGetWindowSize(window, &w, &h);

	static int last_x = w / 2, last_y = h / 2;
	// NOTE(__LUNA__): Prevents large jump on first cursor focus
	static bool first_mouse = true;
	if (first_mouse) {
		last_x = x;
		last_y = y;
		first_mouse = false;
	}
	float x_offset = x - last_x;
	float y_offset = last_y - y;
	last_x = x;
	last_y = y;

	const float sensitivity = 0.1f;
	x_offset *= sensitivity; y_offset *= sensitivity;
	input->m_yaw += x_offset; input->m_pitch += y_offset;

	// NOTE(__LUNA__): Prevent lookAt flip | can be optimized with clamp?
	if (input->m_pitch > 89.0f) {
		input->m_pitch = 89.0f;
	}
	if (input->m_pitch < -89.0f) {
		input->m_pitch = -89.0f;
	}
}

void Input::scrollCallback(GLFWwindow *window, double x, double y)
{
	(void) x;
	auto input = reinterpret_cast<Input *>(glfwGetWindowUserPointer(window));

	input->m_fov -= static_cast<float>(y);
	if (input->m_fov < 1.0f) {
		input->m_fov = 1.0f;
	}
	if (input->m_fov > 45.0f) {
		input->m_fov = 45.0f;
	}
}

void Input::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	(void) scancode;
	(void) mods;
#ifdef NDEBUG
	(void) window;
#else
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
#endif // NDEBUG
	auto input = reinterpret_cast<Input *>(glfwGetWindowUserPointer(window));
	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		input->m_up = true;
	} else if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
		input->m_up = false;
	}

	if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		input->m_left = true;
	} else if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
		input->m_left = false;
	}

	if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		input->m_down = true;
	} else if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
		input->m_down = false;
	}

	if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		input->m_right = true;
	} else if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
		input->m_right = false;
	}
}

void Input::setWindow(GLFWwindow *window)
{
	m_window = window;

	glfwSetWindowUserPointer(m_window, this);
	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSetCursorPosCallback(m_window, mouseCallback);
	glfwSetScrollCallback(m_window, scrollCallback);
	glfwSetKeyCallback(m_window, keyCallback);
}

float Input::getYaw() const
{
	return m_yaw;
}

float Input::getPitch() const
{
	return m_pitch;
}

float Input::getFov() const
{
	return m_fov;
}

bool Input::moveUp() const
{
	return m_up;
}

bool Input::moveLeft() const
{
	return m_left;
}

bool Input::moveDown() const
{
	return m_down;
}

bool Input::moveRight() const
{
	return m_right;
}

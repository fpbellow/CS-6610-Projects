#include "../Headers/InputHandler.hpp"

float InputHandler::xRotation = 0;
float InputHandler::yRotation = 0;
float InputHandler::camX = 0;
float InputHandler::camY = 0;
float InputHandler::camDistance = 1.0f;


float InputHandler::quadRotX = 0;
float InputHandler::quadRotY = 0;
float InputHandler::quadCamDist = 1.15f;


bool InputHandler::cursorLock = false;
bool InputHandler::moveCam = false;
bool InputHandler::toggleGuiMenu = true;
bool InputHandler::modKey = false;
float InputHandler::mouseXStart = 0;
float InputHandler::mouseYStart = 0;



void InputHandler::Initialize(float x, float y, float camDist)
{
	xRotation = x;
	yRotation = y;

	quadRotX = x;
	quadRotY = y;

	camDistance = camDist;
	cursorLock = false;
}

void InputHandler::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && moveCam == false)
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		mouseXStart = static_cast<float>(xpos);
		mouseYStart = static_cast<float>(ypos);
		cursorLock = true;
	}

	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && cursorLock == false)
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		mouseXStart = static_cast<float>(xpos);
		mouseYStart = static_cast<float>(ypos);
		moveCam = true;
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		cursorLock = false;
	}
	
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		moveCam = false;
	}
}

void InputHandler::mouseCursorCallback(GLFWwindow* window, double xposin, double yposin)
{
	float xpos = static_cast<float>(xposin);
	float ypos = static_cast<float>(yposin);


	if (cursorLock == true && moveCam == false)
	{
		float xOffset = xpos - mouseXStart;
		float yOffset = ypos - mouseYStart;
		mouseXStart = xpos;
		mouseYStart = ypos;
		if (modKey == false)
		{
			xRotation += xOffset * 0.01f;
			yRotation += yOffset * 0.01f;
		}
		else
		{
			quadRotX += xOffset * 0.01f;
			quadRotY += yOffset * 0.01f;
		}
		
	}
	
	else if (moveCam == true && cursorLock == false)
	{
		float xOffset = xpos - mouseXStart;
		float yOffset = ypos - mouseYStart;
		mouseXStart = xpos;
		mouseYStart = ypos;
		camX -= xOffset * 0.01f;
		camY -= yOffset * 0.01f;
	}
} 

void InputHandler::mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	float yOff = static_cast<float>(yoffset);
	if (modKey == false && ((yOff < 0 && camDistance < 4.9f) || (yOff > 0 && camDistance > 0.1f)))
		camDistance -= yOff * 0.1f;

	else if (modKey == true && ((yOff < 0 && quadCamDist < 4.9f) || (yOff > 0 && quadCamDist > 1.15f)))
		quadCamDist -= yOff * 0.1f;

}

void InputHandler::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
		toggleGuiMenu = !toggleGuiMenu;

	if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS)
		modKey = true;

	if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE)
		modKey = false;
}
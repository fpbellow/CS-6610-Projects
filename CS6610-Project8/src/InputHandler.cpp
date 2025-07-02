#include "../Headers/InputHandler.hpp"

float InputHandler::xRotation = 0;
float InputHandler::yRotation = 0;
float InputHandler::camX = 0;
float InputHandler::camY = 0;
float InputHandler::camDistance = 1.0;
bool InputHandler::cursorLock = false;
bool InputHandler::moveCam = false;
bool InputHandler::toggleGuiMenu = true;
float InputHandler::mouseXStart = 0;
float InputHandler::mouseYStart = 0;
float InputHandler::tessFactor = 256.0f;
float InputHandler::displacementFactor = 0.5f;
int InputHandler::tessIter = 9;
int InputHandler::displacementIter = 5;
bool InputHandler::toggleTriangulation = false;



void InputHandler::Initialize(float x, float y, float camDist)
{
	xRotation = x;
	yRotation = y;

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
		xRotation += xOffset * 0.01f;
		yRotation += yOffset * 0.01f;
	}
	
	else if (moveCam == true && cursorLock == false)
	{
		float xOffset = xpos - mouseXStart;
		float yOffset = ypos - mouseYStart;
		mouseXStart = xpos;
		mouseYStart = ypos;
		camX -= xOffset * 0.01f;
		camY -= yOffset * 0.01f;
		float maxPitch = DirectX::XM_PIDIV2 - atan2(0.5f, InputHandler::camDistance) - 0.01f;
		camY = std::clamp(camY, -maxPitch, maxPitch);
	}
} 

void InputHandler::mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	float yOff = static_cast<float>(yoffset);
	if ((yOff < 0 && camDistance < 4.9) || (yOff > 0 && camDistance > 0.1))
		camDistance -= yOff * 0.1f;

}

void InputHandler::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_F1 && action == GLFW_PRESS)
		toggleGuiMenu = !toggleGuiMenu;

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		toggleTriangulation = !toggleTriangulation;

	float tessFactorOptions[10] = { 1.0f, 2.0f, 4.0f, 8.0f, 16.0f, 32.0f, 48.0f, 64.0f, 128.0f, 256.0f };
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS && tessIter < 9)
	{
		tessIter += 1;
		tessFactor = tessFactorOptions[tessIter];
	}

	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS && tessIter > 0)
	{
		tessIter -= 1;
		tessFactor = tessFactorOptions[tessIter];
	}

	float displaceFactorOptions[6] = { 0.0f, 0.1f , 0.2f, 0.3f, 0.4f, 0.5f };
	if (key == GLFW_KEY_UP && action == GLFW_PRESS && displacementIter < 5)
	{
		displacementIter += 1;
		displacementFactor = displaceFactorOptions[displacementIter];
	}

	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS && displacementIter > 0)
	{
		displacementIter -= 1;
		displacementFactor = displaceFactorOptions[displacementIter];
	}
}
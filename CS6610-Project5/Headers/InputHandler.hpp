#include <GLFW/glfw3.h>
#include <iostream>

class InputHandler
{
public:
	
	static bool toggleGuiMenu;

	static float xRotation;
	static float yRotation;
	static float camX;
	static float camY;
	static float camDistance;
	

	static float quadRotX;
	static float quadRotY;

	static float quadCamDist;

	static void Initialize(float x, float y, float camDist);

	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void mouseCursorCallback(GLFWwindow* window, double xpos, double ypos);
	static void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
	static bool moveCam;
	static bool cursorLock;
	static bool modKey;

	static float mouseXStart;
	static float mouseYStart;
};
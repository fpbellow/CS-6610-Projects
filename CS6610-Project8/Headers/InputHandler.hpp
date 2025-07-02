#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <DirectXMath.h>

class InputHandler
{
public:
	
	static bool toggleGuiMenu;
	static float xRotation;
	static float yRotation;
	static float camX;
	static float camY;
	static float camDistance;
	static float tessFactor;
	static float displacementFactor;
	
	static bool toggleTriangulation;

	static void Initialize(float x, float y, float camDist);

	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void mouseCursorCallback(GLFWwindow* window, double xpos, double ypos);
	static void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
	static bool moveCam;
	static bool cursorLock;
	static float mouseXStart;
	static float mouseYStart;
	static int tessIter;
	static int displacementIter;
};
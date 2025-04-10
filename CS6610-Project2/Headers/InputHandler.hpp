#include <GLFW/glfw3.h>

class InputHandler
{
public:
	static bool cursorLock;
	static bool toggleGuiMenu;
	static float xRotation;
	static float yRotation;
	static float camDistance;

	static void Initialize(float x, float y, float camDist);

	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void mouseCursorCallback(GLFWwindow* window, double xpos, double ypos);
	static void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
	static float mouseXStart;
	static float mouseYStart;
};
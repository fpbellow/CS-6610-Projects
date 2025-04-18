#include "Definitions.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_dx11.h>

#include <DirectXMath.h>

#include <filesystem>
#include <string>
#include <iostream>

struct ImGuiMenuData
{
	std::string objFile;
	UINT numVertices;
	bool showMenu;
};

class ImGuiMenu
{
public:
	static bool Initialize(GLFWwindow* window, ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	static void Update(ImGuiMenuData menuData);
	static void CleanUp();
};

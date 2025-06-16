#include "../Headers/ImGuiMenu.hpp"

bool ImGuiMenu::Initialize(GLFWwindow* window, ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& _io = ImGui::GetIO(); (void)_io;
	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowMinSize = ImVec2(200, 400);
	ImGui_ImplGlfw_InitForOther(window, true);
	ImGui_ImplDX11_Init(device, deviceContext);

	return true;
}

void ImGuiMenu::Update(ImGuiMenuData menuData)
{
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplDX11_NewFrame();
	ImGui::NewFrame();

	if (menuData.showMenu)
	{
		ImGui::Begin("CS6610 - Project 8");
		ImGui::Text("Quad");
		
		ImGui::Text("Number of Vertices: %u", 4);
		ImGui::Separator();
		ImGui::Text("\n");
		ImGui::Text("Press F1 to Open/Hide Menu.");
		ImGui::Text("Hold Right Click to rotate camera.");
		ImGui::Separator();
		ImGui::Text("\n");
		ImGui::Text("Press Space to Show/Hide triangulation.");
		ImGui::Text("Use Left/Right arrow keys to increase/decrease tessellation factor.");
		ImGui::Separator();
		ImGui::Text("\n");
		ImGui::Text("CamPos X: %f", menuData.camPos[0]);
		ImGui::Text("CamPos Y: %f", menuData.camPos[1]);
		ImGui::Text("CamPos Z: %f", menuData.camPos[2]);
	

		
		ImGui::End();
	}
	
}

void ImGuiMenu::CleanUp()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
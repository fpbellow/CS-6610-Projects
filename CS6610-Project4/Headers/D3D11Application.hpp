#pragma once

#include "Definitions.hpp"
#include "Application.hpp"
#include "ShaderCollection.hpp"
#include "ResourceLoader.hpp"
#include "InputHandler.hpp"
#include "ImGuiMenu.hpp"

#include <d3d11_2.h>
#include <memory>
#include <DirectXMath.h>



//DirectX TK
#include <VertexTypes.h>
#include <Model.h>

class D3D11Application final : public Application
{

public:
	D3D11Application(const std::string& title);
	~D3D11Application() override;

protected:
	bool Initialize() override;
	bool Load() override;
	void CleanUp() override;
	void Update() override;
	void Render() override;

private:
	void CreateRasterState();
	void CreateDepthStencilView();
	void CreateDepthState();
	void CreateConstantBuffers();
	bool CreateSwapchainResources();
	void DestroySwapchainResources();

	WRL::ComPtr<ID3D11Device> _device = nullptr;
	WRL::ComPtr<ID3D11DeviceContext> _deviceContext = nullptr;
	WRL::ComPtr<IDXGIFactory2> _dxgiFactory = nullptr;
	WRL::ComPtr<IDXGISwapChain1> _swapChain = nullptr;
	WRL::ComPtr<ID3D11Buffer> _vertexBuffer = nullptr;
	WRL::ComPtr<ID3D11RenderTargetView> _renderTarget = nullptr;
	WRL::ComPtr<ID3D11RasterizerState> _rasterState = nullptr;
	WRL::ComPtr<ID3D11DepthStencilView> _depthTarget = nullptr;
	WRL::ComPtr<ID3D11DepthStencilState> _depthState = nullptr;
	WRL::ComPtr<ID3D11Debug> _debug = nullptr;

	WRL::ComPtr<ID3D11Buffer> _perFrameConstantBuffer = nullptr;
	WRL::ComPtr<ID3D11Buffer> _perObjectConstantBuffer = nullptr;

	WRL::ComPtr<ID3D11SamplerState> _linearSamplerState = nullptr;
	WRL::ComPtr<ID3D11ShaderResourceView> _fallbackTextureSrv = nullptr;

	PerFrameConstantBuffer _perFrameConstantBufferData{};
	PerObjectConstantBuffer _perObjectConstantBufferData{};


	ShaderCollection _shaderCollection;

	std::vector<Material> _materials;

	UINT _drawCount = 3;

	ImGuiMenuData _menuData;

	
	
};

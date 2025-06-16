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
	void CreateRenderTextureResources();
	bool CreateSwapchainResources();
	void DestroySwapchainResources();


	WRL::ComPtr<ID3D11Device> _device = nullptr;
	WRL::ComPtr<ID3D11DeviceContext> _deviceContext = nullptr;
	WRL::ComPtr<IDXGIFactory2> _dxgiFactory = nullptr;
	WRL::ComPtr<IDXGISwapChain1> _swapChain = nullptr;
	WRL::ComPtr<ID3D11RenderTargetView> _renderTarget = nullptr;
	WRL::ComPtr<ID3D11RasterizerState> _rasterState = nullptr;
	WRL::ComPtr<ID3D11DepthStencilView> _depthTarget = nullptr;
	WRL::ComPtr<ID3D11DepthStencilState> _depthState = nullptr;
	WRL::ComPtr<ID3D11Debug> _debug = nullptr;


	WRL::ComPtr<ID3D11Buffer> _quadVertexBuffer = nullptr;
	WRL::ComPtr<ID3D11Buffer> _quadIndexBuffer = nullptr;

	WRL::ComPtr<ID3D11Buffer> _perFrameConstantBuffer = nullptr;
	WRL::ComPtr<ID3D11Buffer> _quadObjConstantBuffer = nullptr;
	WRL::ComPtr<ID3D11Buffer> _lightConstantBuffer = nullptr;
	WRL::ComPtr<ID3D11Buffer> _tessellationConstantBuffer = nullptr;

	WRL::ComPtr<ID3D11SamplerState> _comparisonSampleState = nullptr;
	WRL::ComPtr<ID3D11SamplerState> _linearSamplerState = nullptr;

	WRL::ComPtr<ID3D11ShaderResourceView> _normalMap = nullptr; 
	WRL::ComPtr<ID3D11ShaderResourceView> _displacementMap = nullptr;

	WRL::ComPtr<ID3D11Texture2D> _shadowMap = nullptr;
	WRL::ComPtr<ID3D11DepthStencilView> _shadowDSV = nullptr;
	WRL::ComPtr<ID3D11ShaderResourceView> _shadowSRV = nullptr;
	WRL::ComPtr<ID3D11RasterizerState> _shadowRasterState = nullptr;

	

	PerFrameConstantBuffer _perFrameConstantBufferData{};
	PerObjectConstantBuffer _quadObjConstantBufferData{};
	LightConstantBuffer _lightConstantBufferData{};
	TessellationConstantBuffer _tessellationConstantBufferData{};



	ShaderCollection _shaderCollection;
	ShaderCollection _shadowShaderCollection;
	ShaderCollection _triangulateShaderCollection;


	std::vector<Material> _materials;

	UINT _drawCount = 3;
	UINT _shadowMapDimension = 4096;

	ImGuiMenuData _menuData;

	
	
};

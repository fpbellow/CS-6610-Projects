#pragma once

#include "Definitions.hpp"
#include "Application.hpp"
#include "ShaderCollection.hpp"

#include <d3d11_2.h>
#include <memory>
#include <DirectXMath.h>

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
	bool CreateSwapchainResources();
	void DestroySwapchainResources();

	WRL::ComPtr<ID3D11Device> _device = nullptr;
	WRL::ComPtr<ID3D11DeviceContext> _deviceContext = nullptr;
	WRL::ComPtr<IDXGIFactory2> _dxgiFactory = nullptr;
	WRL::ComPtr<IDXGISwapChain1> _swapChain = nullptr;
	WRL::ComPtr<ID3D11RenderTargetView> _renderTarget = nullptr;
};

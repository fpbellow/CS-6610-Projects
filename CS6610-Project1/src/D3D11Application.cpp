#include "../Headers/D3D11Application.hpp"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <DirectXMath.h>
#include <d3dcompiler.h>

#include <iostream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dxguid.lib")

D3D11Application::D3D11Application(const std::string& title) : Application(title)
{
}

D3D11Application::~D3D11Application()
{
}

bool D3D11Application::Initialize()
{
	if (!Application::Initialize())
	{
		return false;
	}

	//initialize DX device and swapchain
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory))))
	{
		std::cerr << "DXGI: Failed to create DXGI Factory. \n";
		return false;
	}

	constexpr D3D_FEATURE_LEVEL deviceFeatureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0;

	if (FAILED(D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		&deviceFeatureLevel,
		1,
		D3D11_SDK_VERSION,
		&_device,
		nullptr,
		&_deviceContext)))
	{
		std::cerr << "D3D11: Failed to create device and device context. \n";
		return false;
	}


	DXGI_SWAP_CHAIN_DESC1 swapChainDescriptor = {};
	swapChainDescriptor.Width = GetWindowWidth();
	swapChainDescriptor.Height = GetWindowHeight();
	swapChainDescriptor.Format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDescriptor.SampleDesc.Count = 1;
	swapChainDescriptor.SampleDesc.Quality = 0;
	swapChainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDescriptor.BufferCount = 2;
	swapChainDescriptor.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDescriptor.Scaling = DXGI_SCALING::DXGI_SCALING_STRETCH;
	swapChainDescriptor.Flags = {};

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullscreenDescriptor = {};
	swapChainFullscreenDescriptor.Windowed = true;

	if (FAILED(_dxgiFactory->CreateSwapChainForHwnd(
		_device.Get(),
		glfwGetWin32Window(GetWindow()),
		&swapChainDescriptor,
		&swapChainFullscreenDescriptor,
		nullptr,
		&_swapChain
	)))
	{
		std::cerr << "DXGI: Failed to create swapchain. \n";
		return false;
	}

	if (!CreateSwapchainResources())
	{
		return false;
	}

	return true;
}


bool D3D11Application::CreateSwapchainResources()
{
	WRL::ComPtr<ID3D11Texture2D> backBuffer = nullptr;
	if (FAILED(_swapChain->GetBuffer( 0, IID_PPV_ARGS(&backBuffer) )))
	{
		std::cerr << "D3D11: Failed to get back buffer from the swap chain. \n";
		return false;
	}


	if (FAILED(_device->CreateRenderTargetView( backBuffer.Get(), nullptr, &_renderTarget )))
	{
		std::cerr << "D3D111: Failed to create render target view from back buffer.\n";
		return false;
	}

	return true;
}

void D3D11Application::DestroySwapchainResources()
{
	_renderTarget.Reset();
}


bool D3D11Application::Load()
{
	return true;
}

void D3D11Application::CleanUp()
{
}

void D3D11Application::Update()
{
	Application::Update();
}


void D3D11Application::Render()
{
	D3D11_VIEWPORT viewport = {
		0.0f,
		0.0f,
		static_cast<float>(GetWindowWidth()),
		static_cast<float>(GetWindowHeight()),
		0.0f,
		1.0f
	};

	constexpr float clearColor[] = { 0.1f, 0.1f, 0.3f, 1.0f };

	_deviceContext->ClearRenderTargetView( _renderTarget.Get(), clearColor);

	_deviceContext->RSSetViewports( 1, &viewport );

	_deviceContext->OMSetRenderTargets(1, _renderTarget.GetAddressOf(), nullptr);

	_swapChain->Present(1, 0);
}
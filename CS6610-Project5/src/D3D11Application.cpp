#include "../Headers/D3D11Application.hpp"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <DirectXMath.h>


#include <iostream>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dxguid.lib")


using VertexType = DirectX::VertexPositionNormalTexture;

D3D11Application::D3D11Application(const std::string& title) : Application(title)
{
}

D3D11Application::~D3D11Application()
{
	_deviceContext->Flush();
	_vertexBuffer.Reset();
	_renTexVertexBuffer.Reset();
	_perFrameConstantBuffer.Reset();
	_perObjectConstantBuffer.Reset();
	_shaderCollection.Destroy();
	_renderTexShaderCollection.Destroy();
	DestroySwapchainResources();
	_swapChain.Reset();
	_dxgiFactory.Reset();
	_deviceContext.Reset();
	_device.Reset();
}

bool D3D11Application::Initialize()
{
	if (!Application::Initialize())
	{
		return false;
	}

	InputHandler::Initialize(0.0f, 0.0f, 1.5f);

	glfwSetMouseButtonCallback(_window, InputHandler::mouseButtonCallback);
	glfwSetCursorPosCallback(_window, InputHandler::mouseCursorCallback);
	glfwSetScrollCallback(_window, InputHandler::mouseScrollCallback);
	glfwSetKeyCallback(_window, InputHandler::keyCallback);


	//initialize DX device and swapchain
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory))))
	{
		std::cerr << "DXGI: Failed to create DXGI Factory. \n";
		return false;
	}

	constexpr D3D_FEATURE_LEVEL deviceFeatureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0;
	uint32_t deviceFlags = 0;

	WRL::ComPtr<ID3D11DeviceContext> deviceContext;
	if (FAILED(D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		deviceFlags,
		&deviceFeatureLevel,
		1,
		D3D11_SDK_VERSION,
		&_device,
		nullptr,
		&deviceContext)))
	{
		std::cerr << "D3D11: Failed to create device and device context. \n";
		return false;
	}

	_deviceContext = deviceContext;


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
		glfwGetWin32Window(_window),
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
	CreateRasterState();
	CreateDepthStencilView();
	CreateDepthState();
	CreateConstantBuffers();

	if (!ImGuiMenu::Initialize(_window, _device.Get(), _deviceContext.Get()))
		std::cout << "ImGui: Failed to initialize ImGui. \n";

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

void D3D11Application::CreateRasterState()
{
	D3D11_RASTERIZER_DESC rasterDesc{};
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.FillMode = D3D11_FILL_SOLID;

	_device->CreateRasterizerState(&rasterDesc, &_rasterState);
}

void D3D11Application::CreateDepthStencilView()
{
	D3D11_TEXTURE2D_DESC texDesc{};
	texDesc.Height = GetWindowHeight();
	texDesc.Width = GetWindowWidth();
	texDesc.ArraySize = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.MipLevels = 1;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texDesc.Format = DXGI_FORMAT_R32_TYPELESS;

	ID3D11Texture2D* texture = nullptr;
	if (FAILED(_device->CreateTexture2D(&texDesc, nullptr, &texture)))
	{
		std::cerr << "D3D11: Failed to create texture for DepthStencilView. \n";
		return;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	if (FAILED(_device->CreateDepthStencilView(texture, &dsvDesc, &_depthTarget)))
	{
		std::cerr << "D3D11: Failed to create DepthStencilView. \n";
		texture->Release();
		return;
	}

	texture->Release();
}

void D3D11Application::CreateDepthState()
{
	D3D11_DEPTH_STENCIL_DESC depthDesc{};
	depthDesc.DepthEnable = TRUE;
	depthDesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS;
	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;

	_device->CreateDepthStencilState(&depthDesc, &_depthState);
}

void D3D11Application::CreateConstantBuffers()
{
	D3D11_BUFFER_DESC desc{};
	desc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
	desc.ByteWidth = sizeof(PerFrameConstantBuffer);
	desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;

	_device->CreateBuffer(&desc, nullptr, &_perFrameConstantBuffer);

	desc.ByteWidth = sizeof(PerObjectConstantBuffer);
	_device->CreateBuffer(&desc, nullptr, &_perObjectConstantBuffer);
}

void D3D11Application::CreateRenderTextureResources()
{
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = static_cast<UINT>(GetWindowWidth());
	textureDesc.Height = static_cast<UINT>(GetWindowHeight());
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	_device->CreateTexture2D(&textureDesc, nullptr, &_renderTexture);
	_device->CreateRenderTargetView(_renderTexture.Get(), nullptr, &_renderTextureView);
	_device->CreateShaderResourceView(_renderTexture.Get(), nullptr, &_textureResourceView);

	ShaderCollectionDescriptor shaderDescriptor = {};
	shaderDescriptor.VertexShaderFilePath = L"../Assets/Shaders/renderTex.vs.hlsl";
	shaderDescriptor.PixelShaderFilePath = L"../Assets/Shaders/renderTex.ps.hlsl";
	shaderDescriptor.InputElems = VertexType::InputElements;
	shaderDescriptor.NumElems = VertexType::InputElementCount;


	_renderTexShaderCollection = ShaderCollection::CreateShaderCollection(shaderDescriptor, _device.Get());

	_menuData.objFile = "../Assets/Models/teapot/teapot.obj";
	std::vector<VertexType> vertices;
	std::vector<Material> materials;
	if (!LoadOBJ(_menuData.objFile, vertices, materials, _device.Get()))
	{
		std::cerr << "ModelLoad: Failed to load obj file. \n";
	}

	_drawCount = static_cast<UINT>(vertices.size());
	_menuData.numVertices = _drawCount;

	D3D11_BUFFER_DESC bufferInfo = {};
	bufferInfo.ByteWidth = static_cast<UINT>(sizeof(VertexType) * vertices.size());
	bufferInfo.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
	bufferInfo.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA resourceData = {};
	resourceData.pSysMem = vertices.data();

	if (FAILED(_device->CreateBuffer(&bufferInfo, &resourceData, &_renTexVertexBuffer)))
	{
		std::cerr << "D3D11: Failed to create vertex buffer for the render texture. \n";
		return;
	}

	_materials = materials;

	_fallbackTextureSrv = CreateTextureView(_device.Get(), "../Assets/Textures/default.png");
	assert(_fallbackTextureSrv != nullptr);

	if (_materials[0].diffuseTexture == nullptr)
		_materials[0].diffuseTexture = _fallbackTextureSrv;

	if (_materials[0].specularTexture == nullptr)
		_materials[0].specularTexture = _fallbackTextureSrv;

	_perFrameConstantBufferData.lightColor = DirectX::XMFLOAT4(materials[0].specular.x, materials[0].specular.y, materials[0].specular.z, 1.0f);

	_perObjectConstantBufferData.materialColor = materials[0].diffuse;
	_perObjectConstantBufferData.shininess = materials[0].shininess;

	D3D11_SAMPLER_DESC linearSamplerStateDescriptor = {};
	linearSamplerStateDescriptor.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	linearSamplerStateDescriptor.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
	linearSamplerStateDescriptor.AddressV = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
	linearSamplerStateDescriptor.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
	if (FAILED(_device->CreateSamplerState(&linearSamplerStateDescriptor, &_linearSamplerState)))
	{
		std::cerr << "D3D11: Failed to create linear sampler state. \n";
		return;
	}
}


bool D3D11Application::Load()
{
	using namespace DirectX;
	CreateRenderTextureResources();
	ShaderCollectionDescriptor shaderDescriptor = {};
	shaderDescriptor.VertexShaderFilePath = L"../Assets/Shaders/main.vs.hlsl";
	shaderDescriptor.PixelShaderFilePath = L"../Assets/Shaders/main.ps.hlsl";
	shaderDescriptor.InputElems = VertexPositionTexture::InputElements;
	shaderDescriptor.NumElems = VertexPositionTexture::InputElementCount;

	_shaderCollection = ShaderCollection::CreateShaderCollection(shaderDescriptor, _device.Get());

	std::vector<VertexPositionTexture> quad_vertices = {
		{ XMFLOAT3(-0.5f,  0.5f, 0.0f), XMFLOAT2( 0.0, 0.0 )},
		{ XMFLOAT3( 0.5f,  0.5f, 0.0f), XMFLOAT2( 1.0, 0.0 )},
		{ XMFLOAT3(-0.5f, -0.5f, 0.0f), XMFLOAT2( 0.0, 1.0 )},
		{ XMFLOAT3( 0.5f, -0.5f, 0.0f), XMFLOAT2( 1.0, 1.0 )}
	};

	D3D11_BUFFER_DESC bufferInfo = {};
	bufferInfo.ByteWidth = static_cast<UINT>(sizeof(VertexPositionTexture) * quad_vertices.size());
	bufferInfo.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
	bufferInfo.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA resourceData = {};
	resourceData.pSysMem = quad_vertices.data();

	if (FAILED(_device->CreateBuffer(&bufferInfo, &resourceData, &_vertexBuffer)))
	{
		std::cerr << "D3D11: Failed to create vertex buffer. \n";
		return false;
	}


	return true;
}

void D3D11Application::CleanUp()
{
	Application::CleanUp();

}

void D3D11Application::Update()
{
	Application::Update();


	using namespace DirectX;

	static float _scale = 1.0f;

	XMFLOAT3 _cameraPosition = { 0.0, 0.0, InputHandler::camDistance };

	//camera configuration + view and proj matrices
	XMVECTOR camPos = XMLoadFloat3(&_cameraPosition);
	XMMATRIX revolveY = XMMatrixRotationY(InputHandler::camX);
	XMMATRIX revolveX = XMMatrixRotationX(InputHandler::camY);

	camPos = XMVector3Transform(camPos, revolveY);
	camPos = XMVector3Transform(camPos, revolveX);

	XMMATRIX view = XMMatrixLookAtRH(camPos, g_XMZero, { 0,1,0,0 });

	XMMATRIX proj = XMMatrixPerspectiveFovRH( 90.0f * 0.0174533f, static_cast<float>(_width) / static_cast<float>(_height), 0.1f, 100.0f);

	XMMATRIX viewProjection = XMMatrixMultiply(view, proj);
	XMStoreFloat4x4(&_perFrameConstantBufferData.viewProjectionMatrix, viewProjection);

	XMFLOAT3 lightPosition = XMFLOAT3(1.0, 2.0, 4.0);
	XMFLOAT3 lightColor = XMFLOAT3(1.0, 1.0, 1.0);

	_perFrameConstantBufferData.viewPos = XMFLOAT4(_cameraPosition.x, _cameraPosition.y, _cameraPosition.z, 1.0);
	_perFrameConstantBufferData.lightPos = XMFLOAT4(lightPosition.x, lightPosition.y, lightPosition.z, 0.0);

	//3d object transformations
	XMMATRIX originTranslation = XMMatrixTranslation(0.0f, 0.0f, -0.3f);


	XMMATRIX translation = XMMatrixTranslation(0, 0, 0.0);
	XMMATRIX scaling = XMMatrixScaling(_scale, _scale, _scale);
	XMMATRIX rotation = XMMatrixRotationRollPitchYaw(InputHandler::yRotation, InputHandler::xRotation, 0);

	//model matrix
	XMMATRIX modelMatrix = originTranslation * scaling * rotation * translation;
	XMStoreFloat4x4(&_perObjectConstantBufferData.modelMatrix, modelMatrix); 

	XMMATRIX invTranspose = XMMatrixTranspose(XMMatrixInverse(nullptr, modelMatrix));
	XMStoreFloat4x4(&_perObjectConstantBufferData.invTranspose, invTranspose);

	XMFLOAT3 materialColor = XMFLOAT3(1.0, 0.0, 0.0);
	float shineVal = 32;

	

	_menuData.showMenu = InputHandler::toggleGuiMenu;
	ImGuiMenu::Update(_menuData);

	//update constant buffers
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	_deviceContext->Map(_perFrameConstantBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &_perFrameConstantBufferData, sizeof(PerFrameConstantBuffer));
	_deviceContext->Unmap(_perFrameConstantBuffer.Get(), 0);

	_deviceContext->Map(_perObjectConstantBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &_perObjectConstantBufferData, sizeof(PerObjectConstantBuffer));
	_deviceContext->Unmap(_perObjectConstantBuffer.Get(), 0);
}


void D3D11Application::Render()
{
	D3D11_VIEWPORT viewport = {
		0.0f, //TopLeftX
		0.0f, //TopLeftY
		static_cast<float>(GetWindowWidth()),
		static_cast<float>(GetWindowHeight()),
		0.0f, //MinDepth
		1.0f //MaxDepth
	};

	constexpr float texBackColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	UINT stride = sizeof(VertexType);
	constexpr UINT renTexVertexOffset = 0;
	constexpr UINT vertexOffset = 0;
	ID3D11RenderTargetView* nullTarget = nullptr;

	ImGui::Render();

	_deviceContext->OMSetRenderTargets(1, &nullTarget, nullptr);
	_deviceContext->ClearRenderTargetView(_renderTextureView.Get(), texBackColor);
	_deviceContext->ClearDepthStencilView(_depthTarget.Get(), D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH, 1.0f, 0);

	_deviceContext->OMSetRenderTargets(1, _renderTextureView.GetAddressOf(), _depthTarget.Get());

	_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	_deviceContext->IASetVertexBuffers( 0, 1, _renTexVertexBuffer.GetAddressOf(), &stride, &renTexVertexOffset);


	_renderTexShaderCollection.ApplyToContext(_deviceContext.Get());


	_deviceContext->RSSetViewports( 1, &viewport );
	_deviceContext->RSSetState(_rasterState.Get());
	_deviceContext->OMSetDepthStencilState(_depthState.Get(), 0);

	_deviceContext->PSSetShaderResources(0, 1, _materials[0].diffuseTexture.GetAddressOf());
	_deviceContext->PSSetShaderResources(1, 1, _materials[0].specularTexture.GetAddressOf());
	_deviceContext->PSSetSamplers(0, 1, _linearSamplerState.GetAddressOf());

	ID3D11Buffer* constantBuffers[2] =
	{
		_perFrameConstantBuffer.Get(),
		_perObjectConstantBuffer.Get()
	};

	_deviceContext->VSSetConstantBuffers(0, 2, constantBuffers);
	_deviceContext->PSSetConstantBuffers(1, 1, &constantBuffers[1]);
	_deviceContext->Draw(_drawCount, 0);


	constexpr float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	_deviceContext->OMSetRenderTargets(1, &nullTarget, nullptr);
	_deviceContext->ClearRenderTargetView(_renderTarget.Get(), clearColor);
	_deviceContext->ClearDepthStencilView(_depthTarget.Get(), D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH, 1.0f, 0);

	_deviceContext->OMSetRenderTargets(1, _renderTarget.GetAddressOf(), _depthTarget.Get());
	_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	stride = sizeof(DirectX::VertexPositionTexture);
	_deviceContext->IASetVertexBuffers(0, 1, _vertexBuffer.GetAddressOf(), &stride, &vertexOffset);

	_shaderCollection.ApplyToContext(_deviceContext.Get());
	_deviceContext->PSSetShaderResources(0, 1, _textureResourceView.GetAddressOf());
	_deviceContext->PSSetSamplers(0, 1, _linearSamplerState.GetAddressOf());

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	_deviceContext->Draw(4, 0);
	_swapChain->Present(1, 0);
}
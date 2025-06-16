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
	_perFrameConstantBuffer.Reset();
	_quadObjConstantBuffer.Reset();
	_lightConstantBuffer.Reset();
	_tessellationConstantBuffer.Reset();
	_rasterState.Reset();
	_renderTarget.Reset();
	_depthTarget.Reset();
	_depthState.Reset();
	_quadVertexBuffer.Reset();
	_quadIndexBuffer.Reset();
	_comparisonSampleState.Reset();
	_linearSamplerState.Reset();
	_shadowMap.Reset();
	_shadowDSV.Reset();
	_shadowSRV.Reset();
	_shadowRasterState.Reset();
	_shaderCollection.Destroy();
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

	InputHandler::Initialize(0.0f, 0.0, 1.5f);

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
	CreateRenderTextureResources();

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
	rasterDesc.DepthClipEnable = true;

	_device->CreateRasterizerState(&rasterDesc, &_rasterState);

	rasterDesc.DepthBias = 150;            
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.SlopeScaledDepthBias = 16.0f;
	_device->CreateRasterizerState(&rasterDesc, &_shadowRasterState);
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
	_device->CreateBuffer(&desc, nullptr, &_quadObjConstantBuffer);

	desc.ByteWidth = sizeof(LightConstantBuffer);
	_device->CreateBuffer(&desc, nullptr, &_lightConstantBuffer);

	desc.ByteWidth = sizeof(TessellationConstantBuffer);
	_device->CreateBuffer(&desc, nullptr, &_tessellationConstantBuffer);
}

void D3D11Application::CreateRenderTextureResources()
{
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = _shadowMapDimension;
	textureDesc.Height = _shadowMapDimension;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R24G8_TYPELESS;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;

	if (FAILED(_device->CreateTexture2D(&textureDesc, nullptr, &_shadowMap)))
	{
		std::cerr << "D3D11: Failed to create shadow map texture. \n";
	}


	D3D11_DEPTH_STENCIL_VIEW_DESC shadowMapDepthDesc = {};
	shadowMapDepthDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
	shadowMapDepthDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowMapDepthDesc.Texture2D.MipSlice = 0;

	if (FAILED(_device->CreateDepthStencilView(_shadowMap.Get(), &shadowMapDepthDesc, &_shadowDSV)))
	{
		std::cerr << "D3D11: Failed to create shadow depth stencil view. \n";
	}


	D3D11_SHADER_RESOURCE_VIEW_DESC shadowMapResourceViewDesc = {};
	shadowMapResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shadowMapResourceViewDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shadowMapResourceViewDesc.Texture2D.MipLevels = 1;

	if (FAILED(_device->CreateShaderResourceView(_shadowMap.Get(), &shadowMapResourceViewDesc, &_shadowSRV)))
	{
		std::cerr << "D3D11: Failed to create shadow shader resource view. \n";
	}


	D3D11_SAMPLER_DESC comparisonSamplerDesc;
	comparisonSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	comparisonSamplerDesc.BorderColor[0] = 1.0f;
	comparisonSamplerDesc.BorderColor[1] = 1.0f;
	comparisonSamplerDesc.BorderColor[2] = 1.0f;
	comparisonSamplerDesc.BorderColor[3] = 1.0f;
	comparisonSamplerDesc.MinLOD = 0.0f;
	comparisonSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	comparisonSamplerDesc.MipLODBias = 0.0f;
	comparisonSamplerDesc.MaxAnisotropy = 0;
	comparisonSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	comparisonSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;

	if (FAILED(_device->CreateSamplerState(&comparisonSamplerDesc, &_comparisonSampleState)))
	{
		std::cerr << "D3D11: Failed to create comparison sampler state. \n";
	}

	float aspectRatio = static_cast<float>(GetWindowWidth()) / static_cast<float>(GetWindowHeight());
	float widthScale = 1.0f / aspectRatio;
	_perFrameConstantBufferData.aspectRatio = DirectX::XMFLOAT4(static_cast<float>(GetWindowWidth()), static_cast<float>(GetWindowHeight()), aspectRatio, widthScale);
}



bool D3D11Application::Load()
{
	using namespace DirectX;


	ShaderCollectionDescriptor shaderDescriptor = {};
	D3D11_BUFFER_DESC bufferInfo = {};
	D3D11_SUBRESOURCE_DATA resourceData = {};

	shaderDescriptor.GeometryShaderFilePath.clear();
	shaderDescriptor.HullShaderFilePath.clear();
	shaderDescriptor.DomainShaderFilePath.clear();
	shaderDescriptor.InputElems = VertexType::InputElements;
	shaderDescriptor.NumElems = VertexType::InputElementCount;



	D3D11_SAMPLER_DESC linearSamplerStateDescriptor = {};
	linearSamplerStateDescriptor.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	linearSamplerStateDescriptor.AddressU = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
	linearSamplerStateDescriptor.AddressV = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
	linearSamplerStateDescriptor.AddressW = D3D11_TEXTURE_ADDRESS_MODE::D3D11_TEXTURE_ADDRESS_WRAP;
	if (FAILED(_device->CreateSamplerState(&linearSamplerStateDescriptor, &_linearSamplerState)))
	{
		std::cerr << "D3D11: Failed to create linear sampler state. \n";
		return false;
	}

	_normalMap = CreateTextureView(_device.Get(), "../Assets/Textures/teapot_normal.png");
	_displacementMap = CreateTextureView(_device.Get(), "../Assets/Textures/teapot_disp.png");


	//////////// configure the quad


	std::vector<VertexType> qVertices = {
		{XMFLOAT3(-2.0f,  -1.0f,  2.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f)}, //0
		{XMFLOAT3(-2.0f,  -1.0f, -2.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f)}, //1
		{XMFLOAT3( 2.0f,  -1.0f, -2.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f)}, //2
		{XMFLOAT3( 2.0f,  -1.0f,  2.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f)}, //3
	};

	constexpr uint32_t qIndices[] =
	{
		//Bottom
		0, 1, 2,
		2, 3, 0
	};

	bufferInfo.ByteWidth = static_cast<UINT>(sizeof(VertexType) * qVertices.size());
	bufferInfo.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
	bufferInfo.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;

	resourceData.pSysMem = qVertices.data();

	if (FAILED(_device->CreateBuffer(&bufferInfo, &resourceData, &_quadVertexBuffer)))
	{
		std::cerr << "D3D11: Failed to create quad vertex buffer. \n";
		return false;
	}

	bufferInfo.ByteWidth = sizeof(qIndices);
	bufferInfo.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;

	resourceData.pSysMem = qIndices;

	if (FAILED(_device->CreateBuffer(&bufferInfo, &resourceData, &_quadIndexBuffer)))
	{
		std::cerr << "D3D11: Failed to create quad index buffer. \n";
		return false;
	}

	//shadow shaders
	shaderDescriptor.VertexShaderFilePath = L"../Assets/Shaders/shadow.vs.hlsl";
	shaderDescriptor.PixelShaderFilePath = L"../Assets/Shaders/shadow.ps.hlsl";

	_shadowShaderCollection = ShaderCollection::CreateShaderCollection(shaderDescriptor, _device.Get());


	//main shaders 
	shaderDescriptor.VertexShaderFilePath = L"../Assets/Shaders/main.vs.hlsl";
	shaderDescriptor.PixelShaderFilePath = L"../Assets/Shaders/main.ps.hlsl";
	shaderDescriptor.GeometryShaderFilePath.clear();
	shaderDescriptor.HullShaderFilePath = L"../Assets/Shaders/main.hs.hlsl";
	shaderDescriptor.DomainShaderFilePath = L"../Assets/Shaders/main.ds.hlsl";

	_shaderCollection = ShaderCollection::CreateShaderCollection(shaderDescriptor, _device.Get());


	//triangulation shaders
	shaderDescriptor.GeometryShaderFilePath = L"../Assets/Shaders/triangulate.gs.hlsl";
	shaderDescriptor.PixelShaderFilePath = L"../Assets/Shaders/triangulate.ps.hlsl";
	

	_triangulateShaderCollection = ShaderCollection::CreateShaderCollection(shaderDescriptor, _device.Get());
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

	XMFLOAT3 _cameraPosition = { 0.0f, 0.5f, InputHandler::camDistance };
	
	_tessellationConstantBufferData.tessellationFactor = { InputHandler::tessFactor, 0.0f, 0.0f, 0.0f };
	_menuData.tessFactor = InputHandler::tessFactor;

	//camera configuration + view and proj matrices
	XMVECTOR camPos = XMLoadFloat3(&_cameraPosition);
	
	XMVECTOR camYaw = XMQuaternionRotationAxis({ 0, 1, 0 }, InputHandler::camX);
	XMVECTOR camPitch = XMQuaternionRotationAxis({ 1, 0, 0 }, InputHandler::camY);
	XMVECTOR camRotate = XMQuaternionMultiply(camPitch, camYaw);

	camPos = XMVector3Rotate(camPos, camRotate);
	_menuData.camPos[0] = XMVectorGetX(camPos);
	_menuData.camPos[1] = XMVectorGetY(camPos);
	_menuData.camPos[2] = XMVectorGetZ(camPos);

	XMVECTOR focusPos = { 0, 0, 0, 0 };
	XMVECTOR upDir = { 0,1,0,0 };

	XMMATRIX view = XMMatrixLookAtRH(camPos, focusPos, upDir);

	XMMATRIX proj = XMMatrixPerspectiveFovRH(XM_PIDIV2, static_cast<float>(_width) / static_cast<float>(_height), 0.1f, 100.0f);

	XMStoreFloat4x4(&_perFrameConstantBufferData.viewMatrix, view);
	XMStoreFloat4x4(&_perFrameConstantBufferData.projectionMatrix, proj);
	
	_perFrameConstantBufferData.viewPos = XMFLOAT4(XMVectorGetX(camPos), XMVectorGetY(camPos), XMVectorGetZ(camPos), 1.0);

	XMFLOAT3 lightPosition = XMFLOAT3(1.0f, 2.5f, 2.0f);
	XMFLOAT3 lightColor = XMFLOAT3(1.0, 1.0, 1.0);

	_lightConstantBufferData.lightPos = XMFLOAT4(lightPosition.x, lightPosition.y, lightPosition.z, 1.0);

	XMMATRIX lightView = XMMatrixLookAtRH(XMLoadFloat3(&lightPosition), focusPos, upDir);
	XMMATRIX lightProj = XMMatrixOrthographicOffCenterRH(-5.0f, 5.0f, -5.0f, 5.0f, 0.01f, 6.0f);

	XMMATRIX lightViewProj = XMMatrixMultiply(lightView, lightProj);
	XMStoreFloat4x4(&_lightConstantBufferData.lightViewProjectionMatrix, lightViewProj);

	

	//3d object transformations
	XMMATRIX originTranslation = XMMatrixTranslation(0.0f, 0.0f, -0.3f);

	XMMATRIX translation = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
	XMMATRIX scaling = XMMatrixScaling(_scale, _scale, _scale);



	//model matrices

	XMMATRIX modelMatrix = XMMatrixIdentity() * translation;
	XMMATRIX invTranspose = XMMatrixTranspose(XMMatrixInverse(nullptr, modelMatrix));


	XMStoreFloat4x4(&_quadObjConstantBufferData.modelMatrix, modelMatrix);
	XMStoreFloat4x4(&_quadObjConstantBufferData.invTranspose, invTranspose);



	_menuData.showMenu = InputHandler::toggleGuiMenu;
	ImGuiMenu::Update(_menuData);

	//update constant buffers
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	_deviceContext->Map(_perFrameConstantBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &_perFrameConstantBufferData, sizeof(PerFrameConstantBuffer));
	_deviceContext->Unmap(_perFrameConstantBuffer.Get(), 0);
	
	_deviceContext->Map(_quadObjConstantBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &_quadObjConstantBufferData, sizeof(PerObjectConstantBuffer));
	_deviceContext->Unmap(_quadObjConstantBuffer.Get(), 0);

	_deviceContext->Map(_lightConstantBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &_lightConstantBufferData, sizeof(LightConstantBuffer));
	_deviceContext->Unmap(_lightConstantBuffer.Get(), 0);

	_deviceContext->Map(_tessellationConstantBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &_tessellationConstantBufferData, sizeof(TessellationConstantBuffer));
	_deviceContext->Unmap(_tessellationConstantBuffer.Get(), 0);
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

	D3D11_VIEWPORT shadowViewport = {
		0.0f, //TopLeftX
		0.0f, //TopLeftY
		static_cast<float>(_shadowMapDimension),
		static_cast<float>(_shadowMapDimension),
		0.0f, //MinDepth
		1.0f //MaxDepth
	};

	constexpr float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	constexpr float clearTeapotTargetColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	UINT stride = sizeof(VertexType);
	constexpr UINT vertexOffset = 0;
	ID3D11RenderTargetView* nullTarget = nullptr;

	ID3D11Buffer* constantBuffers[4] =
	{
		_perFrameConstantBuffer.Get(),
		_lightConstantBuffer.Get(),
		_quadObjConstantBuffer.Get(),
		_tessellationConstantBuffer.Get(),
	};

	ImGui::Render();

	// render shadowmap
	_deviceContext->ClearRenderTargetView(_renderTarget.Get(), clearColor);
	_deviceContext->ClearDepthStencilView(_shadowDSV.Get(), D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH, 1.0f, 0);

	_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	_deviceContext->OMSetRenderTargets(0, nullptr, _shadowDSV.Get());
	
	_deviceContext->RSSetState(_shadowRasterState.Get());
	_deviceContext->RSSetViewports(1, &shadowViewport);
	
	
	_deviceContext->OMSetDepthStencilState(_depthState.Get(), 0); //enable depth writing

	_shadowShaderCollection.ApplyToContext(_deviceContext.Get());






	// render to main back buffer
	_deviceContext->OMSetRenderTargets(1, _renderTarget.GetAddressOf(), _depthTarget.Get());
	_deviceContext->ClearDepthStencilView(_depthTarget.Get(), D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH, 1.0f, 0);
	_deviceContext->RSSetViewports(1, &viewport);
	_deviceContext->RSSetState(_rasterState.Get());

	//render quad (normal)
	/*
	_shaderCollection.ApplyToContext(_deviceContext.Get());

	_deviceContext->IASetVertexBuffers(0, 1, _quadVertexBuffer.GetAddressOf(), &stride, &vertexOffset);
	_deviceContext->IASetIndexBuffer(_quadIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);


	_deviceContext->VSSetConstantBuffers(0, 3, constantBuffers);

	_deviceContext->PSSetSamplers(0, 1, _linearSamplerState.GetAddressOf());
	_deviceContext->PSSetShaderResources(0, 1, _normalMap.GetAddressOf());

	_deviceContext->PSSetSamplers(1, 1, _comparisonSampleState.GetAddressOf());
	_deviceContext->PSSetShaderResources(1, 1, _shadowSRV.GetAddressOf());

	_deviceContext->DrawIndexed(6, 0, 0);
	*/

	/////////
	//render quad (displace)
	
	_shaderCollection.ApplyToContext(_deviceContext.Get());
	_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	_deviceContext->IASetVertexBuffers(0, 1, _quadVertexBuffer.GetAddressOf(), &stride, &vertexOffset);
	_deviceContext->IASetIndexBuffer(_quadIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	_deviceContext->HSSetConstantBuffers(0, 1, &constantBuffers[3]);
	_deviceContext->DSSetConstantBuffers(0, 3, constantBuffers);


	//_deviceContext->DSSetSamplers(0, 1, _linearSamplerState.GetAddressOf());
	//_deviceContext->DSSetShaderResources(0, 1, _displacementMap.GetAddressOf());

	_deviceContext->PSSetSamplers(0, 1, _linearSamplerState.GetAddressOf());
	_deviceContext->PSSetShaderResources(0, 1, _normalMap.GetAddressOf());

	_deviceContext->PSSetSamplers(1, 1, _comparisonSampleState.GetAddressOf());
	_deviceContext->PSSetShaderResources(1, 1, _shadowSRV.GetAddressOf());

	_deviceContext->Draw(4, 0);
	
	/////////
	
	//triangulate
	if (InputHandler::toggleTriangulation)
	{
		_triangulateShaderCollection.ApplyToContext(_deviceContext.Get());

		_deviceContext->VSSetConstantBuffers(0, 3, constantBuffers);
		_deviceContext->Draw(4, 0);;
	}
	


	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	_swapChain->Present(1, 0);
}
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
	_teapotVertexBuffer.Reset();
	_perFrameConstantBuffer.Reset();
	_teapotObjConstantBuffer.Reset();
	_teapotShaderCollection.Destroy();
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

	depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // no depth writing
	_device->CreateDepthStencilState(&depthDesc, &_depthLessState);
}

void D3D11Application::CreateConstantBuffers()
{
	D3D11_BUFFER_DESC desc{};
	desc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
	desc.ByteWidth = sizeof(PerFrameConstantBuffer);
	desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;

	_device->CreateBuffer(&desc, nullptr, &_perFrameConstantBuffer);
	_device->CreateBuffer(&desc, nullptr, &_refFrameConstantBuffer);

	desc.ByteWidth = sizeof(PerObjectConstantBuffer);
	_device->CreateBuffer(&desc, nullptr, &_teapotObjConstantBuffer);
	_device->CreateBuffer(&desc, nullptr, &_quadObjConstantBuffer);

	desc.ByteWidth = sizeof(LightConstantBuffer);
	_device->CreateBuffer(&desc, nullptr, &_lightConstantBuffer);
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

	float aspectRatio = static_cast<float>(GetWindowWidth()) / static_cast<float>(GetWindowHeight());
	float widthScale = 1.0f / aspectRatio;
	_perFrameConstantBufferData.aspectRatio = DirectX::XMFLOAT4(static_cast<float>(GetWindowWidth()), static_cast<float>(GetWindowHeight()), aspectRatio, widthScale);
}



bool D3D11Application::Load()
{
	using namespace DirectX;

	//////////// configure skybox 

	ShaderCollectionDescriptor shaderDescriptor = {};
	shaderDescriptor.VertexShaderFilePath = L"../Assets/Shaders/sky.vs.hlsl";
	shaderDescriptor.PixelShaderFilePath = L"../Assets/Shaders/sky.ps.hlsl";
	shaderDescriptor.InputElems = VertexPosition::InputElements;
	shaderDescriptor.NumElems = VertexPosition::InputElementCount;

	_skyShaderCollection = ShaderCollection::CreateShaderCollection(shaderDescriptor, _device.Get());

	std::vector<VertexPosition> cVertices = {
		// Front
		{XMFLOAT3(-0.5f,  -0.5f,  0.5f)},  // 0
		{XMFLOAT3( 0.5f,  -0.5f,  0.5f)},  // 1
		{XMFLOAT3(-0.5f,   0.5f,  0.5f)},  // 2
		{XMFLOAT3( 0.5f,   0.5f,  0.5f)},  // 3

		// Back
		{XMFLOAT3(-0.5f,  -0.5f, -0.5f) }, // 4
		{XMFLOAT3( 0.5f,  -0.5f, -0.5f) }, // 5
		{XMFLOAT3(-0.5f,   0.5f, -0.5f)},  // 6
		{XMFLOAT3( 0.5f,   0.5f, -0.5f)},  // 7
	};

	constexpr uint32_t indices[] =
	{
		//Front
		3, 2, 0,
		0, 1, 3,

		//Back
		4, 6, 7,
		7, 5, 4,

		//Left
		0, 2, 6,
		6, 4, 0,

		//Right
		7, 3, 1,
		1, 5, 7,

		//Top
		7, 6, 2,
		2, 3, 7,

		//Bottom
		0, 4, 5,
		5, 1, 0

	};


	D3D11_BUFFER_DESC bufferInfo = {};
	bufferInfo.ByteWidth = static_cast<UINT>(sizeof(VertexType) * cVertices.size());
	bufferInfo.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
	bufferInfo.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA resourceData = {};
	resourceData.pSysMem = cVertices.data();

	if (FAILED(_device->CreateBuffer(&bufferInfo, &resourceData, &_skyVertexBuffer)))
	{
		std::cerr << "D3D11: Failed to create sky vertex buffer. \n";
		return false;
	}

	bufferInfo.ByteWidth = sizeof(indices);
	bufferInfo.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;

	resourceData.pSysMem = indices;

	if (FAILED(_device->CreateBuffer(&bufferInfo, &resourceData, &_skyIndexBuffer)))
	{
		std::cerr << "D3D11: Failed to create skymap index buffer. \n";
		return false;
	}

	_skyMapSRV = CreateCubeView(_device.Get(), "../Assets/Textures/skymap/");




	//////////// configure teapot 

	shaderDescriptor.VertexShaderFilePath = L"../Assets/Shaders/teapot.vs.hlsl";
	shaderDescriptor.PixelShaderFilePath = L"../Assets/Shaders/teapot.ps.hlsl";
	shaderDescriptor.InputElems = VertexType::InputElements;
	shaderDescriptor.NumElems = VertexType::InputElementCount;

	_teapotShaderCollection = ShaderCollection::CreateShaderCollection(shaderDescriptor, _device.Get());

	shaderDescriptor.VertexShaderFilePath = L"../Assets/Shaders/teapotref.vs.hlsl";
	shaderDescriptor.PixelShaderFilePath = L"../Assets/Shaders/teapotref.ps.hlsl";

	_refTeapotShaderCollection = ShaderCollection::CreateShaderCollection(shaderDescriptor, _device.Get());
	
	_menuData.objFile = "../Assets/Models/teapot/teapot.obj";
	std::vector<VertexType> vertices;
	if (!LoadOBJ(_menuData.objFile, vertices, _device.Get()))
	{
		std::cerr << "ModelLoad: Failed to load obj file. \n";
	}

	_drawCount = static_cast<UINT>(vertices.size());
	_menuData.numVertices = _drawCount;

	bufferInfo.ByteWidth = static_cast<UINT>(sizeof(VertexType) * vertices.size());
	bufferInfo.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
	bufferInfo.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;

	resourceData.pSysMem = vertices.data();

	if (FAILED(_device->CreateBuffer(&bufferInfo, &resourceData, &_teapotVertexBuffer)))
	{
		std::cerr << "D3D11: Failed to create teapot vertex buffer. \n";
		return false;
	}


	_materialConstantBufferData.shininess = 32;

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




	//////////// configure the quad

	shaderDescriptor.VertexShaderFilePath = L"../Assets/Shaders/quad.vs.hlsl";
	shaderDescriptor.PixelShaderFilePath = L"../Assets/Shaders/quad.ps.hlsl";
	shaderDescriptor.InputElems = VertexType::InputElements;
	shaderDescriptor.NumElems = VertexType::InputElementCount;

	_quadShaderCollection = ShaderCollection::CreateShaderCollection(shaderDescriptor, _device.Get());

	std::vector<VertexType> qVertices = {
		{XMFLOAT3(-1.0f,  -1.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f)}, //0
		{XMFLOAT3(-1.0f,  -1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f)}, //1
		{XMFLOAT3( 1.0f,  -1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f)}, //2
		{XMFLOAT3( 1.0f,  -1.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f)}, //3
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

	float radius = InputHandler::camDistance;        // Orbit distance from focus point
	float yaw = InputHandler::camX;                  // Horizontal angle (in radians)
	float pitch = -InputHandler::camY;
	
	//camera configuration + view and proj matrices
	XMFLOAT3 target = { 0.0f, 0.5f, 0.0f };          // Point to orbit around (e.g., the object or scene center)

	// Calculate camera position in world space
	XMFLOAT3 camPosFloat = {
		target.x + radius * cosf(pitch) * sinf(yaw),
		target.y + radius * sinf(pitch),
		target.z + radius * cosf(pitch) * cosf(yaw)
	};
	XMVECTOR camPos = XMLoadFloat3(&camPosFloat);

	_menuData.camPos[0] = XMVectorGetX(camPos);
	_menuData.camPos[1] = XMVectorGetY(camPos);
	_menuData.camPos[2] = XMVectorGetZ(camPos);

	XMVECTOR focusPos = XMLoadFloat3(&target);
	XMVECTOR upDir = { 0,1,0,0 };

	XMMATRIX view = XMMatrixLookAtRH(camPos, focusPos, upDir);

	XMMATRIX proj = XMMatrixPerspectiveFovRH( 90.0f * 0.0174533f, static_cast<float>(_width) / static_cast<float>(_height), 0.1f, 100.0f);

	XMStoreFloat4x4(&_perFrameConstantBufferData.viewMatrix, view);
	XMStoreFloat4x4(&_perFrameConstantBufferData.projectionMatrix, proj);
	



	XMVECTOR plane = XMVectorSet(0, 1.0f, 0, 0.5f);  // Assuming plane y = 0.5f
	XMMATRIX reflectionMatrix = XMMatrixReflect(plane);


	XMVECTOR refCamPos = XMVector3TransformCoord(camPos, reflectionMatrix);

	XMVECTOR refUpDir = XMVector3TransformNormal(upDir, reflectionMatrix);

	XMMATRIX refView = XMMatrixLookAtRH(refCamPos, focusPos, refUpDir);


	XMStoreFloat4x4(&_refFrameConstantBufferData.viewMatrix, refView);
	XMStoreFloat4x4(&_refFrameConstantBufferData.projectionMatrix, proj);




	XMFLOAT3 lightPosition = XMFLOAT3(1.0, 4.0, -2);
	XMFLOAT3 lightColor = XMFLOAT3(1.0, 1.0, 1.0);

	_perFrameConstantBufferData.viewPos = XMFLOAT4(XMVectorGetX(camPos), XMVectorGetY(camPos), XMVectorGetZ(camPos), 1.0);

	_lightConstantBufferData.lightPos = XMFLOAT4(lightPosition.x, lightPosition.y, lightPosition.z, 0.0);

	//3d object transformations
	XMMATRIX originTranslation = XMMatrixTranslation(0.0f, 0.0f, -0.3f);


	XMMATRIX translation = XMMatrixTranslation(0.0f, 0.1f, 0.0f);
	XMMATRIX scaling = XMMatrixScaling(_scale, _scale, _scale);
	XMMATRIX rotation = XMMatrixRotationRollPitchYaw(-XM_PIDIV2,0 , 0);
	_menuData.potRotX = InputHandler::xRotation;
	_menuData.potRotY = InputHandler::yRotation;


	//model matrices

	XMMATRIX modelMatrix = originTranslation * scaling * rotation * translation;
	XMMATRIX invTranspose = XMMatrixTranspose(XMMatrixInverse(nullptr, modelMatrix));

	XMStoreFloat4x4(&_teapotObjConstantBufferData.modelMatrix, modelMatrix);
	XMStoreFloat4x4(&_teapotObjConstantBufferData.invTranspose, invTranspose);


	translation = XMMatrixTranslation(0.0f, 0.5f, 0.0f);
	modelMatrix = XMMatrixIdentity() * translation;
	invTranspose = XMMatrixTranspose(XMMatrixInverse(nullptr, modelMatrix));

	XMStoreFloat4x4(&_quadObjConstantBufferData.modelMatrix, modelMatrix);
	XMStoreFloat4x4(&_quadObjConstantBufferData.invTranspose, invTranspose);



	_menuData.showMenu = InputHandler::toggleGuiMenu;
	ImGuiMenu::Update(_menuData);

	//update constant buffers
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	_deviceContext->Map(_perFrameConstantBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &_perFrameConstantBufferData, sizeof(PerFrameConstantBuffer));
	_deviceContext->Unmap(_perFrameConstantBuffer.Get(), 0);

	_deviceContext->Map(_refFrameConstantBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &_refFrameConstantBufferData, sizeof(PerFrameConstantBuffer));
	_deviceContext->Unmap(_refFrameConstantBuffer.Get(), 0);

	_deviceContext->Map(_teapotObjConstantBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &_teapotObjConstantBufferData, sizeof(PerObjectConstantBuffer));
	_deviceContext->Unmap(_teapotObjConstantBuffer.Get(), 0);
	
	_deviceContext->Map(_quadObjConstantBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &_quadObjConstantBufferData, sizeof(PerObjectConstantBuffer));
	_deviceContext->Unmap(_quadObjConstantBuffer.Get(), 0);

	_deviceContext->Map(_lightConstantBuffer.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &_lightConstantBufferData, sizeof(LightConstantBuffer));
	_deviceContext->Unmap(_lightConstantBuffer.Get(), 0);
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

	constexpr float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	constexpr float clearTeapotTargetColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	UINT stride = sizeof(VertexType);
	constexpr UINT vertexOffset = 0;
	ID3D11RenderTargetView* nullTarget = nullptr;

	ID3D11Buffer* constantBuffers[6] =
	{
		_perFrameConstantBuffer.Get(),
		_lightConstantBuffer.Get(),
		_teapotObjConstantBuffer.Get(),
		_quadObjConstantBuffer.Get(),
		_refFrameConstantBuffer.Get()
	};

	ImGui::Render();

	// render teapot to texture
	_deviceContext->OMSetRenderTargets(1, &nullTarget, nullptr);
	_deviceContext->OMSetRenderTargets(1, _renderTextureView.GetAddressOf(), _depthTarget.Get());
	_deviceContext->ClearRenderTargetView(_renderTextureView.Get(), clearTeapotTargetColor);
	_deviceContext->ClearDepthStencilView(_depthTarget.Get(), D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH, 1.0f, 0);

	

	_deviceContext->RSSetViewports(1, &viewport);
	_deviceContext->RSSetState(_rasterState.Get());
	_deviceContext->OMSetDepthStencilState(_depthState.Get(), 0); //enable depth writing

	_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	_deviceContext->IASetVertexBuffers(0, 1, _teapotVertexBuffer.GetAddressOf(), &stride, &vertexOffset);
	_refTeapotShaderCollection.ApplyToContext(_deviceContext.Get());


	_deviceContext->PSSetShaderResources(0, 1, _skyMapSRV.GetAddressOf());
	_deviceContext->PSSetSamplers(0, 1, _linearSamplerState.GetAddressOf());


	_deviceContext->VSSetConstantBuffers(0, 1, &constantBuffers[4]);
	_deviceContext->VSSetConstantBuffers(1, 2, &constantBuffers[1]);


	_deviceContext->Draw(_drawCount, 0);

	// render to main back buffer
	_deviceContext->OMSetRenderTargets(1, _renderTarget.GetAddressOf(), _depthTarget.Get());
	_deviceContext->ClearRenderTargetView(_renderTarget.Get(), clearColor);
	_deviceContext->ClearDepthStencilView(_depthTarget.Get(), D3D11_CLEAR_FLAG::D3D11_CLEAR_DEPTH, 1.0f, 0);

	_deviceContext->OMSetDepthStencilState(_depthLessState.Get(), 0); //disable depth writing

	

	//prepare and render skybox
	stride = sizeof(DirectX::VertexPosition);

	_deviceContext->IASetVertexBuffers(0, 1, _skyVertexBuffer.GetAddressOf(), &stride, &vertexOffset);
	_deviceContext->IASetIndexBuffer(_skyIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	_skyShaderCollection.ApplyToContext(_deviceContext.Get());

	_deviceContext->PSSetShaderResources(0, 1, _skyMapSRV.GetAddressOf());
	_deviceContext->PSSetSamplers(0, 1, _linearSamplerState.GetAddressOf());

	_deviceContext->VSSetConstantBuffers(0, 1, &constantBuffers[0]);

	_deviceContext->DrawIndexed(36, 0, 0);
	
	_deviceContext->OMSetDepthStencilState(_depthState.Get(), 0); //enable depth writing

	//render teapot
	stride = sizeof(VertexType);
	_deviceContext->IASetVertexBuffers(0, 1, _teapotVertexBuffer.GetAddressOf(), &stride, &vertexOffset);


	_teapotShaderCollection.ApplyToContext(_deviceContext.Get());


	_deviceContext->PSSetShaderResources(0, 1, _skyMapSRV.GetAddressOf());
	_deviceContext->PSSetSamplers(0, 1, _linearSamplerState.GetAddressOf());

	_deviceContext->VSSetConstantBuffers(0, 1, &constantBuffers[0]);
	_deviceContext->VSSetConstantBuffers(1, 2, &constantBuffers[1]);


	_deviceContext->Draw(_drawCount, 0);


	//render quad
	_deviceContext->IASetVertexBuffers(0, 1, _quadVertexBuffer.GetAddressOf(), &stride, &vertexOffset);
	_deviceContext->IASetIndexBuffer(_quadIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	_quadShaderCollection.ApplyToContext(_deviceContext.Get());


	_deviceContext->PSSetShaderResources(0, 1, _skyMapSRV.GetAddressOf());
	_deviceContext->PSSetShaderResources(1, 1, _textureResourceView.GetAddressOf());
	_deviceContext->PSSetSamplers(0, 1, _linearSamplerState.GetAddressOf());

	_deviceContext->VSSetConstantBuffers(0, 2, &constantBuffers[0]);
	_deviceContext->VSSetConstantBuffers(2, 1, &constantBuffers[3]);

	_deviceContext->DrawIndexed(6, 0, 0);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	_swapChain->Present(1, 0);
}
#pragma once

#include <wrl/client.h>
#include <DirectXMath.h>

namespace WRL = Microsoft::WRL;

struct PerFrameConstantBuffer
{
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;
	DirectX::XMFLOAT4 viewPos;
	DirectX::XMFLOAT4 aspectRatio; //width, height, ratio, widthScale
};

struct PerObjectConstantBuffer
{
	DirectX::XMFLOAT4X4 modelMatrix;
	DirectX::XMFLOAT4X4 invTranspose;
	
};

struct LightConstantBuffer
{
	DirectX::XMFLOAT4 lightPos;
	DirectX::XMFLOAT4 lightColor;
	DirectX::XMFLOAT4X4 lightViewProjectionMatrix;
};

struct MaterialConstantBuffer
{
	DirectX::XMFLOAT3 materialColor;
	float shininess;
};

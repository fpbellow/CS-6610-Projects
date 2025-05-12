#pragma once

#include <wrl/client.h>
#include <DirectXMath.h>

namespace WRL = Microsoft::WRL;

struct PerFrameConstantBuffer
{
	DirectX::XMFLOAT4X4 viewProjectionMatrix;
	DirectX::XMFLOAT4 viewPos;
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
};

struct MaterialConstantBuffer
{
	DirectX::XMFLOAT3 materialColor;
	float shininess;
};

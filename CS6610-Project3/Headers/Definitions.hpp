#pragma once

#include <wrl/client.h>
#include <DirectXMath.h>

namespace WRL = Microsoft::WRL;

struct PerFrameConstantBuffer
{
	DirectX::XMFLOAT4X4 viewProjectionMatrix;
	DirectX::XMFLOAT4 viewPos;
	DirectX::XMFLOAT4 lightPos;
	DirectX::XMFLOAT4 lightColor;
};

struct PerObjectConstantBuffer
{
	DirectX::XMFLOAT4X4 modelMatrix;
	DirectX::XMFLOAT4X4 invTranspose;
	DirectX::XMFLOAT3 materialColor;
	float shininess;
};


struct ObjectTransforms
{
	DirectX::XMFLOAT3 rotation;
	float camDistance;
};

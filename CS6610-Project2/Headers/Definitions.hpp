#pragma once

#include <wrl/client.h>
#include <DirectXMath.h>

namespace WRL = Microsoft::WRL;

struct PerObjectConstantBuffer
{
	DirectX::XMFLOAT4X4 modelMatrix;
};

struct PerFrameConstantBuffer
{
	DirectX::XMFLOAT4X4 viewProjectionMatrix;
};

struct ObjectTransforms
{
	DirectX::XMFLOAT3 rotation;
	float camDistance;
};

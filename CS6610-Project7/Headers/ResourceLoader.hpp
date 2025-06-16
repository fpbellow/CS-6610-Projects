#pragma once

#include "Definitions.hpp"
#include "lodepng.hpp"

#include <VertexTypes.h>
#include <filesystem>

#include <string>
#include <vector>
#include <iostream>


struct Material
{
	std::string name;
	DirectX::XMFLOAT3 diffuse;
	DirectX::XMFLOAT3 specular;
	float shininess;
	WRL::ComPtr<ID3D11ShaderResourceView> diffuseTexture = nullptr;
	WRL::ComPtr<ID3D11ShaderResourceView> specularTexture = nullptr;
};


bool LoadOBJ(const std::string& filename, std::vector<DirectX::VertexPositionNormalTexture>& vertices, std::vector<Material>& materials, ID3D11Device* device);

WRL::ComPtr<ID3D11ShaderResourceView> CreateTextureView(ID3D11Device* device, const std::string& pathToTexture);
WRL::ComPtr<ID3D11ShaderResourceView> CreateCubeView(ID3D11Device* device, const std::string& folderPath);
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
	float shininess = 0.0;
	WRL::ComPtr<ID3D11ShaderResourceView> diffuseTexture = nullptr;
	WRL::ComPtr<ID3D11ShaderResourceView> specularTexture = nullptr;
};


WRL::ComPtr<ID3D11ShaderResourceView> CreateTextureView(ID3D11Device* device, const std::string& pathToTexture);

#include "../Headers/ResourceLoader.hpp"


WRL::ComPtr<ID3D11ShaderResourceView> CreateTextureView(ID3D11Device* device, const std::string& pathToTexture)
{
	std::vector<unsigned char> image;
	unsigned width, height;
	unsigned error = lodepng::decode(image, width, height, pathToTexture);

	if (error != 0)
	{
		std::cerr << "CreateTextureView: Failed to load texture file (" << pathToTexture <<
			"). \n LodePNG error: " << lodepng_error_text(error) << std::endl;
		return nullptr;
	}

	DXGI_FORMAT textureFormat = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Format = textureFormat;
	textureDesc.ArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	//prepare resource data
	D3D11_SUBRESOURCE_DATA initialData = {};
	initialData.pSysMem = image.data();
	initialData.SysMemPitch = static_cast<UINT>(width * 4);


	//create the texture
	WRL::ComPtr<ID3D11Texture2D> texture = nullptr;
	if (FAILED(device->CreateTexture2D(&textureDesc, &initialData, texture.GetAddressOf())))
	{
		std::cerr << "CreateTextureView: Failed to create texture from file: " << pathToTexture << ". \n";
		return nullptr;
	}

	ID3D11ShaderResourceView* srv = nullptr;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = textureDesc.Format;
	srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;

	if (FAILED(device->CreateShaderResourceView(texture.Get(), &srvDesc, &srv)))
	{
		std::cerr << "CreateTextureView: Failed to create SRV from texture" << pathToTexture << ". \n";
		return nullptr;
	}

	return srv;
}


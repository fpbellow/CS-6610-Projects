#include "../Headers/ResourceLoader.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../Headers/tiny_obj_loader.h"

bool LoadOBJ(const std::string& filename, std::vector<DirectX::VertexPositionNormalTexture>& vertices, std::vector<Material>& materials, ID3D11Device* device)
{
	tinyobj::ObjReaderConfig reader_config;
	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(filename, reader_config)) {
		if (!reader.Error().empty())
			std::cerr << "TinyObjReader: " << reader.Error() << std::endl;

		return false;
	}

	if (!reader.Warning().empty())
		std::cerr << "TinyObjReader " << reader.Warning() << std::endl;

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	auto& mats = reader.GetMaterials();

	//load vertex data
	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			DirectX::VertexPositionNormalTexture vertex = {};
			vertex.position = {
				DirectX::XMFLOAT3(
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				),
			};

			if (index.normal_index >= 0)
			{
				vertex.normal = {
					DirectX::XMFLOAT3(
						attrib.normals[3 * index.normal_index + 0],
						attrib.normals[3 * index.normal_index + 1],
						attrib.normals[3 * index.normal_index + 2]
					)
				};
			}

			if (index.texcoord_index >= 0) {
				vertex.textureCoordinate = 
				{
					DirectX::XMFLOAT2
					(
						attrib.texcoords[2 * index.texcoord_index + 0],
						attrib.texcoords[2 * index.texcoord_index + 1]
					)
				};
			}

			vertices.push_back(vertex);
		}
	}

	//load material data (if any)
	if (!mats.empty())
	{
		std::string folderPath = std::filesystem::path(filename).parent_path().string();

		for (const auto& mat : mats)
		{
			Material material;
			material.name = mat.name;
			material.diffuse = DirectX::XMFLOAT3(
				mat.diffuse[0],
				mat.diffuse[1],
				mat.diffuse[2]
			);
			material.specular = DirectX::XMFLOAT3(
				mat.specular[0],
				mat.specular[1],
				mat.specular[2]
			);
			material.shininess = mat.shininess;

			//Load texture maps if available
			if (!mat.diffuse_texname.empty())
				material.diffuseTexture = CreateTextureView(device, folderPath + "/" + mat.diffuse_texname);

			if (!mat.specular_texname.empty())
				material.specularTexture = CreateTextureView(device, folderPath + "/" + mat.specular_texname);

			materials.push_back(material);
		}
	}
	else
	{
		Material defaultMaterial;
		defaultMaterial.name = "Default";
		defaultMaterial.diffuse = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
		defaultMaterial.specular = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		defaultMaterial.shininess = 0.0f;
		defaultMaterial.diffuseTexture = nullptr;
		defaultMaterial.specularTexture = nullptr;
		materials.push_back(defaultMaterial);

		std::cerr << "LoadObj: No materials found. Using default material";
	}

	return true;
}

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

WRL::ComPtr<ID3D11ShaderResourceView> CreateCubeView(ID3D11Device* device, const std::string& folderPath)
{
	std::vector<std::vector<unsigned char>> image(6);
	unsigned width, height;

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 6;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;


	const std::vector<std::string> faceSides =
	{
		"right.png",  // Positive X
		"left.png",   // Negative X
		"top.png",    // Positive Y
		"bottom.png", // Negative Y
		"front.png",  // Positive Z
		"back.png"    // Negative Z
	};

	std::vector<D3D11_SUBRESOURCE_DATA> subresourceData(6);

	for (int i = 0; i < 6; i++)
	{
		std::string filePath = folderPath + faceSides[i];
		unsigned error = lodepng::decode(image[i], width, height, folderPath + faceSides[i]);

		if (error != 0)
		{
			std::cerr << "CreateCubeView: Failed to load texture file (" << filePath <<
				"). \n LodePNG error: " << lodepng_error_text(error) << std::endl;
			return nullptr;
		}

		if (i == 0)
		{
			textureDesc.Width = width;
			textureDesc.Height = height;
		}

		subresourceData[i].pSysMem = image[i].data();
		subresourceData[i].SysMemPitch = static_cast<UINT>(width * 4);
	}

	WRL::ComPtr<ID3D11Texture2D> cubeMapTexture = nullptr;
	if (FAILED(device->CreateTexture2D(&textureDesc, subresourceData.data(), cubeMapTexture.GetAddressOf())))
	{
		std::cerr << "CreateCubeView: Failed to create cubemap texture from files: " << folderPath << ". \n";
		return nullptr;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = 1;
	srvDesc.TextureCube.MostDetailedMip = 0;

	ID3D11ShaderResourceView* srv = nullptr;
	if (FAILED(device->CreateShaderResourceView(cubeMapTexture.Get(), &srvDesc, &srv)))
	{
		std::cerr << "CreateCubeView: Failed to create SRV from texture" << folderPath << ". \n";
		return nullptr;
	}

	return srv;
}
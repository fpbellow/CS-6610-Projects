#include "../Headers/OBJLoader.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../Headers/tiny_obj_loader.h"

bool LoadOBJ(const std::string& filename, std::vector<DirectX::VertexPositionNormalTexture>& vertices)
{
	tinyobj::ObjReaderConfig reader_config;
	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(filename, reader_config)) {
		if (!reader.Error().empty())
			std::cerr << "TinyObjReader: " << reader.Error();

		return false;
	}

	if (!reader.Warning().empty())
		std::cerr << "TinyObjReader " << reader.Warning();

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	// auto& materials = reader.GetMaterials();

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

	return true;
}


#pragma once

#include <VertexTypes.h>

#include <string>
#include <vector>
#include <iostream>

bool LoadOBJ(const std::string& filename, std::vector<DirectX::VertexPositionNormalTexture>& vertices);
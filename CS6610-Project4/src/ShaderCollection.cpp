#include "../Headers/ShaderCollection.hpp"
#include <d3dcompiler.h>
#include <iostream>


ShaderCollection ShaderCollection::CreateShaderCollection(const ShaderCollectionDescriptor& settings, ID3D11Device* device)
{
    ShaderCollection collection;

    WRL::ComPtr<ID3DBlob> vertexShaderBlob;
    collection._vertexShader = CreateVertexShader(device, settings.VertexShaderFilePath, vertexShaderBlob);
    collection._pixelShader = CreatePixelShader(device, settings.PixelShaderFilePath);
    
    if (!CreateInputLayout(device, settings.InputElems, settings.NumElems, vertexShaderBlob, collection._inputLayout))
        return {};

    return collection;
}





WRL::ComPtr<ID3D11VertexShader> ShaderCollection::CreateVertexShader(ID3D11Device* device, const std::wstring& filePath, WRL::ComPtr<ID3DBlob>& vertexShaderBlob)
{
    if (!CompileShader(filePath, "Main", "vs_5_0", vertexShaderBlob))
        return nullptr;
    
    WRL::ComPtr<ID3D11VertexShader> vertexShader;
    if (FAILED(device->CreateVertexShader(
        vertexShaderBlob->GetBufferPointer(),
        vertexShaderBlob->GetBufferSize(),
        nullptr,
        &vertexShader
    )))
    {
        std::cerr << "D3D11: Failed to compile vertex shader. \n";
        return nullptr;
    }

    return vertexShader;
}


WRL::ComPtr<ID3D11PixelShader> ShaderCollection::CreatePixelShader(ID3D11Device* device, const std::wstring& filePath)
{
    WRL::ComPtr<ID3DBlob> pixelShaderBlob = nullptr;
    if (!CompileShader(filePath, "Main", "ps_5_0", pixelShaderBlob))
        return nullptr;

    WRL::ComPtr<ID3D11PixelShader> pixelShader;
    if (FAILED(device->CreatePixelShader(
        pixelShaderBlob->GetBufferPointer(),
        pixelShaderBlob->GetBufferSize(),
        nullptr,
        &pixelShader
    )))
    {
        std::cerr << "D3D11: Failed to compile pixel shader. \n";
        return nullptr;
    }

    return pixelShader;
}


bool ShaderCollection::CreateInputLayout(ID3D11Device* device, const D3D11_INPUT_ELEMENT_DESC* InputElements, UINT InputElementCount, const WRL::ComPtr<ID3DBlob>& vertexBlob, WRL::ComPtr<ID3D11InputLayout>& inputLayout)
{

    if (FAILED(device->CreateInputLayout(
        InputElements,
        InputElementCount,
        vertexBlob->GetBufferPointer(),
        vertexBlob->GetBufferSize(),
        &inputLayout
    )))
    {
        std::cerr << "D3D11: Failed to create the input layout. \n";
        return false;
    }

    return true;
}


bool ShaderCollection::CompileShader(const std::wstring& filePath, const std::string& entryPoint, const std::string& profile, WRL::ComPtr<ID3DBlob>& shaderBlob)
{
    constexpr uint32_t compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;

    
    WRL::ComPtr<ID3DBlob> tempShaderBlob = nullptr;
    WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

    if (FAILED(D3DCompileFromFile(
        filePath.data(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint.data(),
        profile.data(),
        compileFlags,
        0,
        &tempShaderBlob,
        &errorBlob
    )))
    {
        std::wcerr << "D3D11: Failed to read shader from file: " << filePath << " \n";
        if (errorBlob != nullptr)
            std::cerr << "D3D11: With message: " << static_cast<const char*>(errorBlob->GetBufferPointer()) << "\n";

        return false;
    }

    shaderBlob = tempShaderBlob;
    return true;
}


void ShaderCollection::ApplyToContext(ID3D11DeviceContext* context)
{
    context->IASetInputLayout(_inputLayout.Get());
    context->VSSetShader(_vertexShader.Get(), nullptr, 0);
    context->PSSetShader(_pixelShader.Get(), nullptr, 0);
}


void ShaderCollection::Destroy()
{
    _vertexShader.Reset();
    _pixelShader.Reset();
    _inputLayout.Reset();
}
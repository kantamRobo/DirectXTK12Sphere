#pragma once

#pragma once
#include "pch.h"
#include <DeviceResources.h>
#include <DescriptorHeap.h>
#include <ResourceUploadBatch.h>
#include <VertexTypes.h>
#include <d3dcompiler.h>
#include <d3dx12.h>
#include <EffectPipelineStateDescription.h>
#include <DirectXHelpers.h>
#include <CommonStates.h>

#include <DirectXMath.h>
struct SceneCB {
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};
// 変更後 (推奨)
// XMFLOAT4を使うことで、自動的に16バイトサイズになり、HLSLのアライメントと一致します。
// ※w成分は使用しませんが、詰め物(パディング)として機能します。
struct Lambert
{
	DirectX::XMFLOAT4 LightDir;       // 16 bytes
	DirectX::XMFLOAT4 LightColor;     // 16 bytes
	DirectX::XMFLOAT4 MaterialColor;  // 16 bytes
	DirectX::XMFLOAT4 AmbientColor;   // 16 bytes
};

// Create root signature.
enum RootParameterIndex
{
	SceneBuffer,
	LambertBuffer,
	TextureSRV,
	TextureSampler,
	RootParameterCou

};


class DirectXTK12Spehere_Lambert
{
public:
	HRESULT CreateBuffer(DirectX::GraphicsMemory* graphicsmemory, DX::DeviceResources* deviceResources, int height, int width);


	std::unique_ptr<DirectX::DescriptorHeap> m_resourceDescriptors;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> CreateGraphicsPipelineState(DX::DeviceResources* deviceresources, const std::wstring& vertexShaderPath, const std::wstring& pixelShaderPath);
	void CreateDescriptors(DX::DeviceResources* DR);
	void Draw(const DX::DeviceResources* DR);


	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

	Lambert lambert;


	//バッファ

	Microsoft::WRL::ComPtr<ID3D12Resource> m_ConstantBuffer;

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_layout;
	std::vector<DirectX::VertexPositionNormal> vertices;
	std::vector<unsigned short> indices;

	DirectX::XMMATRIX modelmat;
	//シェーダーの作成
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;//新規追加
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;//新規追加
 	DirectX::SharedGraphicsResource SceneCBResource;//新規追加
    DirectX::SharedGraphicsResource lambertCB;
	DirectX::SharedGraphicsResource m_VertexBuffer;
	DirectX::SharedGraphicsResource m_IndexBuffer;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;//新規追加
};


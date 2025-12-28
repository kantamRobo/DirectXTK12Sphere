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
struct Lambert
{
	DirectX::XMFLOAT3 LightDir;       // ライトの進行方向 (例: 上から下なら 0, -1, 0)
	DirectX::XMFLOAT3 LightColor;     // ライトの色と強さ (例: 1.0, 1.0, 1.0)
	DirectX::XMFLOAT3 MaterialColor;  // 物体の色 (アルベド)
	DirectX::XMFLOAT3 AmbientColor;   // 環境光 (真っ黒になるのを防ぐための底上げ用の光)

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
	DirectX::SharedGraphicsResource m_VertexBuffer;
	DirectX::SharedGraphicsResource m_IndexBuffer;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;//新規追加
};


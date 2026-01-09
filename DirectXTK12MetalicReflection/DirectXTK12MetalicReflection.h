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
#include <DDSTextureLoader.h>
#include <DirectXMath.h>
#include <CommonStates.h>
struct SceneCB {
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};
// 変更後 (推奨)
// XMFLOAT4を使うことで、自動的に16バイトサイズになり、HLSLのアライメントと一致します。
// ※w成分は使用しませんが、詰め物(パディング)として機能します。



// デスクリプタヒープのインデックス管理用
enum DescriptorsMetal
{
	EnvMapDiff = 0,
	CountMetal
};
// Create root signature.
enum RootParameterIndex
{
	SceneBuffer,
	MetalicBuffer,
	TextureSRV,
	TextureSampler,
	RootParameterCou

};
// HLSLの cbuffer MaterialBuffer に対応
struct MaterialConstants
{
	DirectX::XMFLOAT3 CameraPos;
	float Padding1;             // float3の後ろの隙間埋め

	DirectX::XMFLOAT3 AlbedoColor;
	float Roughness;            // ここでちょうど16バイト境界 (12 + 4)

	float F0;
	float Padding2[3];          // 構造体サイズを16の倍数にするためのパディング
};

class DirectXTK12MetalicReflection
{
public:
	HRESULT CreateBuffer(DirectX::GraphicsMemory* graphicsmemory, DX::DeviceResources* deviceResources, int height, int width);


	std::unique_ptr<DirectX::DescriptorHeap> m_resourceDescriptors;
	//サンプラーヒープ

	Microsoft::WRL::ComPtr<ID3D12PipelineState> CreateGraphicsPipelineState(DX::DeviceResources* deviceresources, const std::wstring& vertexShaderPath, const std::wstring& pixelShaderPath);
	void CreateDescriptors(DX::DeviceResources* DR);
	void Draw(const DX::DeviceResources* DR);
	void InitializeResources(DX::DeviceResources* DR);
	

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

	MaterialConstants cb;
	std::unique_ptr<DirectX::CommonStates> m_states;

	//バッファ

	Microsoft::WRL::ComPtr<ID3D12Resource> m_ConstantBuffer;

	// DirectXTK12のコンポーネント
	

	
	Microsoft::WRL::ComPtr<ID3D12Resource>   m_envMapTexture;       // キューブマップテクスチャ


	std::vector<D3D12_INPUT_ELEMENT_DESC> m_layout;
	std::vector<DirectX::VertexPositionNormal> vertices;
	std::vector<unsigned short> indices;
	DirectX::XMFLOAT3 m_cameraPos;
	DirectX::XMMATRIX modelmat;
	//シェーダーの作成
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;//新規追加
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;//新規追加
	DirectX::SharedGraphicsResource SceneCBResource;//新規追加
	DirectX::SharedGraphicsResource metalicCB;
	DirectX::SharedGraphicsResource m_VertexBuffer;
	DirectX::SharedGraphicsResource m_IndexBuffer;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;//新規追加
};



#include "pch.h"
#include "DirectXTK12MetalicReflection.h"






/*


// DirectXTK12MetalicReflection.cpp

void DirectXTK12MetalicReflection::Draw(DirectX::GraphicsMemory* graphicsMemory, const DX::DeviceResources* DR) {

    // 1. コマンドリスト等の取得
    auto commandList = DR->GetCommandList();
    if (!commandList) return;

    ID3D12DescriptorHeap* heaps[] = { m_resourceDescriptors->Heap(), m_samplerHeap->Heap() };
    commandList->SetDescriptorHeaps(2, heaps);

    // --- 定数バッファの更新 ---

    // A. シーン定数バッファ (View/Projection)
    // ※ CreateBufferで計算した m_cameraPos などを再利用します。
    // ※ 注意: Draw内で毎回 Identity にリセットすると回転しません。
    //    回転させたい場合はメンバ変数の world 行列を使用してください。
    SceneCB sceneData;
    DirectX::XMStoreFloat4x4(&sceneData.world, worldMatrix); // ★メンバ変数などで保持している行列を使う
    DirectX::XMStoreFloat4x4(&sceneData.view, viewMatrix);   // ★CreateBuffer等で計算・保持したもの
    DirectX::XMStoreFloat4x4(&sceneData.projection, projMatrix);

    // 仮にここで計算する場合（動かない場合）:
    /*
    DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(
        DirectX::XMLoadFloat3(&m_cameraPos), // 保存されたカメラ位置を使う
        DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
    XMStoreFloat4x4(&sceneData.view, view);
    // ... world, proj も同様に設定
    

auto dynamicSceneCB = graphicsMemory->AllocateConstant(sceneData);


MaterialConstants constants;
constants.CameraPos = m_cameraPos;
constants.AlbedoColor = DirectX::XMFLOAT3(1.0f, 0.76f, 0.33f); // ゴールド
constants.Roughness = 0.2f;
constants.F0 = 1.0f; // ★重要: シェーダー修正後は 1.0 でもOKですが、強すぎる場合は 0.9 程度に

auto dynamicMaterialCB = graphicsMemory->AllocateConstant(constants);


commandList->SetGraphicsRootSignature(m_rootSignature.Get());


commandList->SetGraphicsRootConstantBufferView(RootParameterIndex::SceneBuffer, dynamicSceneCB.GpuAddress());

commandList->SetGraphicsRootConstantBufferView(RootParameterIndex::MetalicBuffer, dynamicMaterialCB.GpuAddress());

commandList->SetGraphicsRootDescriptorTable(RootParameterIndex::TextureSRV, m_resourceDescriptors->GetFirstGpuHandle());

commandList->SetGraphicsRootDescriptorTable(RootParameterIndex::TextureSampler, m_samplerHeap->GetFirstGpuHandle());

commandList->SetPipelineState(m_pipelineState.Get());
commandList->IASetIndexBuffer(&m_indexBufferView);
commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

if (!indices.empty()) {
    commandList->DrawIndexedInstanced(static_cast<UINT>(indices.size()), 1, 0, 0, 0);
}
}

*/









enum Descriptors
{
    WindowsLogo,
    CourierFont,
    ControllerFont,
    GamerPic,
    Count
};
using namespace DirectX;
HRESULT DirectXTK12MetalicReflection::CreateBuffer(DirectX::GraphicsMemory* graphicsMemory, DX::DeviceResources* deviceResources, int height, int width)
{
    
	
    constexpr int slices = 32;  // 経度方向の分割数
    constexpr int stacks = 32;  // 緯度方向の分割数
    constexpr float radius = 1.0f;

    vertices.clear();
    for (int i = 0; i <= stacks; ++i)
    {
        float phi = XM_PI * i / stacks;  // 0?π
        float y = cosf(phi);
        float r = sinf(phi);

        for (int j = 0; j <= slices; ++j)
        {
            float theta = XM_2PI * j / slices;  // 0?2π

            float x = r * cosf(theta);
            float z = r * sinf(theta);

            DirectX::VertexPositionNormal vertex = {};
            vertex.position = { x * radius, y * radius, z * radius };
            vertex.normal = { x, y, z }; // 法線は球面上の位置ベクトルと一致
            vertices.push_back(vertex);
        }
    }


    indices.clear();
    for (int i = 0; i < stacks; ++i)
    {
        for (int j = 0; j < slices; ++j)
        {
            int first = i * (slices + 1) + j;
            int second = first + slices + 1;

            // 1枚目の三角形
            indices.push_back(static_cast<uint16_t>(first));
            indices.push_back(static_cast<uint16_t>(second));
            indices.push_back(static_cast<uint16_t>(first + 1));

            // 2枚目の三角形
            indices.push_back(static_cast<uint16_t>(second));
            indices.push_back(static_cast<uint16_t>(second + 1));
            indices.push_back(static_cast<uint16_t>(first + 1));
        }
    }



    //Game.cppの方で、レンダラーの初期化をすべて終えたら同期処理を行う


    m_VertexBuffer = graphicsMemory->Allocate(sizeof(DirectX::VertexPositionNormal) * static_cast<int>(vertices.size()));
    memcpy(m_VertexBuffer.Memory(), vertices.data(), sizeof(DirectX::VertexPositionNormal) * vertices.size());


    m_IndexBuffer = graphicsMemory->Allocate(sizeof(unsigned short) * static_cast<int>(indices.size()));
    memcpy(m_IndexBuffer.Memory(), indices.data(), sizeof(unsigned short) * indices.size());
    

    //(DirectXTK12Assimpで追加)
    m_vertexBufferView.BufferLocation = m_VertexBuffer.GpuAddress();
    m_vertexBufferView.StrideInBytes = sizeof(DirectX::VertexPositionNormal);
    m_vertexBufferView.SizeInBytes = sizeof(DirectX::VertexPositionNormal) * vertices.size();

    m_indexBufferView.BufferLocation = m_IndexBuffer.GpuAddress();
    m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
    m_indexBufferView.SizeInBytes = sizeof(unsigned short) * indices.size();



    DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);

    DirectX::XMVECTOR eye = DirectX::XMVectorSet(2.0f, 2.0f, -2.0f, 0.0f);
    DirectX::XMVECTOR focus = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(eye, focus, up);




	DirectX::XMStoreFloat3(&m_cameraPos, eye);
    
    constexpr float fov = DirectX::XMConvertToRadians(45.0f);
    float aspect = float(width) / float(height);

    float    nearZ = 0.1f;
    float    farZ = 100.0f;
    DirectX::XMMATRIX projMatrix = DirectX::XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ);

    SceneCB cb;
    /*
    XMStoreFloat4x4(&cb.world, XMMatrixTranspose(worldMatrix));
    XMStoreFloat4x4(&cb.view, XMMatrixTranspose(viewMatrix));
    XMStoreFloat4x4(&cb.projection, XMMatrixTranspose(projMatrix));
    
    */
    
    // ★修正: HLSL側で row_major を指定したので、Transpose(転置)は不要です。
    // そのまま代入してください。
    XMStoreFloat4x4(&cb.world, worldMatrix);       // XMMatrixTranspose を削除
    XMStoreFloat4x4(&cb.view, viewMatrix);         // XMMatrixTranspose を削除
    XMStoreFloat4x4(&cb.projection, projMatrix);   // XMMatrixTranspose を削除auto lightDir = XMVector3Normalize(XMVectorSet(10.0f, -1.0f, 1.0f, 0.0f));
	
    
    
    MaterialConstants metal;
	metal.CameraPos = m_cameraPos;
	metal.AlbedoColor = DirectX::XMFLOAT3(1.0f, 0.76f, 0.33f); // 例: ゴールド
	metal.Roughness = 0.2f;         // 少しツヤがある
	metal.F0 = 1.0f;                // 金属なので高め

    metalicCB = graphicsMemory->AllocateConstant(metal);


    //定数バッファの作成(DIrectXTK12Assimpで追加)

    //https://github.com/microsoft/DirectXTK12/wiki/GraphicsMemory


    SceneCBResource = graphicsMemory->AllocateConstant(cb);

    //定数バッファの作成(DIrectXTK12Assimpで追加)

    //https://github.com/microsoft/DirectXTK12/wiki/GraphicsMemory
	InitializeResources(deviceResources);
    m_pipelineState = CreateGraphicsPipelineState(deviceResources, L"VertexShader.hlsl", L"PixelShader.hlsl");


    return S_OK;
}


//(DIrectXTK12Assimpで追加)
void  DirectXTK12MetalicReflection::Draw(DirectX::GraphicsMemory* graphicsMemory,const DX::DeviceResources* DR) {
	
    // --- 1. 定数バッファのデータの準備 ---
    MaterialConstants constants;
    constants.CameraPos = m_cameraPos;
    constants.AlbedoColor = DirectX::XMFLOAT3(1.0f, 0, 0); // ゴールド
    constants.Roughness = 0.05f;
    constants.F0 = 1.0f;

    // ★重要修正: 毎フレーム、新しいGPUメモリを確保してデータを書き込む
    // (以前の metalicCB ではなく、今作った dynamicCB を使う)
    auto dynamicCB = graphicsMemory->AllocateConstant(constants);
    DirectX::ResourceUploadBatch resourceUpload(DR->GetD3DDevice());
    // Draw関数の先頭に追加
    SceneCB sceneData;
    // ※CreateBufferで計算した値を保持していない場合、初期位置で表示させるために以下をセット
    DirectX::XMMATRIX world = DirectX::XMMatrixIdentity(); // 原点
    DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(
        DirectX::XMVectorSet(2.0f, 2.0f, -2.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
    DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XMConvertToRadians(45.0f),
        DR->GetOutputSize().right / (float)DR->GetOutputSize().bottom,
        0.1f, 100.0f);

    XMStoreFloat4x4(&sceneData.world, world);
    XMStoreFloat4x4(&sceneData.view, view);
    XMStoreFloat4x4(&sceneData.projection, proj);

    auto dynamicSceneCB = graphicsMemory->AllocateConstant(sceneData);
    resourceUpload.Begin();
    if (vertices.empty() || indices.empty()) {
        OutputDebugStringA("Vertices or indices buffer is empty.\n");
        return;
    }

    auto commandList = DR->GetCommandList();
    auto renderTarget = DR->GetRenderTarget();
    if (!commandList) {
        OutputDebugStringA("Command list is null.\n");
        return;
    }
	ID3D12DescriptorHeap* heaps[] = { m_resourceDescriptors->Heap(),m_samplerHeap->Heap(),};
    commandList->SetDescriptorHeaps(2,heaps);

    // 入力アセンブラー設定
    commandList->IASetIndexBuffer(&m_indexBufferView);
    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


    
    // ルートシグネチャ設定
    commandList->SetGraphicsRootSignature(m_rootSignature.Get());

   
    // ★修正: 定数バッファ (b0, b1) は GPU仮想アドレス を直接渡す
     // SceneCBResource は GraphicsMemory::AllocateConstant で確保されているので GpuAddress() を持っています
   
  // ★修正: 正しいバッファを正しいスロットにセット
    // スロット0 (SceneCB) <- 計算した dynamicSceneCB をセット (カメラ移動に対応するため)
    commandList->SetGraphicsRootConstantBufferView(RootParameterIndex::SceneBuffer, dynamicSceneCB.GpuAddress());
    // スロット1 (MetalicBuffer) <- マテリアル用 dynamicMaterialCB をセット
    commandList->SetGraphicsRootConstantBufferView(RootParameterIndex::MetalicBuffer, metalicCB.GpuAddress());
    // ★重要修正: 定数バッファ 1 (Material)
     // ここで metalicCB.GpuAddress() ではなく、さっき作った dynamicCB.GpuAddress() を渡す！
   commandList->SetGraphicsRootConstantBufferView(1, dynamicSceneCB.GpuAddress());
    commandList->SetGraphicsRootConstantBufferView(1, dynamicCB.GpuAddress());

    // ★修正: テクスチャ (t0)
    // テクスチャはヒープの「先頭(0番目)」に作りましたが、
    // ルートパラメータのインデックスとしては「2番目(TextureSRV)」に渡す必要があります。
    commandList->SetGraphicsRootDescriptorTable(2, m_resourceDescriptors->GetFirstGpuHandle());

    // サンプラー (s0)
    // ルートパラメータ「3番目」に渡す
    commandList->SetGraphicsRootDescriptorTable(3, m_samplerHeap->GetFirstGpuHandle()); // パイプラインステート設定
  
    commandList->SetPipelineState(m_pipelineState.Get());

    // 描画コール
    commandList->DrawIndexedInstanced(
        static_cast<UINT>(indices.size()), // インデックス数
        1,                                 // インスタンス数
        0,                                 // 開始インデックス
        0,                                 // 頂点オフセット
        0                                  // インスタンスオフセット
    );
    auto uploadResourcesFinished = resourceUpload.End(
        DR->GetCommandQueue());

    uploadResourcesFinished.wait();
}
void DirectXTK12MetalicReflection::InitializeResources(DX::DeviceResources* DR)
{

    auto device = DR->GetD3DDevice();


    m_states = std::make_unique<CommonStates>(device);
    // 2. デスクリプタヒープの作成
    m_resourceDescriptors = std::make_unique<DirectX::DescriptorHeap>(device,
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
        3);
       m_samplerHeap = std::make_unique<DirectX::DescriptorHeap>(device,
        D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
        D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
        1
	);
    
    // 3. テクスチャのロード (ResourceUploadBatchを使用)
       DirectX::ResourceUploadBatch resourceUpload(device);
    resourceUpload.Begin();

    // キューブマップ (.dds) をロード
    // ※TextureCubeとして作成されます
    DX::ThrowIfFailed(
       DirectX::CreateDDSTextureFromFileEx(
            device,
            resourceUpload,
           L"E:\\repos\\DirectXTK12Sphere\\DirectXTK12MetalicReflection\\earth-cubemap.dds",
            0,
            D3D12_RESOURCE_FLAG_NONE,
            DirectX::DDS_LOADER_DEFAULT,
		   m_envMapTexture.ReleaseAndGetAddressOf())
    );

   
    
    // SRV (Shader Resource View) をヒープに作成
    DirectX::CreateShaderResourceView(
        device,
        m_envMapTexture.Get(),
        m_resourceDescriptors->GetFirstCpuHandle(),
        true // isCubeMap = true (TextureCubeとして扱うために重要)
    );
    // ---------------------------------------------------------
    // ★ここを追加！: サンプラーの中身を作成する
    // ---------------------------------------------------------
    D3D12_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // 線形補間
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 繰り返し
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    samplerDesc.BorderColor[0] = 1.0f;
    samplerDesc.BorderColor[1] = 1.0f;
    samplerDesc.BorderColor[2] = 1.0f;
    samplerDesc.BorderColor[3] = 1.0f;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;

    // サンプラーヒープの先頭ハンドルに書き込む
    device->CreateSampler(&samplerDesc, m_samplerHeap->GetFirstCpuHandle());
   
    // アップロードの完了待ち
    auto uploadResourcesFinished = resourceUpload.End(DR->GetCommandQueue());
    uploadResourcesFinished.wait();
}
using Microsoft::WRL::ComPtr;
//(DIrectXTK12Assimpで追加)
// グラフィックパイプラインステートを作成する関数
Microsoft::WRL::ComPtr<ID3D12PipelineState>  DirectXTK12MetalicReflection::CreateGraphicsPipelineState(
    DX::DeviceResources* deviceresources,

    const std::wstring& vertexShaderPath,
    const std::wstring& pixelShaderPath)
{
    // シェーダーをコンパイル
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;
    ComPtr<ID3DBlob> errorBlob;
    DirectX::RenderTargetState rtState(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_D32_FLOAT);
    HRESULT hr = D3DCompileFromFile(
        vertexShaderPath.c_str(),
        nullptr,
        nullptr,
        "main", // エントリーポイント
        "vs_5_0", // シェーダーモデル
        0,
        0,
        &vertexShader,
        &errorBlob
    );

    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
        throw std::runtime_error("Failed to compile vertex shader");
    }

    hr = D3DCompileFromFile(
        pixelShaderPath.c_str(),
        nullptr,
        nullptr,
        "main",
        "ps_5_0",
        0,
        0,
        &pixelShader,
        &errorBlob
    );

    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
        throw std::runtime_error("Failed to compile pixel shader");
    }

    // 入力レイアウトを定義
    m_layout = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
         { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
    // ★修正: 定数バッファ用のRangeは不要になったので、配列サイズを 4 -> 2 に減らす
    CD3DX12_DESCRIPTOR_RANGE ranges[2] = {};

    // 定数バッファ(b0, b1)の range 定義は削除します

    // Range[0] : テクスチャ (t0) 用
    ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);

    // Range[1] : サンプラー (s0) 用
    ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);

    // ルートパラメータの作成
    CD3DX12_ROOT_PARAMETER rootParameters[4] = {};

    // ★修正: ここを DescriptorTable ではなく ConstantBufferView に変更
    // b0: SceneBuffer
    rootParameters[RootParameterIndex::SceneBuffer].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);

    // b1: MetalicBuffer
    rootParameters[RootParameterIndex::MetalicBuffer].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL);

    // t0: Texture (これはテーブルのまま。ranges[0]を使う)
    rootParameters[RootParameterIndex::TextureSRV].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);

    // s0: Sampler (これはテーブルのまま。ranges[1]を使う)
    rootParameters[RootParameterIndex::TextureSampler].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_ALL);

    // Root parameter descriptor
    CD3DX12_ROOT_SIGNATURE_DESC rsigDesc = {};
    
    // use all parameters
    rsigDesc.Init(static_cast<UINT>(std::size(rootParameters)), rootParameters, 0, nullptr, rootSignatureFlags);

    DX::ThrowIfFailed(DirectX::CreateRootSignature(deviceresources->GetD3DDevice(), &rsigDesc, m_rootSignature.ReleaseAndGetAddressOf()));

    //https://github.com/microsoft/DirectXTK12/wiki/PSOs,-Shaders,-and-Signatures
    // 
    // 
    D3D12_INPUT_LAYOUT_DESC inputlayaout = { m_layout.data(), m_layout.size() };
    DirectX::EffectPipelineStateDescription pd(
        &inputlayaout,
        DirectX::CommonStates::Opaque,
        DirectX::CommonStates::DepthDefault,
        DirectX::CommonStates::CullCounterClockwise,
        rtState);

    D3D12_SHADER_BYTECODE vertexshaderBCode = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };


    D3D12_SHADER_BYTECODE pixelShaderBCode = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };

    
    // パイプラインステートオブジェクトを作成
    ComPtr<ID3D12PipelineState> pipelineState;

    pd.CreatePipelineState(
        deviceresources->GetD3DDevice(),
        m_rootSignature.Get(),
        vertexshaderBCode,

        pixelShaderBCode,

        pipelineState.GetAddressOf()
    );
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create pipeline state");
    }

    return pipelineState;
}
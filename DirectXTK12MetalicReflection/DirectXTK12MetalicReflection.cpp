#include "pch.h"
#include "DirectXTK12MetalicReflection.h"
















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
    lambert.LightColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);    // ライトの色 (白)
    lambert.MaterialColor = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f); // マテリアルの色 (赤)
    lambert.AmbientColor = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);  // 環境光 (弱めのグレー)


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

    constexpr float fov = DirectX::XMConvertToRadians(45.0f);
    float aspect = float(width) / float(height);

    float    nearZ = 0.1f;
    float    farZ = 100.0f;
    DirectX::XMMATRIX projMatrix = DirectX::XMMatrixPerspectiveFovLH(fov, aspect, nearZ, farZ);

    SceneCB cb;
    XMStoreFloat4x4(&cb.world, XMMatrixTranspose(worldMatrix));
    XMStoreFloat4x4(&cb.view, XMMatrixTranspose(viewMatrix));
    XMStoreFloat4x4(&cb.projection, XMMatrixTranspose(projMatrix));
    auto lightDir = XMVector3Normalize(XMVectorSet(10.0f, -1.0f, 1.0f, 0.0f));
    XMStoreFloat4(&lambert.LightDir, lightDir);
    lambertCB = graphicsMemory->AllocateConstant(lambert);


    //定数バッファの作成(DIrectXTK12Assimpで追加)

    //https://github.com/microsoft/DirectXTK12/wiki/GraphicsMemory


    SceneCBResource = graphicsMemory->AllocateConstant(cb);

    //定数バッファの作成(DIrectXTK12Assimpで追加)

    //https://github.com/microsoft/DirectXTK12/wiki/GraphicsMemory

    m_pipelineState = CreateGraphicsPipelineState(deviceResources, L"VertexShader.hlsl", L"PixelShader.hlsl");


    return S_OK;
}


//(DIrectXTK12Assimpで追加)
void  DirectXTK12MetalicReflection::Draw(const DX::DeviceResources* DR) {


    DirectX::ResourceUploadBatch resourceUpload(DR->GetD3DDevice());

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

    // 入力アセンブラー設定
    commandList->IASetIndexBuffer(&m_indexBufferView);
    commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



    // ルートシグネチャ設定
    commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    //2024/12/30/9:42
    commandList->SetGraphicsRootConstantBufferView(0, SceneCBResource.GpuAddress());
    commandList->SetGraphicsRootConstantBufferView(1, lambertCB.GpuAddress());
    // パイプラインステート設定
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

    // 1. グラフィックスメモリヘルパーの初期化 (定数バッファ用)
    m_graphicsMemory = std::make_unique<DirectX::GraphicsMemory>(device);

    // 2. デスクリプタヒープの作成
    m_resourceDescriptors = std::make_unique<DirectX::DescriptorHeap>(
        device,
        Descriptors::Count
    );

    // 3. テクスチャのロード (ResourceUploadBatchを使用)
    DirectX::ResourceUploadBatch resourceUpload(device);
    resourceUpload.Begin();

    // キューブマップ (.dds) をロード
    // ※TextureCubeとして作成されます
    DX::ThrowIfFailed(
        DirectX::CreateWICTextureFromFile(
            device,
            resourceUpload,
            L"Assets/EnvironmentMap.dds", // キューブマップDDS
            m_envMapTexture.ReleaseAndGetAddressOf()
        )
    );

    // SRV (Shader Resource View) をヒープに作成
    DirectX::CreateShaderResourceView(
        device,
        m_envMapTexture.Get(),
        m_resourceDescriptors->GetCpuHandle(Descriptors::EnvMapDiff),
        true // isCubeMap = true (TextureCubeとして扱うために重要)
    );

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

    // Create root parameters and initialize first (constants)
    CD3DX12_ROOT_PARAMETER rootParameters[2] = {};
    rootParameters[RootParameterIndex::SceneBuffer].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
    rootParameters[RootParameterIndex::LambertBuffer].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL);

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
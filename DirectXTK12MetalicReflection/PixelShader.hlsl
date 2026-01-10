// PixelShader.hlsl

// 定数バッファ (マテリアル設定)
// register(b0) はルートシグネチャの設定に合わせて変更してください
cbuffer MaterialBuffer : register(b0)
{
    float3  CameraPos;    // カメラのワールド座標
    float3  AlbedoColor;  // 金属のベース色
    float   Roughness;    // 表面の粗さ (0.0: 鏡面 ～ 1.0: 曇り)
    float   F0;           // 正面反射率 (金属は1.0に近い値)
};

// テクスチャとサンプラー
TextureCube EnvMap : register(t0); // 環境マップ (キューブマップ)
SamplerState Sampler : register(s0);

// 入力構造体
// ★重要: バーテックスシェーダーの VS_OUTPUT とセマンティクス(POSITION, NORMAL等)を一致させる
struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 WorldPos : POSITION;    // VSで計算したワールド座標
    float3 Normal : NORMAL;        // VSで計算した法線
};

float4 main(PS_INPUT input) : SV_TARGET
{
    // 1. ベクトルの正規化
    // 補間された法線は長さが1ではなくなっているため再正規化必須
    float3 N = normalize(input.Normal);

// 視線ベクトル (ワールド座標での カメラ位置 - 頂点位置)
float3 V = normalize(CameraPos - input.WorldPos);

// 2. 反射ベクトルの計算
// reflect(入射ベクトル, 法線)
// 視線Vは「カメラへ向かう」定義なので、-V (カメラから来る) にして反射させる
float3 R = reflect(-V, N);

// 3. 環境マップのサンプリング
// Roughness に応じてミップマップレベル(ぼかし)を変える
// 鏡面(0.0)ならレベル0、粗い(1.0)ならレベル10(最大ボケ)など
float3 envColor = EnvMap.SampleLevel(Sampler, R, Roughness * 10.0f).rgb;

// 4. フレネル効果 (Schlickの近似式)
// 視線と法線の角度(NdotV)によって反射率を変化させる
// 輪郭部分(浅い角度)ほど、反射が強くなる現象
float NdotV = saturate(dot(N, V));
float fresnel = F0 + (1.0f - F0) * pow(1.0f - NdotV, 5.0f);

// 5. 最終カラーの合成
// 金属: 環境マップの色に、金属自体の色(Albedo)が乗算される
float3 finalColor = envColor * AlbedoColor;

// フレネル反射の適用
// 輪郭部分は素材色に関わらず、環境マップの色そのものに近づく(白っぽくなる)
finalColor = lerp(finalColor, envColor, fresnel);

return float4(finalColor, 1.0f);
}
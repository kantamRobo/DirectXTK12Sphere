// 定数バッファ (C++側から送られるデータ)
cbuffer MaterialBuffer : register(b0)
{
    float3  CameraPos;    // カメラのワールド座標
    float3  AlbedoColor;  // 金属のベース色 (金なら黄色、銅なら赤茶色など)
    float   Roughness;    // 表面の粗さ (0.0: 鏡面, 1.0: 曇りガラス)
    float   F0;           // フレネル反射率 (金属は通常高い値を持つ)
};

// テクスチャリソース
TextureCube EnvMap : register(t0); // 環境マップ (空や周囲の景色)
SamplerState Sampler : register(s0);

// 入力構造体 (バーテックスシェーダーからの出力)
struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 WorldPos : POSITION;    // ワールド空間での頂点位置
    float3 Normal : NORMAL;      // ワールド空間での法線
};

float4 main(PS_INPUT input) : SV_TARGET
{
    // 1. ベクトルの正規化
    float3 N = normalize(input.Normal);                    // 法線
    float3 V = normalize(CameraPos - input.WorldPos);      // 視線ベクトル (カメラへの向き)

    // 2. 反射ベクトルの計算
    // reflect関数は、入射ベクトルと法線から反射方向を求めます。
    // 視線ベクトルVは「カメラに向かう」ベクトルなので、-V (カメラから来る) として渡します。
    float3 R = reflect(-V, N);

    // 3. 環境マップのサンプリング (反射色を取得)
    // 粗さ(Roughness)がある場合、ミップマップレベルを上げてぼかすのが一般的です。
    // TextureCube.SampleLevel(Sampler, Vector, MipLevel) を使います。
    float3 envColor = EnvMap.SampleLevel(Sampler, R, Roughness * 10.0f).rgb;

    // 4. フレネル効果 (Schlickの近似)
    // 視線が浅い角度(輪郭部分)になるほど、反射が強くなる現象です。
    float NdotV = saturate(dot(N, V));
    float fresnel = F0 + (1.0f - F0) * pow(1.0f - NdotV, 5.0f);

    // 5. 最終カラーの合成
    // 金属の特徴: 環境の反射色に、金属自体の色(Albedo)を乗算します。
    // ※ 非金属(プラスチック等)の場合は、ここでEnvColor * Fresnel + DiffuseColor となりますが、
    //    金属はDiffuse(拡散反射)がほぼ0のため、反射光自体に色がつきます。
    float3 finalColor = envColor * AlbedoColor;

    // フレネル項を適用して、輪郭を少し強調・補正する (簡易的なPBR的アプローチ)
    finalColor = lerp(finalColor, envColor, fresnel);

    return float4(finalColor, 1.0f);
}


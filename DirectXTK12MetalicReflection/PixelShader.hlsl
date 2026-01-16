// PixelShader.hlsl
cbuffer MaterialBuffer : register(b1)
{
    float3  CameraPos;
    float3  AlbedoColor;
    float   Roughness;
    float   F0;
};

TextureCube EnvMap : register(t0);
SamplerState Sampler : register(s0);

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 WorldPos : POSITION;
    float3 Normal : NORMAL;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 N = normalize(input.Normal);
    float3 V = normalize(CameraPos - input.WorldPos);
    float3 R = reflect(-V, N);

    // 環境マップのサンプリング
    float3 envColor = EnvMap.SampleLevel(Sampler, R, Roughness * 10.0f).rgb;

    // --- 修正箇所: 金属反射の計算 ---

    // 1. フレネル項の計算
    float NdotV = saturate(dot(N, V));
    // F0項は「正面反射率」ですが、金属の場合はこれを「反射の色」への影響として扱います。
    // ここでは単純化して、輪郭部分（Grazing Angle）だけ白っぽくなるようにします。
    float fresnel = pow(1.0f - NdotV, 5.0f);

    // 2. ベースの反射色（金属はここで着色される）
    float3 baseReflection = envColor * AlbedoColor;

    // 3. 輪郭の反射色（角度がつくと、素材色に関わらず環境光そのままの色に近づく）
    // F0パラメータを「金属っぽさの強さ」として使い、blendに利用する形に修正
    // または、単純にSchlick近似で「白」へ遷移させます。

    // 修正案: 「ゴールド色の反射」に「白い輪郭反射」を合成
    float3 finalColor = lerp(baseReflection, envColor, fresnel * (1.0 - F0));
    // ※F0が高い(1.0)なら、全面がAlbedoColorになります。
    // ※今回はシンプルに以下を採用してください（確実に金色になります）

    finalColor = baseReflection + (envColor * fresnel * 0.5f);

    return float4(finalColor, 1.0f);
}
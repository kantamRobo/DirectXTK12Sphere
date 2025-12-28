// 定数バッファ (C++側から送られる設定データ)
cbuffer LambertBuffer : register(b1)
{
    float3 LightDir;       // ライトの進行方向 (例: 上から下なら 0, -1, 0)
    float3 LightColor;     // ライトの色と強さ (例: 1.0, 1.0, 1.0)
    float3 MaterialColor;  // 物体の色 (アルベド)
    float3 AmbientColor;   // 環境光 (真っ黒になるのを防ぐための底上げ用の光)
};

// 入力構造体 (バーテックスシェーダーからの出力)
struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;      // ワールド空間での法線
};

float4 main(PS_INPUT input) : SV_TARGET
{
    // 1. ベクトルの正規化 (重要)
    // 補間された法線は長さが1ではなくなっている可能性があるため、必ず正規化します。
    float3 N = normalize(input.Normal);

// 2. ライトベクトルの準備
// ランバート計算では「表面からライトに向かうベクトル(L)」が必要です。
// 定数バッファのLightDirが「光の進行方向」の場合、逆ベクトルにします。
float3 L = normalize(-LightDir);

// 3. ランバート拡散反射の計算 (Lambert Diffuse)
// 内積 (dot) の結果が 1.0 なら正面、0.0 なら真横、マイナスなら裏側です。
// 裏側からの光は計算しないため、saturate で 0.0 ～ 1.0 にクランプします。
// (max(0, dot(N, L)) と同じ意味です)
float diffuseIntensity = saturate(dot(N, L));

// 4. 最終カラーの合成
// Diffuse = (光の色 * 物体の色) * 強度
float3 diffuse = (LightColor * MaterialColor) * diffuseIntensity;

// 環境光 (Ambient) を足して、影の部分が真っ黒になるのを防ぐ
float3 ambient = AmbientColor * MaterialColor;

return float4(diffuse + ambient, 1.0f);
}
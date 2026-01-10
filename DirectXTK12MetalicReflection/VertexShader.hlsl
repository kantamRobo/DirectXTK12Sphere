struct VS_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
};

cbuffer ConstantBuffer : register(b0)
{
    // ★修正1: C++側(DirectXMath)とメモリ配置を合わせるため row_major を明記
    row_major float4x4 World;
    row_major float4x4 View;
    row_major float4x4 Projection;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 WorldPos : POSITION;
    float3 normal : NORMAL;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT o;

    float4 p = float4(input.pos, 1.0f);

    // ★修正2: DirectXの標準は「ベクトル × 行列」の順序 (mul(p, World))
    float4 wp = mul(p, World);
    float4 vp = mul(wp, View);
    o.pos = mul(vp, Projection);

    // ★修正3: ここを追加（これがないとピクセルシェーダーに座標が渡りません）
    o.WorldPos = wp.xyz;

    // 法線の計算（回転成分のみ適用）
    o.normal = mul(input.normal, (float3x3)World);

    return o;
}

/*
// 2. メイン関数の修正 (デバッグ用: 座標変換なし)
VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT o;

    // --- 座標変換をスキップ ---
    // 入力されたモデルの頂点座標(input.pos)を、そのまま画面座標(o.pos)として使います。
    // モデルのサイズが「半径1.0」くらいなら、画面中央にシルエットが映るはずです。

    // 【重要】Zを0.5に固定し、Wを1.0に固定します。
    // これで「カメラの目の前」に強制的に配置されます。
    o.pos = float4(input.pos.x, input.pos.y, 0.5f, 1.0f);


    // --- ピクセルシェーダーへのダミー渡し ---
    // ここで値を渡さないとリンクエラーになるため、
    // 変換なしの値をそのまま渡しておきます。
    // (照明計算は正しく行われませんが、色は出るはずです)
    o.WorldPos = input.pos;
    o.normal = input.normal; // 法線もそのまま

    return o;
}

*/
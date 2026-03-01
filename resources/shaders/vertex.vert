struct Input {
    float2 Position : POSITION;
    float3 Color    : TEXCOORD0;
};

struct Output {
    float4 Position : SV_Position;
    float4 Color    : TEXCOORD0;
};

cbuffer Transforms : register(b0, space1)
{
    float4x4 Camera;
    float4x4 Model;
};

Output main(Input input) {
    Output output;
    output.Position = mul(Camera, mul(Model, float4(input.Position, 0.0f, 1.0f)));
    output.Color    = float4(input.Color, 1.0f);
    return output;
}
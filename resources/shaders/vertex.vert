struct Input {
    float4 Color : TEXCOORD0;
    float3 Position : TEXCOORD1;
    float3 Normal : TEXCOORD2;
    float3 Tangent : TEXCOORD3;
    float3 Bitangent : TEXCOORD4;
    float2 TextureCoordinates : TEXCOORD5;
};

struct Output {
    float4 Position : SV_Position;
    float2 TextureCoordinates : TEXCOORD0;
};

cbuffer Transforms : register(b0, space1)
{
    float4x4 Camera;
    float4x4 Model;
};

Output main(Input input) {
    Output output;
    output.Position = mul(Camera, mul(Model, float4(input.Position, 1.0f)));
    output.TextureCoordinates = input.TextureCoordinates;
    return output;
}
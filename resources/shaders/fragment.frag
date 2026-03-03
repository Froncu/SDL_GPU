struct Input {
   float2 TextureCoordinates : TEXCOORD0;
};

Texture2DArray BaseColors : register(t0, space2);
SamplerState Sampler : register(s0, space2);
cbuffer Material : register(b0, space3)
{
   int BaseColorIndex;
};

float4 main(Input input) : SV_Target0
{
    return BaseColors.Sample(Sampler, float3(input.TextureCoordinates, float(BaseColorIndex)));
}
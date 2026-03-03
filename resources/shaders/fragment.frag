struct Input {
   float2 TextureCoordinates : TEXCOORD0;
};

Texture2D Texture : register(t0, space2);
SamplerState Sampler : register(s0, space2);

float4 main(Input input) : SV_Target0
{
    return Texture.Sample(Sampler, input.TextureCoordinates);
}
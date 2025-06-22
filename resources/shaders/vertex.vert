struct Input {
    uint VertexIndex : SV_VertexID;
};

struct Output {
    float4 Position : SV_Position;
    float4 Color : TEXCOORD0;
};

static const float2 POSITIONS[3] = {
    { -0.5f, -0.5f },
    { 0.5f, -0.5f },
    { 0.0f, 0.5f }
};

static const float3 COLORS[3] = {
    { 1.0f, 0.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f }
};

Output main(Input input) {
    Output output;
    output.Position = float4(POSITIONS[input.VertexIndex], 0.0f, 1.0f);
    output.Color = float4(COLORS[input.VertexIndex], 1.0f);
    return output;
}
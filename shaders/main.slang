struct VSOutput {
    float4 pos : SV_Position;
    float2 uv  : TEXCOORD0;
};

[shader("vertex")]
VSOutput vertMain(uint vertexId : SV_VertexID) {
    VSOutput output;

    float2 positions[3] = {
          float2(-1.0, -1.0),
          float2( 3.0, -1.0),
          float2(-1.0,  3.0)
    };

    float2 uvs[3] = {
          float2(0.0, 0.0),
          float2(2.0, 0.0),
          float2(0.0, 2.0)
    };

    output.pos = float4(positions[vertexId], 0.0, 1.0);
    output.uv = uvs[vertexId];

    return output;
}

[[vk::binding(0, 0)]]
Sampler2D pixels;

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_TARGET {
    return pixels.Sample(input.uv);
}

[[vk::binding(0, 0)]]
RWTexture2D<float4> pixelsOut;

struct PushConstants {
    uint width;
    uint height;
};

[[vk::push_constant]]
PushConstants pc;

[shader("compute")]
[numthreads(8,8,1)]
void compMain(uint3 threadId : SV_DispatchThreadID) {
    uint2 pixel = threadId.xy;

    if (pixel.x >= pc.width || pixel.y >= pc.height)
        return;

    float r = float(pixel.x) / float(pc.width - 1);
    float g = float(pixel.y) / float(pc.height - 1);
    float b = 0.0;

    pixelsOut[pixel] = float4(r, g, b, 1.0);
}
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

[[vk::binding(1, 0)]]
Sampler2D pixels;

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_TARGET {
    return pixels.Sample(input.uv);
}

import ray;
import hitRecord;
import camera;
import random;
import color;
import material;
import scene;

struct PushConstants {
    uint64_t bufferAddress;
    uint32_t currentTile;
    float4 resolution;
    float4 camCenter;
    float4 camFront;
    float4 camRight;
    float4 camUp;
};

[[vk::push_constant]]
PushConstants pc;

[[vk::binding(0, 0)]]
RWTexture2D<float4> pixelsOut;

static constexpr float FLT_MAX = 3.402823466e+38f;
static constexpr int MAX_DEPTH = 40;
static constexpr float DEFOCUS_ANGLE = 0.0;
static constexpr float FOCUS_DIST = 1.0;
static constexpr float VFOV = 40.0;
static constexpr int SAMPLES_PER_PIXEL = 500;
static constexpr float PIXEL_SAMPLES_SCALE = 1.0 / SAMPLES_PER_PIXEL;
static constexpr float3 BGCOLOR = float3(0.1);
static constexpr uint TILE_SIZE = 8;

float3 rayColor(Ray ray, int depth, Scene scene, inout Random rand) {
    Ray r = ray;
    float3 throughput = float3(1.0);
    float3 radiance = float3(0.0);

    for (uint bounce = 0; bounce < depth; ++bounce) {
        HitRecord rec;

        if (!scene.hit(r, float2(0.001, FLT_MAX), rec, rand)) {
            radiance += throughput * BGCOLOR;
            return radiance;
        }

        Ray scattered;
        float3 matAtten;
        float3 emission = emitted(rec);

        radiance += throughput * emission;

        if (!scatter(r, rec, matAtten, scattered, rand)) {
            return radiance;
        }
        
        throughput *= matAtten;
        r = scattered; 
    }

    return radiance;
}

[shader("compute")]
[numthreads(8,8,1)]
void compMain(uint3 groupId : SV_GroupID, uint3 threadId : SV_GroupThreadID) {
    uint width = (uint)pc.resolution.x;
    uint height = (uint)pc.resolution.y;

    const uint tilesX = (width + TILE_SIZE - 1) / TILE_SIZE;
    uint tileIndex = pc.currentTile + groupId.x;

    uint tileX = tileIndex % tilesX;
    uint tileY = tileIndex / tilesX;

    uint x = tileX * 8 + threadId.x;
    uint y = tileY * 8 + threadId.y;

    if (x >= width || y >= height)
        return;

    Camera camera = {width, height, VFOV, DEFOCUS_ANGLE, FOCUS_DIST};
    Scene scene = {pc.bufferAddress};

    uint seed = x + y * width + width * height + 1;
    Random random = {seed};

    float3 pixColor = float3(0.0);
    for (int i = 0; i < SAMPLES_PER_PIXEL; ++i) {
        Ray ray = camera.getRay(x, y, pc.camCenter.xyz, pc.camFront.xyz, pc.camRight.xyz, pc.camUp.xyz, random);
        pixColor += rayColor(ray, MAX_DEPTH, scene, random);
    }

    float3 color = pixColor * PIXEL_SAMPLES_SCALE;
    pixelsOut[uint2(x, y)] = float4(linear2gamma(color), 1.0);
}
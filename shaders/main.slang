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

[[vk::binding(2, 0)]]
Sampler2D pixels;

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_TARGET {
    return pixels.Sample(input.uv);
}

import ray;
import sphere;
import hitRecord;
import camera;
import random;
import color;
import material;
import scene;

struct PushConstants {
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

[[vk::binding(1, 0)]]
StructuredBuffer<SphereData> sphereDatas;

static constexpr float FLT_MAX = 3.402823466e+38f;
static constexpr int MAX_DEPTH = 5;
static constexpr float DEFOCUS_ANGLE = 0.0;
static constexpr float FOCUS_DIST = 1.0;
static constexpr float VFOV = 90.0;
static constexpr int SAMPLES_PER_PIXEL = 20;
static constexpr float PIXEL_SAMPLES_SCALE = 1.0 / SAMPLES_PER_PIXEL;

float3 rayColor(Ray ray, int depth, Scene scene, inout Random rand) {
    Ray r = ray;
    float3 attenuation = float3(1.0);

    for (uint bounce = 0; bounce < depth; ++bounce) {
        HitRecord rec;

        if (scene.hit(r, float2(0.001, FLT_MAX), rec)) {
            Ray scattered;
            float3 matAtten;

            if (scatter(r, rec, matAtten, scattered, rand)) {
                attenuation *= matAtten;
                r = scattered;
            } else {
                return float3(0.0);
            }
        } else {
            float3 unitDir = normalize(r.direction);
            float a = 0.5 * (unitDir.y + 1.0);
            return attenuation * lerp(float3(1.0, 1.0, 1.0), float3(0.5, 0.7, 1.0), a);
        }
    }

    return float3(0.0);
}

[shader("compute")]
[numthreads(10,10,1)]
void compMain(uint3 threadId : SV_DispatchThreadID) {
    uint2 pixel = threadId.xy;
    uint width = (uint)pc.resolution.x;
    uint height = (uint)pc.resolution.y;

    if (pixel.x >= width || pixel.y >= height)
        return;

    Camera camera = {width, height, VFOV, DEFOCUS_ANGLE, FOCUS_DIST};
    Scene scene = {sphereDatas};

    uint seed = pixel.x + pixel.y * width + width * height + 1;
    Random random = {seed};

    float3 pixColor = float3(0.0);
    for (int i = 0; i < SAMPLES_PER_PIXEL; ++i) {
        Ray ray = camera.getRay(pixel.x, pixel.y, pc.camCenter.xyz, pc.camFront.xyz, pc.camRight.xyz, pc.camUp.xyz, random);
        pixColor += rayColor(ray, MAX_DEPTH, scene, random);
    }

    float3 color = pixColor * PIXEL_SAMPLES_SCALE;
    pixelsOut[pixel] = float4(linear2gamma(color), 1.0);
}
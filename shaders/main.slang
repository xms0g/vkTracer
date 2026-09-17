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

struct PushConstants {
    uint width;
    uint height;
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
StructuredBuffer<Sphere> spheres;

static constexpr int MAX_DEPTH = 5;
static constexpr int SAMPLES_PER_PIXEL = 50;
static constexpr float PIXEL_SAMPLES_SCALE = 1.0 / SAMPLES_PER_PIXEL;

bool hitAnything(Ray ray, constexpr float tmin, constexpr float tmax, inout HitRecord rec) {
    bool hitAnything = false;
    float closestSoFar = tmax;

    for (uint i = 0; i < spheres.getCount(); ++i) {
        Sphere sphere = spheres[i];

        if (sphere.hit(ray, tmin, closestSoFar, rec)) {
            hitAnything = true;
            closestSoFar = rec.interval;
        }
    }

    return hitAnything;
}

float3 rayColor(Ray ray, int sampleIdx, int depth) {
    Ray scatteredRay = ray;
    float3 attenuation = float3(1.0);
    uint seed = pc.width + pc.width * pc.height * sampleIdx + 1;

    for (uint bounce = 0; bounce < depth; ++bounce) {
        HitRecord rec;

        if (hitAnything(scatteredRay, 0.001, 1000.0, rec)) {
            float3 dir = randomOnHemisphere(rec.normal, seed++);

            scatteredRay = {rec.point, dir};
            attenuation *= 0.5;
        } else {
            float3 unitDir = normalize(ray.direction);
            float a = 0.5 * (unitDir.y + 1.0);
            return attenuation * lerp(float3(1.0, 1.0, 1.0), float3(0.0, 0.7, 1.0), a);
        }
    }

    return float3(0);
}

[shader("compute")]
[numthreads(8,8,1)]
void compMain(uint3 threadId : SV_DispatchThreadID) {
    uint2 pixel = threadId.xy;

    if (pixel.x >= pc.width || pixel.y >= pc.height)
        return;

    Camera camera = {pc.width, pc.height};

    float3 pixColor = float3(0.0);
    for (int i = 0; i < SAMPLES_PER_PIXEL; ++i) {
        Ray ray = camera.getRay(pixel.x, pixel.y, i, pc.camCenter, pc.camFront, pc.camRight, pc.camUp);
        pixColor += rayColor(ray, i, MAX_DEPTH);
    }

    pixelsOut[pixel] = float4(pixColor * PIXEL_SAMPLES_SCALE, 1.0);
}
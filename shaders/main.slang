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

struct PushConstants {
    uint width;
    uint height;
    uint sphereCount;
};

[[vk::push_constant]]
PushConstants pc;

[[vk::binding(0, 0)]]
RWTexture2D<float4> pixelsOut;

[[vk::binding(1, 0)]]
StructuredBuffer<Sphere> spheres;

bool hitAnything(Ray ray, float tmin, float tmax, inout HitRecord rec) {
    bool hitAnything = false;
    float closestSoFar = tmax;

    for (uint i = 0; i < pc.sphereCount; ++i) {
        Sphere sphere = spheres[i];

        if (sphere.hit(ray, tmin, closestSoFar, rec)) {
            hitAnything = true;
            closestSoFar = rec.interval;
        }
    }

    return hitAnything;
}

float3 rayColor(Ray ray) {
    HitRecord rec;

    if (hitAnything(ray, 0, 1000, rec)) {
        return 0.5 * (rec.normal + float3(1,1,1));
    }

    float3 unitDir = normalize(ray.direction);
    float a = 0.5 * (unitDir.y + 1.0);
    return lerp(float3(1.0, 1.0, 1.0), float3(0.0, 0.7, 1.0), a);
}

[shader("compute")]
[numthreads(8,8,1)]
void compMain(uint3 threadId : SV_DispatchThreadID) {
    uint2 pixel = threadId.xy;

    if (pixel.x >= pc.width || pixel.y >= pc.height)
        return;

    // Camera
    float focal = 1.0;
    float viewportHeight = 2.0;
    float viewportWidth = viewportHeight * (float(pc.width)/pc.height);
    float3 cameraCenter = float3(0, 0, 0);

    float3 viewportU = float3(viewportWidth, 0, 0);
    float3 viewportV = float3(0, -viewportHeight, 0);

    float3 pixDeltaU = viewportU / pc.width;
    float3 pixDeltaV = viewportV / pc.height;

    float3 viewportUpperLeft = cameraCenter - float3(0, 0, focal) - viewportU/2 - viewportV/2;
    float3 pixel00Loc = viewportUpperLeft + 0.5 * (pixDeltaU + pixDeltaV);

    float3 pixelCenter = pixel00Loc + (pixel.x * pixDeltaU) + (pixel.y * pixDeltaV);
    float3 rayDir = pixelCenter - cameraCenter;

    Ray r = {cameraCenter, rayDir};

    float3 pixColor = rayColor(r);

    pixelsOut[pixel] = float4((float3)pixColor, 1.0);
}
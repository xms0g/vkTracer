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

[[vk::binding(0, 0)]]
RWTexture2D<float4> pixelsOut;

struct PushConstants {
    uint width;
    uint height;
};

[[vk::push_constant]]
PushConstants pc;

struct Ray {
    float3 orig;
    float3 dir;

    __init() {}
    __init(float3 o, float3 d) {
        orig = o;
        dir = d;
    }

    property float3 origin    { get { return orig; } }
    property float3 direction { get { return dir; } }

    float3 at(float t) {
        return orig + t * dir;
    }
};

float hitSphere(float3 center, float radius, Ray ray) {
    float3 oc = center - ray.origin;
    float a = dot(ray.direction, ray.direction);
    float h = dot(ray.direction, oc);
    float c = dot(oc, oc) - radius * radius;
    float discriminant = h * h - a * c;

    if (discriminant < 0) {
        return -1.0;
    } else {
        return (h - sqrt(discriminant)) / a;
    }
}

float3 rayColor(Ray ray) {
    float t = hitSphere(float3(0, 0, -1), 0.5, ray);
    if (t > 0.0) {
        float3 N = normalize(ray.at(t) - float3(0, 0, -1));
        return 0.5 * float3(N.x+1, N.y+1, N.z+1);
    }

    float3 unitDir = normalize(ray.direction);
    float a = 0.5 * (unitDir.y + 1.0);
    return (1.0 - a) * float3(1.0, 1.0, 1.0) + a * float3(0.0, 0.7, 1.0);
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
module camera;
import ray;
import random;

public struct Camera {
    private uint width;
    private uint height;
    private float viewportHeight;
    private float viewportWidth;

    public __init (uint width, uint height, float vfov) {
        this.width = width;
        this.height = height;

        float theta = radians(vfov);
        float h = tan(theta / 2);
        this.viewportHeight = 2.0 * h;
        this.viewportWidth = viewportHeight * (float(width)/height);
    }

    public Ray getRay(int x, int y, float4 center, float4 front, float4 right, float4 up, inout Random random) {
        float3 u = right.xyz;
        float3 v = up.xyz;
        float3 w = front.xyz;
        float3 viewportU = u * viewportWidth;
        float3 viewportV = -v * viewportHeight;

        float3 pixDeltaU = viewportU / width;
        float3 pixDeltaV = viewportV / height;

        float3 viewportUpperLeft = center.xyz + w - viewportU/2 - viewportV/2;

        float3 pixel00Loc = viewportUpperLeft + 0.5 * (pixDeltaU + pixDeltaV);

        float3 offset = random.sampleSquare();
        float3 pixelSample = pixel00Loc + (pixDeltaU * (x + offset.x)) + (pixDeltaV * (y + offset.y));

        float3 rayOrig = center.xyz;
        float3 rayDir = normalize(pixelSample - rayOrig);
        return {rayOrig, rayDir};
    }
};
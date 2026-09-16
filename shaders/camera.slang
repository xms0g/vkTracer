module camera;
import ray;

public struct Camera {
    private uint width;
    private uint height;
    private float focal;
    private float viewportHeight;
    private float viewportWidth;

    public __init (uint width, uint height) {
        this.focal = 1.0;
        this.width = width;
        this.height = height;
        this.viewportHeight = 2.0;
        this.viewportWidth = viewportHeight * (float(width)/height);
    }

    public Ray getRay(int x, int y, float4 center,  float4 front, float4 right, float4 up) {
        float3 viewportU = right.xyz * viewportWidth;
        float3 viewportV = up.xyz * -viewportHeight;
        float3 pixDeltaU = viewportU / width;
        float3 pixDeltaV = viewportV / height;
        float3 viewportCenter = center.xyz + front.xyz * focal;
        float3 viewportUpperLeft = viewportCenter - viewportU/2 - viewportV/2;
        float3 pixel00Loc = viewportUpperLeft + 0.5 * (pixDeltaU + pixDeltaV);
        float3 pixelCenter = pixel00Loc + (x * pixDeltaU) + (y * pixDeltaV);
        float3 rayDir = pixelCenter - center.xyz;

        return {center.xyz, rayDir};
    }

};
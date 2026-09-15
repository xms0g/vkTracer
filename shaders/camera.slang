module camera;
import ray;

public struct Camera {
    private float3 center;
    private float3 pixel00Loc;
    private float3 pixDeltaU;
    private float3 pixDeltaV;

    public __init (uint width, uint height, float3 c) {
        center = c;
        float focal = 1.0;
        float viewportHeight = 2.0;
        float viewportWidth = viewportHeight * (float(width)/height);

        float3 viewportU = float3(viewportWidth, 0, 0);
        float3 viewportV = float3(0, -viewportHeight, 0);

        pixDeltaU = viewportU / width;
        pixDeltaV = viewportV / height;

        float3 viewportUpperLeft = center - float3(0, 0, focal) - viewportU/2 - viewportV/2;
        pixel00Loc = viewportUpperLeft + 0.5 * (pixDeltaU + pixDeltaV);
    }

    public Ray getRay(int x, int y) {
        float3 pixelCenter = pixel00Loc + (x * pixDeltaU) + (y * pixDeltaV);
        float3 rayDir = pixelCenter - center;

        return {center, rayDir};
    }

};
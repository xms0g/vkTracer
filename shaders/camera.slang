module camera;
import ray;

public struct Camera {
    private float focal;
    private float3 viewportU;
    private float3 viewportV;
    private float3 pixDeltaU;
    private float3 pixDeltaV;

    public __init (uint width, uint height) {
        focal = 1.0;
        float viewportHeight = 2.0;
        float viewportWidth = viewportHeight * (float(width)/height);

        viewportU = float3(viewportWidth, 0, 0);
        viewportV = float3(0, -viewportHeight, 0);

        pixDeltaU = viewportU / width;
        pixDeltaV = viewportV / height;
    }

    public Ray getRay(int x, int y, float3 center) {
        float3 viewportUpperLeft = center - float3(0, 0, focal) - viewportU/2 - viewportV/2;
        float3 pixel00Loc = viewportUpperLeft + 0.5 * (pixDeltaU + pixDeltaV);
        float3 pixelCenter = pixel00Loc + (x * pixDeltaU) + (y * pixDeltaV);
        float3 rayDir = pixelCenter - center;

        return {center, rayDir};
    }

};
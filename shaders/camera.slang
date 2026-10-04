module camera;
import ray;
import random;
import config;

public struct Camera {
    private float3 center;
    private float3 front;
    private float3 right;
    private float3 up;
    private uint width;
    private uint height;
    private float3 defocusDiskU;
    private float3 defocusDiskV;
    private float viewportHeight;
    private float viewportWidth;

    public __init(float3 center, float3 front, float3 right, float3 up, uint width, uint height) {
        this.center = center;
        this.front = front;
        this.right = right;
        this.up = up;
        this.width = width;
        this.height = height;
        float theta = radians(VFOV);
        float h = tan(theta / 2);
        this.viewportHeight = 2.0 * h * FOCUS_DIST;
        this.viewportWidth = viewportHeight * (float(width)/height);
    }

    [mutating]
    public Ray getRay(int x, int y, int si, int sj, inout Random rand) {
        float3 viewportU = right * viewportWidth;
        float3 viewportV = -up * viewportHeight;

        float3 pixDeltaU = viewportU / width;
        float3 pixDeltaV = viewportV / height;

        float3 viewportUpperLeft = center + (FOCUS_DIST * front) - viewportU/2 - viewportV/2;

        float3 pixel00Loc = viewportUpperLeft + 0.5 * (pixDeltaU + pixDeltaV);

        float defocusRadius = FOCUS_DIST * tan(radians(DEFOCUS_ANGLE / 2));
        defocusDiskU = right * defocusRadius;
        defocusDiskV = up * defocusRadius;

        float3 offset = rand.sampleSquareStratified(si, sj, RECIP_SQRT_SPP);
        float3 pixelSample = pixel00Loc + (pixDeltaU * (x + offset.x)) + (pixDeltaV * (y + offset.y));

        float3 rayOrig = DEFOCUS_ANGLE <= 0 ? center : defocusDiskSample(center, rand);
        float3 rayDir = normalize(pixelSample - rayOrig);
        float rayTime = rand.random();
        return {rayOrig, rayDir, rayTime};
    }

    float3 defocusDiskSample(float3 center, inout Random rand) {
        // Returns a random point in the camera defocus disk.
        float3 p = rand.randomInUnitDisk();
        return center + (p[0] * defocusDiskU) + (p[1] * defocusDiskV);
    }
};
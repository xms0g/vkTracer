module camera;
import ray;
import random;

public struct Camera {
    private uint width;
    private uint height;
    private float defocusAngle;
    private float focusDist;
    private float3 defocusDiskU;
    private float3 defocusDiskV;
    private float viewportHeight;
    private float viewportWidth;

    public __init (uint width, uint height, float vfov, float defocusAngle, float focusDist) {
        this.width = width;
        this.height = height;
        this.defocusAngle = defocusAngle;
        this.focusDist = focusDist;
        float theta = radians(vfov);
        float h = tan(theta / 2);
        this.viewportHeight = 2.0 * h * focusDist;
        this.viewportWidth = viewportHeight * (float(width)/height);
    }

    [mutating]
    public Ray getRay(int x, int y, float4 center, float4 front, float4 right, float4 up, inout Random rand) {
        float3 u = right.xyz;
        float3 v = up.xyz;
        float3 w = front.xyz;
        float3 viewportU = u * viewportWidth;
        float3 viewportV = -v * viewportHeight;

        float3 pixDeltaU = viewportU / width;
        float3 pixDeltaV = viewportV / height;

        float3 viewportUpperLeft = center.xyz + (focusDist * w) - viewportU/2 - viewportV/2;

        float3 pixel00Loc = viewportUpperLeft + 0.5 * (pixDeltaU + pixDeltaV);

        float defocusRadius = focusDist * tan(radians(defocusAngle / 2));
        defocusDiskU = u * defocusRadius;
        defocusDiskV = v * defocusRadius;

        float3 offset = rand.sampleSquare();
        float3 pixelSample = pixel00Loc + (pixDeltaU * (x + offset.x)) + (pixDeltaV * (y + offset.y));

        float3 rayOrig = defocusAngle <= 0 ? center.xyz : defocusDiskSample(center.xyz, rand);
        float3 rayDir = normalize(pixelSample - rayOrig);
        return {rayOrig, rayDir};
    }

    float3 defocusDiskSample(float3 center, inout Random rand) {
        // Returns a random point in the camera defocus disk.
        float3 p = rand.randomInUnitDisk();
        return center + (p[0] * defocusDiskU) + (p[1] * defocusDiskV);
    }
};
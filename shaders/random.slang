module random;
__include "rng";

public struct Random {
    private RNG rng;

    public __init(uint seed) {
        rng = {seed};
    }

    [mutating]
    public float random() {
        return rng.nextFloat();
    }

    [mutating]
    float random(float min, float max) {
        return min + (max - min) * rng.nextFloat();
    }

    [mutating]
    public float3 randomUnitVector() {
        for(;;) {
            float3 p = float3(random(-1, 1), random(-1, 1), random(-1, 1));
            float lensq = dot(p, p);

            if (1e-8f < lensq && lensq <= 1) {
                return normalize(p);
            }
        }
    }

    [mutating]
    public float3 randomInUnitDisk() {
        for(;;) {
            float3 p = float3(random(-1, 1), random(-1, 1), 0.0);
            float lensq = dot(p, p);

            if (lensq < 1) {
                return p;
            }
        }
    }

    [mutating]
    public float3 randomOnHemisphere(const float3 N) {
        float3 onUnitSphere = randomUnitVector();

        if (dot(onUnitSphere, N) > 0.0) {
            return onUnitSphere;
        }

        return -onUnitSphere;
    }

    [mutating]
    public float3 sampleSquare() {
        return float3(random() - 0.5, random() - 0.5, 0.0);
    }
};
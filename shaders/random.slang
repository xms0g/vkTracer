module random;
__include "rng";

float random(inout RNG rng) {
    return rng.nextFloat();
}

float random(float min, float max, inout RNG rng) {
    return min + (max - min) * random(rng);
}

float3 randomUnitVector(inout RNG rng) {
    for(;;) {
        float3 p = float3(random(-1, 1, rng), random(-1, 1, rng), random(-1, 1, rng));
        float lensq = dot(p, p);

        if (1e-8f < lensq && lensq <= 1) {
            return normalize(p);
        }
    }
}

public float3 randomOnHemisphere(const float3 N, uint seed) {
    RNG rng = {seed};
    float3 onUnitSphere = randomUnitVector(rng);

    if (dot(onUnitSphere, N) > 0.0) {
        return onUnitSphere;
    }

    return -onUnitSphere;
}

public float3 sampleSquare(uint seed) {
    RNG rng = {seed};
    return float3(rng.nextFloat() - 0.5, rng.nextFloat() - 0.5, 0.0);
}


module random;
__include "rng";

float random(uint seed) {
    RNG rng = {seed};
    return rng.nextFloat();
}

float random(float min, float max, uint seed) {
    return min + (max - min) * random(seed);
}

public float3 randomUnitVector(uint seed) {
    RNG rng = {seed};

    for(;;) {
        float3 p = float3(
            rng.nextFloat() * 2.0 - 1.0,
            rng.nextFloat() * 2.0 - 1.0,
            rng.nextFloat() * 2.0 - 1.0);

        float lensq = dot(p, p);

        if (1e-8f < lensq && lensq <= 1) {
            return normalize(p);
        }
    }
}

public float3 randomOnHemisphere(const float3 N, uint seed) {
    float3 onUnitSphere = randomUnitVector(seed);

    if (dot(onUnitSphere, N) > 0.0) {
        return onUnitSphere;
    }

    return -onUnitSphere;
}

public float3 sampleSquare(uint seed) {
    RNG rng = {seed};
    return float3(rng.nextFloat() - 0.5, rng.nextFloat() - 0.5, 0.0);
}


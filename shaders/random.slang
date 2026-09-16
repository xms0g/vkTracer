module random;
__include "rng";

public float3 sampleSquare(uint seed) {
    RNG rng = {seed};
    return float3(rng.nextFloat() - 0.5, rng.nextFloat() - 0.5, 0.0);
}
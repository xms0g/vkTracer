module color;

public float3 linear2gamma(float3 linearColor) {
    return sqrt(max(linearColor, 0.0));
}
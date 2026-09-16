module ray;

public struct Ray {
    private float3 orig;
    private float3 dir;

    public __init(float3 o, float3 d) {
        orig = o;
        dir = d;
    }

    public property float3 origin    { get { return orig; } }
    public property float3 direction { get { return dir; } }

    public float3 at(float t) {
        return orig + t * dir;
    }
};
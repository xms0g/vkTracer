module ray;

public struct Ray {
    private float3 orig;
    private float3 dir;
    private float tm;

    public __init(float3 orig, float3 dir) {
        this.orig = orig;
        this.dir = dir;
        this.tm = 0.0;
    }

    public __init(float3 orig, float3 dir, float tm) {
        this.orig = orig;
        this.dir = dir;
        this.tm = tm;
    }

    public property float3 origin {
        get { return orig; }
    }
    public property float3 direction {
        get { return dir; }
    }
    public property float time {
        get { return tm; }
    }

    public float3 at(float t) {
        return orig + t * dir;
    }
};
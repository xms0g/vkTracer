module aabb;
import ray;

public struct AABB {
    float2 x;
    float2 y;
    float2 z;

    public __init() {}

    public __init(float2 x, float2 y, float2 z) {
        this.x = x;
        this.y = y;
        this.z = z;
    }

    public __init(float3 a, float3 b) {
        x = a.x < b.x ? float2(a.x, b.x) : float2(b.x, a.x);
        y = a.y < b.y ? float2(a.y, b.y) : float2(b.y, a.y);
        z = a.z < b.z ? float2(a.z, b.z) : float2(b.z, a.z);
    }

    public __init(AABB a, AABB b) {
        x = float2(fmin(a.x.x, b.x.x), fmax(a.x.y, b.x.y));
        y = float2(fmin(a.y.x, b.y.x), fmax(a.y.y, b.y.y));
        z = float2(fmin(a.z.x, b.z.x), fmax(a.z.y, b.z.y));
    }

    float2 axis(int axis) {
        if (axis == 0) return x;
        if (axis == 1) return y;
        return z;
    }

    public bool hit(Ray ray, float2 t) {
        float3 rayOrigin = ray.origin;
        float3 rayDir = ray.direction;

        for (int i = 0; i < 3; ++i) {
        	float2 axis = axis(i);
        	float invD = 1.0 / rayDir[i];

           	float t0 = invD * (axis.x - rayOrigin[i]);
           	float t1 = invD * (axis.y - rayOrigin[i]);

           	if (t0 < t1) {
        		if (t0 > t.x) t.x = t0;
            	if (t1 < t.y) t.y = t1;
			} else {
				if (t1 > t.x) t.x = t1;
				if (t0 < t.y) t.y = t0;
			}

			if (t.y <= t.x) {
				return false;
			}
        }
		return true;
    }
};
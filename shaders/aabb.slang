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
        padToMin();
    }

    public __init(float3 a, float3 b) {
        x = float2(fmin(a.x, b.x), fmax(a.x, b.x));
        y = float2(fmin(a.y, b.y), fmax(a.y, b.y));
        z = float2(fmin(a.z, b.z), fmax(a.z, b.z));
        padToMin();
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

    [mutating]
    void padToMin() {
		constexpr float delta = 0.0001;
		const float xSize = x.y - x.x;
		const float ySize = y.y - y.x;
		const float zSize = z.y - z.x;

		x = xSize < delta ? float2(x.x - delta/2, x.y + delta/2) : x;
		y = ySize < delta ? float2(y.x - delta/2, y.y + delta/2) : y;
		z = zSize < delta ? float2(z.x - delta/2, z.y + delta/2) : z;
	}
};
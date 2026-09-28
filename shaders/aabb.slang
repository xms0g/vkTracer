module aabb;
import ray;

public struct AABB {
    float2 x;
    float2 y;
    float2 z;

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
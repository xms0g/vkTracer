module sphere;
import ray;
import hitRecord;
import material;

public struct Sphere {
    private float4 centerRadius;
    private Material mat;

    public bool hit(Ray ray, float tmin, float tmax, inout HitRecord rec) {
        float3 center = centerRadius.xyz;
        float radius = centerRadius.w;
        float3 oc = center - ray.origin;
        float a = dot(ray.direction, ray.direction);
        float h = dot(ray.direction, oc);
        float c = dot(oc, oc) - radius * radius;
        float discriminant = h * h - a * c;

        if (discriminant < 0) {
            return false;
        }

        float sqrtd = sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        float root = (h - sqrtd) / a;
        if (root <= tmin || tmax <= root) {
            root = (h + sqrtd) / a;

            if (root <= tmin || tmax <= root)
                return false;
        }

        rec.interval = root;
        rec.point = ray.at(rec.interval);
        rec.material = mat;

        float3 outwardNormal = (rec.point - center) / radius;
        rec.setFaceNormal(ray, outwardNormal);

        return true;
    }
};
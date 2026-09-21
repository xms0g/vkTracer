module sphere;
import ray;
import hitRecord;
import material;

public struct SphereData {
    public float4 centerRadius;
    public float4 centerMovable;
    public Material mat;
};

public struct Sphere {
    private Ray center;
    private float radius;
    private Material mat;

    public __init(float3 center1, float3 center2, float radius, bool movable, Material mat) {
        if (movable) {
             this.center = {center1, center2 - center1};
        } else {
             this.center = {center1, float3(0.0)};
        }

        this.radius = fmax(0, radius);
        this.mat = mat;
    }

    public bool hit(Ray ray, float tmin, float tmax, inout HitRecord rec) {
        float3 currentCenter = center.at(ray.time);
        float3 oc = currentCenter - ray.origin;
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

        float3 outwardNormal = (rec.point - currentCenter) / radius;
        rec.setFaceNormal(ray, outwardNormal);

        return true;
    }
};
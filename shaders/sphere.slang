module sphere;
import ray;
import hitRecord;
import material;
import texture;

public struct Sphere {
    float4 centerRadius;
    float4 center2;
    Material mat;

    public bool hit(Ray ray, float2 t, inout HitRecord rec) {
        float3 activeMask = float3(any(center2 != 0.0));
        Ray center = Ray(centerRadius.xyz, (center2.xyz - centerRadius.xyz) * activeMask);
        
        float3 currentCenter = center.at(ray.time);
        float3 oc = currentCenter - ray.origin;
        float a = dot(ray.direction, ray.direction);
        float h = dot(ray.direction, oc);
        float c = dot(oc, oc) - centerRadius.w * centerRadius.w;
        float discriminant = h * h - a * c;

        if (discriminant < 0) {
            return false;
        }

        float sqrtd = sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        float root = (h - sqrtd) / a;
        if (root <= t.x || t.y <= root) {
            root = (h + sqrtd) / a;

            if (root <= t.x || t.y <= root)
                return false;
        }

        rec.interval = root;
        rec.point = ray.at(rec.interval);
        rec.material = mat;

        float3 outwardNormal = (rec.point - currentCenter) / centerRadius.w;
        rec.setFaceNormal(ray, outwardNormal);
        rec.uv = Texture::uvFromPoint(outwardNormal);

        return true;
    }
};
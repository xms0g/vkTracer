module scene;
import ray;
import hitRecord;
import sphere;
import interval;
import aabb;

public struct Scene {
    AABB bounds;
    StructuredBuffer<SphereData> spheres;

    public __init(StructuredBuffer<SphereData> buffer) {
        spheres = buffer;
        
        for (uint i = 0; i < spheres.getCount(); ++i) {
            Sphere sphere = getSphere(i);
            bounds = AABB(bounds, sphere.boundingBox());
        }
    }

    public bool hit(Ray ray, interval t, inout HitRecord rec) {
        bool hitAnything = false;
        float closestSoFar = t.max;

        for (uint i = 0; i < spheres.getCount(); ++i) {
            Sphere sphere = getSphere(i);
            
            if (sphere.hit(ray, interval(t.min, closestSoFar), rec)) {
                hitAnything = true;
                closestSoFar = rec.interval;
            }
        }

        return hitAnything;
    }

    Sphere getSphere(uint index) {
        SphereData data = spheres[index];

        return {data.centerRadius.xyz,
                data.center2.xyz,
                data.centerRadius.w,
                data.mat};
    }
};
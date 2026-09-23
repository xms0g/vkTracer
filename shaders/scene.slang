module scene;
import ray;
import hitRecord;
import sphere;

public struct Scene {
    StructuredBuffer<SphereData> spheres;

    public __init(StructuredBuffer<SphereData> buffer) {
        spheres = buffer;
    }

    public bool hit(Ray ray, float2 t, inout HitRecord rec) {
        bool hitAnything = false;
        float closestSoFar = t.y;

        for (uint i = 0; i < spheres.getCount(); ++i) {
            Sphere sphere = getSphere(i);
            
            if (sphere.hit(ray, float2(t.x, closestSoFar), rec)) {
                hitAnything = true;
                closestSoFar = rec.interval;
            }
        }

        return hitAnything;
    }

    public Sphere getSphere(uint index) {
        SphereData data = spheres[index];

        return {data.centerRadius.xyz,
                data.center2.xyz,
                data.centerRadius.w,
                data.mat};
    }
};
module scene;
import ray;
import hitRecord;
import sphere;

public struct Scene {
    private uint64_t bufferAddress;
    private uint64_t count;
    
    public __init(uint64_t bufferAddress, uint64_t count) {
        this.bufferAddress = bufferAddress;
        this.count = count;
    }

    public bool hit(Ray ray, float2 t, inout HitRecord rec) {
        bool hitAnything = false;
        float closestSoFar = t.y;

        for (uint i = 0; i < count; ++i) {
            Sphere sphere = getSphere(i);
            
            if (sphere.hit(ray, float2(t.x, closestSoFar), rec)) {
                hitAnything = true;
                closestSoFar = rec.interval;
            }
        }

        return hitAnything;
    }

    Sphere getSphere(uint index) {
        SphereData* datas = (SphereData*)bufferAddress;
        SphereData data = datas[index];

        return {data.centerRadius.xyz,
                data.center2.xyz,
                data.centerRadius.w,
                data.mat};
    }
};
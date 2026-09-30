module scene;
import ray;
import hitRecord;
import bvh;
import hittable;
import random;

public struct Scene {
    private uint64_t bufferAddress;
    
    public __init(uint64_t bufferAddress) {
        this.bufferAddress = bufferAddress;
    }

    public bool hit(Ray ray, float2 t, inout HitRecord rec, inout Random rand) {
        bool hitAnything = false;
        float closestSoFar = t.y;

        BVHNode* node = (BVHNode*)bufferAddress;
            
        if (node->hit(ray, float2(t.x, closestSoFar), rec, rand)) {
            hitAnything = true;
            closestSoFar = rec.interval;
        }

        return hitAnything;
    }
};
module bvh;
import aabb;
import ray;
import hitRecord;
import random;
import hittable;
import sphere;
import quad;
import volume;

static const uint MAX_STACK_SIZE = 64;

public struct BVHNode {
	HittableRef left;
	HittableRef right;
    AABB bounds;

    public bool hit(Ray ray, float2 t, inout HitRecord rec, inout Random rand) {
        if (!bounds.hit(ray, t)) {
            return false;
        }

        HittableRef stack[MAX_STACK_SIZE];
        uint stackSize = 0;

        stack[stackSize++] = left;
        stack[stackSize++] = right;

        bool hitAnything = false;
        float closest = t.y;

        while (stackSize > 0) {
            HittableRef current = stack[--stackSize];

            if (current.type == HittableType.Sphere) {
                Sphere* s = (Sphere*)current.address;
                
                if (s->hit(ray, float2(t.x, closest), rec, rand)) {
                    hitAnything = true;
                    closest = rec.interval;
                }
            } else if (current.type == HittableType.Quad) {
                Quad* q = (Quad*)current.address;
                
                if (q->hit(ray, float2(t.x, closest), rec, rand)) {
                    hitAnything = true;
                    closest = rec.interval;
                }
            } else if (current.type == HittableType::Volume) {
                Volume* v = (Volume*)current.address;

                if (v->hit(ray, float2(t.x, closest), rec, rand)) {
                    hitAnything = true;
                    closest = rec.interval;
                }
            } else if (current.type == HittableType.BVHNode) {
                BVHNode* node = (BVHNode*)current.address;

                if (!node->bounds.hit(ray, float2(t.x, closest)))
                    continue;

                stack[stackSize++] = node->left;
                stack[stackSize++] = node->right;
            }
        }

        return hitAnything;
    }
};
module bvh;
import aabb;
import ray;
import hitRecord;
import random;
import hittable;
import sphere;

static const uint MAX_STACK_SIZE = 64;

public struct BVHNode {
	AABB bounds;
	HittableRef left;
	HittableRef right;

    public AABB boundingBox() {
        return bounds;
    }

    public bool hit(Ray ray, float2 t, inout HitRecord rec) {
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
                SphereData* data = (SphereData*)current.address;
                Sphere s = Sphere(data->centerRadius.xyz, data->center2.xyz, data->centerRadius.w, data->mat);
                
                if (s.hit(ray, float2(t.x, closest), rec)) {
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
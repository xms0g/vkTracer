module volume;
import ray;
import hitRecord;
import hittable;
import sphere;
import quad;
import random;
import constants;

public struct Volume {
    float4 negInvDensity;
    Material mat;
    HittableRef boundary;
  
    public bool hit(Ray ray, float2 t, inout HitRecord rec, inout Random rand) {
        HitRecord rec1;
        HitRecord rec2;

        if (!boundaryHit(ray, boundary, float2(-FLT_MAX, FLT_MAX), rec1, rand)) {
            return false;
        }

        if (!boundaryHit(ray, boundary, float2(rec1.interval + 0.0001, FLT_MAX), rec2, rand)) {
            return false;
        }

        if (rec1.interval < t.x) {
            rec1.interval = t.x;
        }
        if (rec2.interval > t.y) {
            rec2.interval = t.y;
        }

        if (rec1.interval >= rec2.interval) {
            return false;
        }
        
        if (rec1.interval < 0) {
            rec1.interval = 0;
        }

        float rayLength = length(ray.direction);
        float distanceInsideBoundary = (rec2.interval - rec1.interval) * rayLength;
        float hitDistance = negInvDensity.x * log(rand.random());

        if (hitDistance > distanceInsideBoundary) {
            return false;
        }

        rec.interval = rec1.interval + hitDistance / rayLength;
        rec.point = ray.at(rec.interval);
        rec.material = mat;
        rec.setFaceNormal(ray, float3(1.0, 0.0, 0.0)); // Arbitrary normal, since it's a volume
        
        return true;
    }

    bool boundaryHit(Ray ray, HittableRef boundary, float2 t, inout HitRecord rec, inout Random rand) {
        if (boundary.type == HittableType.Sphere) {
            Sphere* s = (Sphere*)boundary.address;
            return s->hit(ray, t, rec, rand);
        } else if (boundary.type == HittableType.Quad) {
            Quad* q = (Quad*)boundary.address;
            return q->hit(ray, t, rec, rand);
        }

        return false;
    }
}
module quad;
import hitRecord;
import aabb;
import ray;

public struct Quad {
 	float4 Q;
	float4 u;
	float4 v;
	float4 n;
	float3 w;
	float D;
	Material mat;

    public bool hit(Ray ray, float2 t, inout HitRecord rec) {
        float denom = dot(n.xyz, ray.direction);
        if (abs(denom) < 1e-8) {
            return false;
        }

        float tHit = (D - dot(n.xyz, ray.origin)) / denom;
        if (tHit < t.x || tHit > t.y) {
            return false;
        }

        float3 intersection = ray.at(tHit);
        float3 planarHitptVector = intersection - Q.xyz;
        float alpha = dot(w, cross(planarHitptVector, v.xyz));
        float beta = dot(w, cross(u.xyz, planarHitptVector));

        if (alpha < 0.0 || alpha > 1.0 || beta < 0.0 || beta > 1.0) {
            return false;
        }

        rec.uv = float2(alpha, beta);
        rec.interval = tHit;
        rec.point = intersection;
        rec.material = mat;
        rec.setFaceNormal(ray, n.xyz);

        return true;
    }
};
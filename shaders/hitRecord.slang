module hitRecord;
import ray;

public struct HitRecord {
    private float3 p;
    private float3 n;
    private float t;
    private bool frontFace;

    public property float3 point {
        get { return p; }
        set { p = newValue; }
    }
    public property float3 normal {
        get { return n; }
        set { n = newValue; }
    }
    public property float interval {
        get { return t; }
        set { t = newValue; }
    }

    [mutating]
    public void setFaceNormal(Ray ray, float3 outwardNormal) {
        frontFace = dot(ray.direction, outwardNormal) < 0;
        n = frontFace ? outwardNormal : -outwardNormal;
    }
};
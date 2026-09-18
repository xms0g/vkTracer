module hitRecord;
import ray;

public enum MaterialType: uint {
	Lambertian,
	Metal,
	Dielectric
};

public struct Material {
	public float4 color;
	public MaterialType type;
	public float fuzz;
	public float refractionIndex;
	uint pad;
};

public struct HitRecord {
    private float3 p;
    private float3 n;
    private float t;
    private Material mat;
    private bool ff;

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
    public property Material material {
        get { return mat; }
        set { mat = newValue; }
    }
    public property bool frontFace {
        get { return ff; }
        set { ff = newValue; }
    }

    [mutating]
    public void setFaceNormal(Ray ray, float3 outwardNormal) {
        ff = dot(ray.direction, outwardNormal) < 0;
        n = ff ? outwardNormal : -outwardNormal;
    }
};
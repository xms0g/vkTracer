module material;
import ray;
import hitRecord;
import random;

public interface IMaterial {
    public bool scatter(Ray in, HitRecord rec, inout float3 attenuation, inout Ray scattered, inout Random random);
};

public struct Lambertian: IMaterial {
    private float3 albedo;

    public __init(float3 albedo) {
        this.albedo = albedo;
    }

    public bool scatter(Ray in, HitRecord rec, inout float3 attenuation, inout Ray scattered, inout Random random) {
        float3 scatterDir = rec.normal + random.randomUnitVector();

        // Return true if the vector is close to zero in all dimensions.
        if (fabs(scatterDir.x) < 1e-8 && fabs(scatterDir.y) < 1e-8 && fabs(scatterDir.z) < 1e-8) {
            scatterDir = rec.normal;
        }
        scattered = {rec.point, scatterDir};
        attenuation = albedo;
        return true;
    }
};

public struct Metal: IMaterial {
    private float3 albedo;

    public __init(float3 albedo) {
        this.albedo = albedo;
    }

    public bool scatter(Ray in, HitRecord rec, inout float3 attenuation, inout Ray scattered, inout Random random) {
        float3 reflected = reflect(in.direction, rec.normal);
        scattered = {rec.point, reflected};
        attenuation = albedo;
        return true;
    }
};

public bool scatterMaterial(Ray ray,
                            HitRecord rec,
                            inout float3 attenuation,
                            inout Ray scattered,
                            inout Random random) {

    if (rec.material == MaterialType::Lambertian) {
        Lambertian mat = Lambertian(rec.color);
        return mat.scatter(ray, rec, attenuation, scattered, random);
    }

    if (rec.material == MaterialType::Metal) {
        Metal mat = Metal(rec.color);
        return mat.scatter(ray, rec, attenuation, scattered, random);
    }

    return false;
}
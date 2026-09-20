module material;
import ray;
import hitRecord;
import random;

namespace Lambertian {
    bool scatter(HitRecord rec, inout float3 attenuation, inout Ray scattered, inout Random rand) {
        float3 scatterDir = rec.normal + rand.randomUnitVector();

        // Return true if the vector is close to zero in all dimensions.
        if (fabs(scatterDir.x) < 1e-8 && fabs(scatterDir.y) < 1e-8 && fabs(scatterDir.z) < 1e-8) {
            scatterDir = rec.normal;
        }
        scattered = {rec.point, scatterDir};
        attenuation = rec.material.color.rgb;
        return true;
    }
};

namespace Metal {
    bool scatter(Ray in, HitRecord rec, inout float3 attenuation, inout Ray scattered, inout Random rand) {
        float fuzz = min(rec.material.fuzz, 1.0);
        float3 reflected = reflect(in.direction, rec.normal);
        reflected = normalize(reflected) + (fuzz * rand.randomUnitVector());
        scattered = {rec.point, reflected};
        attenuation = rec.material.color.rgb;
        return (dot(scattered.direction, rec.normal) > 0);
    }
};

namespace Dielectric {
    bool scatter(Ray in, HitRecord rec, inout float3 attenuation, inout Ray scattered, inout Random rand) {
        attenuation = {1.0, 1.0, 1.0};
        float ri = rec.frontFace ? (1.0/rec.material.refractionIndex) : rec.material.refractionIndex;
        float3 unitDir = normalize(in.direction);
        float cosTheta = min(dot(-unitDir, rec.normal), 1.0);
        float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

        float3 dir;
        if (ri * sinTheta > 1.0 || reflectance(cosTheta, ri) > rand.random()) {
            dir = reflect(unitDir, rec.normal);
        } else {
            dir = refract(unitDir, rec.normal, ri);
        }
        scattered = {rec.point, dir};
        return true;
    }

    float reflectance(float cosine, float refractionIndex) {
        float r0 = (1 - refractionIndex) / (1 + refractionIndex);
        r0 = r0 * r0;
        return r0 + (1 - r0) * pow((1 - cosine), 5);
    }
}

public bool scatter(Ray ray,
                    HitRecord rec,
                    inout float3 attenuation,
                    inout Ray scattered,
                    inout Random rand) {
    if (rec.material.type == MaterialType::Lambertian) {
        return Lambertian::scatter(rec, attenuation, scattered, rand);
    }

    if (rec.material.type == MaterialType::Metal) {
        return Metal::scatter(ray, rec, attenuation, scattered, rand);
    }

    if (rec.material.type == MaterialType::Dielectric) {
        return Dielectric::scatter(ray, rec, attenuation, scattered, rand);
    }

    return false;
}
module texture;
import perlin;
import constants;

[[vk::binding(2, 0)]]
Sampler2D image;

public enum TextureType: uint {
	SolidColor,
    Checker,
    Noise,
	Image
};

public struct Texture {
	private TextureType type;
	private float3 color;
    private float4 noise;

    public float3 sample(float2 uv, float3 p) {
        if (type == TextureType::SolidColor) {
            return color;
        }

        if (type == TextureType::Checker) {
            float sines = sin(10.0 * p.x) * sin(10.0 * p.y) * sin(10.0 * p.z);
            if (sines < 0.0) {
                return float3(0.1, 0.1, 0.1);
            } else {
                return float3(0.9, 0.9, 0.9);
            }
        }

        if (type == TextureType::Noise) {
            return float3(1.0, 1.0, 1.0) * turbulenceFBM(p, (int)noise.x, noise.y, noise.z);
        }

        if (type == TextureType::Image) {
            float2 uvClamped = clamp(uv, float2(0.0, 0.0), float2(1.0, 1.0));
            uvClamped.y = 1.0 - uvClamped.y; // Flip the y-coordinate for correct orientation
            
            return image.SampleLevel(uvClamped, 0.0).rgb;
        }
        
        return float3(0.0, 0.0, 0.0);
    }

    public static float2 uvFromPoint(float3 p) {
        float theta = acos(-p.y);
        float phi = atan2(-p.z, p.x) + PI;

        return float2(phi / (2*PI), theta / PI);
    }
};
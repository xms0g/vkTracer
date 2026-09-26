module texture;

public enum TextureType: uint {
	SolidColor,
    Checker,
	Image
};

public struct Texture {
	private TextureType type;
	private float3 color;

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
        
        return float3(0.0, 0.0, 0.0);
    }
};
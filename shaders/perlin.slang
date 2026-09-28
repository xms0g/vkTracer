module perlin;

float3 hash(float3 p) {
	p = float3(
        dot(p,float3(127.1,311.7, 74.7)),
        dot(p,float3(269.5,183.3,246.1)),
        dot(p,float3(113.5,271.9,124.6)));

	return -1.0 + 2.0*fract(sin(p)*43758.5453123);
}

float noise(in float3 p){
    float3 i = floor( p );
    float3 f = fract( p );
	
	float3 u = f*f*(3.0-2.0*f);

    return lerp( lerp( lerp( dot( hash( i + float3(0.0,0.0,0.0) ), f - float3(0.0,0.0,0.0) ), 
                        dot( hash( i + float3(1.0,0.0,0.0) ), f - float3(1.0,0.0,0.0) ), u.x),
                   lerp( dot( hash( i + float3(0.0,1.0,0.0) ), f - float3(0.0,1.0,0.0) ), 
                        dot( hash( i + float3(1.0,1.0,0.0) ), f - float3(1.0,1.0,0.0) ), u.x), u.y),
              lerp( lerp( dot( hash( i + float3(0.0,0.0,1.0) ), f - float3(0.0,0.0,1.0) ), 
                        dot( hash( i + float3(1.0,0.0,1.0) ), f - float3(1.0,0.0,1.0) ), u.x),
                   lerp( dot( hash( i + float3(0.0,1.0,1.0) ), f - float3(0.0,1.0,1.0) ), 
                        dot( hash( i + float3(1.0,1.0,1.0) ), f - float3(1.0,1.0,1.0) ), u.x), u.y), u.z );
}


public float turbulenceFBM(float3 p, int32_t octaves, float persistence, float lacunarity) {
    float amplitude = 0.5;
    float frequency = 2.0;
    float total = 0.0;
    float normalization = 0.0;

    for (int i = 0; i < octaves; ++i) {
        float noiseValue = noise(p * frequency);
        noiseValue = abs(noiseValue);

        total += noiseValue * amplitude;
        normalization += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }

    total /= normalization;

    return total;
}

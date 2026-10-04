module config;

public static constexpr int MAX_DEPTH = 50;
public static constexpr float DEFOCUS_ANGLE = 0.0;
public static constexpr float FOCUS_DIST = 1.0;
public static constexpr float VFOV = 40.0;
public static constexpr int SAMPLES_PER_PIXEL = 1000;
public static constexpr int SQRT_SPP = (int)sqrt((float)SAMPLES_PER_PIXEL);
public static constexpr float RECIP_SQRT_SPP = 1.0 / SQRT_SPP;
public static constexpr float PIXEL_SAMPLES_SCALE = 1.0 / (SQRT_SPP * SQRT_SPP);
public static constexpr float3 BGCOLOR = float3(0.1);
public static constexpr uint TILE_SIZE = 8;
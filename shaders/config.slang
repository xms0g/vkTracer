module config;

public static constexpr int MAX_DEPTH = 40;
public static constexpr float DEFOCUS_ANGLE = 0.0;
public static constexpr float FOCUS_DIST = 1.0;
public static constexpr float VFOV = 40.0;
public static constexpr int SAMPLES_PER_PIXEL = 500;
public static constexpr float PIXEL_SAMPLES_SCALE = 1.0 / SAMPLES_PER_PIXEL;
public static constexpr float3 BGCOLOR = float3(0.1);
public static constexpr uint TILE_SIZE = 8;
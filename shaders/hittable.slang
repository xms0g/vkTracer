module hittable;

public enum HittableType : uint {
    Sphere,
    Quad,
    Volume,
    BVHNode
};

public struct HittableRef {
    private uint64_t addr;
    private HittableType t;
    private uint padding;

    public property uint64_t address {
        get { return addr; }
        set { addr = newValue; }
    }
    public property HittableType type {
        get { return t; }
        set { t = newValue; }
    }
};
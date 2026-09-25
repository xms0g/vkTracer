module hittable;

public enum HittableType : uint {
    Sphere,
    BVHNode
};

public struct HittableRef {
    private uint64_t addr;
    private HittableType t;

    public __init(uint64_t address, HittableType type) {
        this.addr  = address;
        this.t = type;
    }

    public property uint64_t address {
        get { return addr; }
        set { addr = newValue; }
    }
    public property HittableType type {
        get { return t; }
        set { t = newValue; }
    }
};
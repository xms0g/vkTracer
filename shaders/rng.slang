implementing random;

struct RNG {
    uint state;

    __init(uint seed) {
        state = seed;
    }

    [mutating]
    float nextFloat() {
        uint oldState = state;
        state = oldState * 747796405u + 2891336453u;
        uint word = ((oldState >> ((oldState >> 28u) + 4u)) ^ oldState) * 277803737u;
        uint result = (word >> 22u) ^ word;
        return float(result) / 4294967295.0;
    }
};
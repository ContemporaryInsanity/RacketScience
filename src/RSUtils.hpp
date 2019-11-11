// Thanks to Marc Boulé
// These clamps are inf & NaN proof

static inline float clamp20V(float in) {
    if(in >= -20.f && in <= 20.f) return in;
    return in > 20.f ? 20.f: -20.f;
}

static inline float clamp10V(float in) {
    if(in >= -10.f && in <= 10.f) return in;
    return in > 10.f ? 10.f : -10.f;
}
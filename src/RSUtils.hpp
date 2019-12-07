#include <ctime>
#include <chrono>

#define quantize(v)  (round(v * 12.f) / 12.f)
#define octave(v)    int(floor(quantize(v)))
#define note(v)      (int(round((v + 10) * 12)) % 12)

static float noteVoltage[] = {
    0.00000f, // C
    0.08333f, // C#
    0.16667f, // D
    0.25000f, // D#
    0.33333f, // E
    0.41667f, // F
    0.50000f, // F#
    0.58333f, // G
    0.66667f, // G#
    0.75000f, // A
    0.83333f, // A#
    0.91667f, // B
};

static inline float RSclamp(float in, float min, float max) {
	return in < min ? min : (in > max ? max : in);
}

static inline float RSscale(float in, float inMin, float inMax, float outMin, float outMax) {
	return((outMax - outMin) * (in - inMin) / (inMax - inMin)) + outMin;
}

// May as well git rid of the following and replace with above?  I expect these are quicker tho
/*
static inline float clamp20V(float in) {
    if(in >= -20.f && in <= 20.f) return in;
    return in > 20.f ? 20.f: -20.f;
}

static inline float clamp10V(float in) {
    if(in >= -10.f && in <= 10.f) return in;
    return in > 10.f ? 10.f : -10.f;
}

static inline float clamp010V(float in) {
    if(in >= 0.f && in <= 10.f) return in;
    return in > 10.f ? 10.f : 0.f;
}
*/
static void saveSettings(json_t *rootJ) {
    std::string settingsFile = asset::user("RacketScience.json");
    FILE *file = fopen(settingsFile.c_str(), "w");

    if(file) {
        json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
        fclose(file);
    }
}

static json_t *loadSettings() {
    std::string settingsFile = asset::user("RacketScience.json");
    FILE *file = fopen(settingsFile.c_str(), "r");
    
    if(!file) {
        return json_object();
    }

    json_error_t error;
    json_t *rootJ = json_loadf(file, 0, &error);

    fclose(file);

    return rootJ;
}

static void saveDefaultTheme(int theme) {
    json_t *rootJ = loadSettings();

    json_object_set_new(rootJ, "DefaultTheme", json_integer(theme));

    saveSettings(rootJ);

    json_decref(rootJ);
}

static int loadDefaultTheme() {
    int theme = 0;

    json_t *rootJ = loadSettings();

    json_t *jsonTheme = json_object_get(rootJ, "DefaultTheme");
    if(jsonTheme) theme = json_integer_value(jsonTheme);
    
    json_decref(rootJ);

    return theme;
}

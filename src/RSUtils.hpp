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

#define quantize(v)  (round(v * 12.f) / 12.f)
#define octave(v)    int(floor(quantize(v)))
#define note(v)      (int(round((v + 10) * 12)) % 12)

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

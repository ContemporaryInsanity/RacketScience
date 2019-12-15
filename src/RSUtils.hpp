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


static void saveSettings(json_t *rootJ) {
    std::string settingsFile = asset::user("RacketScience/RacketScience.json");
    FILE *file = fopen(settingsFile.c_str(), "w");

    if(file) {
        json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
        fclose(file);
    }
}

static json_t *loadSettings() {
    std::string settingsFile = asset::user("RacketScience/RacketScience.json");
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

static void updateRSTheme() {
    RSGlobal.bgColor = nvgHSL(RSGlobal.themes[RSGlobal.themeIdx].bgColor.hue,
                              RSGlobal.themes[RSGlobal.themeIdx].bgColor.sat,
                              RSGlobal.themes[RSGlobal.themeIdx].bgColor.lum);

    RSGlobal.lbColor = nvgHSL(RSGlobal.themes[RSGlobal.themeIdx].lbColor.hue,
                              RSGlobal.themes[RSGlobal.themeIdx].lbColor.sat,
                              RSGlobal.themes[RSGlobal.themeIdx].lbColor.lum);

    RSGlobal.ssColor = nvgHSL(RSGlobal.themes[RSGlobal.themeIdx].ssColor.hue,
                              RSGlobal.themes[RSGlobal.themeIdx].ssColor.sat,
                                RSGlobal.themes[RSGlobal.themeIdx].ssColor.lum);
}

static void saveRSGlobal() {
    std::string settingsFile = asset::user("RacketScience/RSGlobal.dat");
    FILE *file = fopen(settingsFile.c_str(), "w");
    if(file) {
        fwrite(&RSGlobal, sizeof(struct rsglobal), 1, file);
        fclose(file);
    }
}

static void loadRSGlobal() {
    std::string RSGDir = rack::asset::user("RacketScience/");
    if(!rack::system::isDirectory(RSGDir)) {
        rack::system::createDirectory(RSGDir);

        // First time so create defaults
        RSGlobal.themes[0].bgColor = {0.0f, 0.0f, 0.1f};
        RSGlobal.themes[0].lbColor = {0.0f, 0.0f, 0.6f};
        RSGlobal.themes[0].ssColor = {0.0f, 0.0f, 0.6f};

        RSGlobal.themes[1].bgColor = {0.0f, 0.0f, 0.0f};
        RSGlobal.themes[1].lbColor = {0.17f, 0.4f, 0.37f};
        RSGlobal.themes[1].ssColor = {0.17f, 0.4f, 0.37f};

        RSGlobal.themes[2].bgColor = {0.2f, 0.5f, 0.2f};
        RSGlobal.themes[2].lbColor = {0.0f, 0.0f, 0.6f};
        RSGlobal.themes[2].ssColor = {0.0f, 0.0f, 0.6f};

        RSGlobal.themes[3].bgColor = {0.4f, 0.5f, 0.2f};
        RSGlobal.themes[3].lbColor = {0.0f, 0.0f, 0.6f};
        RSGlobal.themes[3].ssColor = {0.0f, 0.0f, 0.6f};

        RSGlobal.themeIdx = 0;
        updateRSTheme();
        saveRSGlobal();
    }
    else {
        std::string settingsFile = asset::user("RacketScience/RSGlobal.dat");
        FILE *file = fopen(settingsFile.c_str(), "r");
        if(file) {
            fread(&RSGlobal, sizeof(struct rsglobal), 1, file);
            fclose(file);
        }
    }
}


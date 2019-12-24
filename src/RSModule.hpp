struct RSModule : Module {
    int RSTheme = 1;
};

static void updateRSTheme(int themeIdx) {
    RSGlobal.themes[themeIdx].bgColor = nvgHSL(RSGlobal.themes[themeIdx].bghsl.hue,
                                               RSGlobal.themes[themeIdx].bghsl.sat,
                                               RSGlobal.themes[themeIdx].bghsl.lum);
    RSGlobal.themes[themeIdx].lbColor = nvgHSL(RSGlobal.themes[themeIdx].lbhsl.hue,
                                               RSGlobal.themes[themeIdx].lbhsl.sat,
                                               RSGlobal.themes[themeIdx].lbhsl.lum);
    RSGlobal.themes[themeIdx].ssColor = nvgHSL(RSGlobal.themes[themeIdx].sshsl.hue,
                                               RSGlobal.themes[themeIdx].sshsl.sat,
                                               RSGlobal.themes[themeIdx].sshsl.lum);
    float ledSat = 1.0f;
    float ledLum = 0.5f;
    RSGlobal.themes[themeIdx].lAColor = nvgHSL(RSGlobal.themes[themeIdx].ledAh, ledSat, ledLum);
    RSGlobal.themes[themeIdx].lBColor = nvgHSL(RSGlobal.themes[themeIdx].ledBh, ledSat, ledLum);
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

        RSGlobal.themes[4].bgColor = {0.0f, 0.0f, 0.1f};
        RSGlobal.themes[4].lbColor = {0.0f, 0.0f, 0.6f};
        RSGlobal.themes[4].ssColor = {0.0f, 0.0f, 0.6f};

        RSGlobal.themes[5].bgColor = {0.0f, 0.0f, 0.0f};
        RSGlobal.themes[5].lbColor = {0.17f, 0.4f, 0.37f};
        RSGlobal.themes[5].ssColor = {0.17f, 0.4f, 0.37f};

        RSGlobal.themes[6].bgColor = {0.2f, 0.5f, 0.2f};
        RSGlobal.themes[6].lbColor = {0.0f, 0.0f, 0.6f};
        RSGlobal.themes[6].ssColor = {0.0f, 0.0f, 0.6f};

        RSGlobal.themes[7].bgColor = {0.4f, 0.5f, 0.2f};
        RSGlobal.themes[7].lbColor = {0.0f, 0.0f, 0.6f};
        RSGlobal.themes[7].ssColor = {0.0f, 0.0f, 0.6f};

        RSGlobal.themeIdx = 0;
        updateRSTheme(RSGlobal.themeIdx);
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

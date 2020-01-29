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
	INFO("Racket Science: saveRsGlobal()");

    std::string RSGFile = asset::user("RacketScience/RSGlobal.dat");
    FILE *file = fopen(RSGFile.c_str(), "w");
    if(file) {
        fwrite(&RSGlobal, sizeof(struct rsglobal), 1, file);
        fclose(file);
    }
}

static void loadRSGlobal() {
	INFO("Racket Science: loadRSGlobal()");

    std::string RSGDir = rack::asset::user("RacketScience/");
    std::string RSGFile = rack::asset::user("RacketScience/RSGlobal.dat");

    if(!rack::system::isDirectory(RSGDir) || !rack::system::isFile(RSGFile)) {
        INFO("Racket Science: Creating default themes");

        rack::system::createDirectory(RSGDir);

        float hue = 0.f;
        float hueStep = 1.f / RSGlobal.themeCount;
        for(int i = 0; i < RSGlobal.themeCount; i++, hue += hueStep) {
            RSGlobal.themes[i].bghsl = {hue, .5f, .3f};
            RSGlobal.themes[i].lbhsl = {hue, .8f, .8f};
            RSGlobal.themes[i].sshsl = {hue, .6f, .7f};
            // LEDs here too once complete
            updateRSTheme(i);
        }

        RSGlobal.themeIdx = 0;
        updateRSTheme(RSGlobal.themeIdx);
        saveRSGlobal();
    }
    else {
        FILE *file = fopen(RSGFile.c_str(), "r");
        if(file) {
            fread(&RSGlobal, sizeof(struct rsglobal), 1, file);
            fclose(file);
        }
    }
}

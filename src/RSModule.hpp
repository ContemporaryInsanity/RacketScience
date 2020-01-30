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

/*
void PatchManager::save(std::string path) {
	INFO("Saving patch %s", path.c_str());
	json_t* rootJ = toJson();
	if (!rootJ)
		return;
	DEFER({
		json_decref(rootJ);
	});

	// Write to temporary path and then rename it to the correct path
	std::string tmpPath = path + ".tmp";
	FILE* file = std::fopen(tmpPath.c_str(), "w");
	if (!file) {
		// Fail silently
		return;
	}

	json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
	std::fclose(file);
	system::moveFile(tmpPath, path);
}
*/

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

/*
bool PatchManager::load(std::string path) {
	INFO("Loading patch %s", path.c_str());
	FILE* file = std::fopen(path.c_str(), "r");
	if (!file) {
		// Exit silently
		return false;
	}
	DEFER({
		std::fclose(file);
	});

	json_error_t error;
	json_t* rootJ = json_loadf(file, 0, &error);
	if (!rootJ) {
		std::string message = string::f("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
		osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, message.c_str());
		return false;
	}
	DEFER({
		json_decref(rootJ);
	});

	APP->history->clear();
	APP->scene->rack->clear();
	APP->scene->rackScroll->reset();
	legacy = 0;
	fromJson(rootJ);
	return true;
}
*/
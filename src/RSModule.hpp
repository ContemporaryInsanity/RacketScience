#include <osdialog.h>

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

    // std::string RSGFile = asset::user("RacketScience/RSGlobal.dat");
    // FILE *file = fopen(RSGFile.c_str(), "w");
    // if(file) {
    //     fwrite(&RSGlobal, sizeof(struct rsglobal), 1, file);
    //     fclose(file);
    // }

    std::string RSGFile = asset::user("RacketScience/RSGlobal.json");;
    FILE *file = fopen(RSGFile.c_str(), "w");
    if(file) {
        json_t* rootJ = json_object();
        json_object_set_new(rootJ, "themeCount", json_integer(RSGlobal.themeCount));
        json_object_set_new(rootJ, "themeIdx", json_integer(RSGlobal.themeIdx));
        json_object_set_new(rootJ, "rateDivider", json_integer(RSGlobal.rateDivider));
        json_object_set_new(rootJ, "logLevel", json_integer(RSGlobal.logLevel));

        for(int theme = 0; theme < RSGlobal.themeCount; theme++) {
            json_t* themeJ = json_object();
            json_object_set_new(themeJ, "bghue", json_real(RSGlobal.themes[theme].bghsl.hue));
            json_object_set_new(themeJ, "bgsat", json_real(RSGlobal.themes[theme].bghsl.sat));
            json_object_set_new(themeJ, "bglum", json_real(RSGlobal.themes[theme].bghsl.lum));

            json_object_set_new(themeJ, "lbhue", json_real(RSGlobal.themes[theme].lbhsl.hue));
            json_object_set_new(themeJ, "lbsat", json_real(RSGlobal.themes[theme].lbhsl.sat));
            json_object_set_new(themeJ, "lblum", json_real(RSGlobal.themes[theme].lbhsl.lum));

            json_object_set_new(themeJ, "sshue", json_real(RSGlobal.themes[theme].sshsl.hue));
            json_object_set_new(themeJ, "sssat", json_real(RSGlobal.themes[theme].sshsl.sat));
            json_object_set_new(themeJ, "sslum", json_real(RSGlobal.themes[theme].sshsl.lum));

            json_object_set_new(themeJ, "ledAhue", json_real(RSGlobal.themes[theme].ledAh));
            json_object_set_new(themeJ, "ledBhue", json_real(RSGlobal.themes[theme].ledBh));

            json_object_set_new(rootJ, ("theme" + std::to_string(theme)).c_str(), themeJ);
        }

        json_dumpf(rootJ, file, JSON_INDENT(2) | JSON_REAL_PRECISION(9));
        fclose(file);
    }
}


static void loadRSGlobal() {
	INFO("Racket Science: loadRSGlobal()");

    std::string RSGDir = rack::asset::user("RacketScience/");
    std::string RSGFile = rack::asset::user("RacketScience/RSGlobal.json");

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

        //RSGlobal.themeIdx = 0;
        //updateRSTheme(RSGlobal.themeIdx);
        saveRSGlobal();
    }
    else {
        FILE *file = fopen(RSGFile.c_str(), "r");
        if(file) {
            //fread(&RSGlobal, sizeof(struct rsglobal), 1, file);
            json_error_t error;
            json_t* rootJ = json_loadf(file, 0, &error);
            if(!rootJ) {
                std::string message = string::f("JSON parsing error at %s %d:%d %s", error.source, error.line, error.column, error.text);
                osdialog_message(OSDIALOG_WARNING, OSDIALOG_OK, message.c_str());
            }
            else {
                json_t* themeCountJ = json_object_get(rootJ, "themeCount");
                // if(themeCountJ) RSGlobal.themeCount = json_integer_value(themeCountJ);
                json_t* themeIdxJ = json_object_get(rootJ, "themeIdx");
                if(themeIdxJ) RSGlobal.themeIdx = json_integer_value(themeIdxJ);
                json_t* rateDividerJ = json_object_get(rootJ, "rateDivider");
                if(rateDividerJ) RSGlobal.rateDivider = json_integer_value(rateDividerJ);
                json_t* logLevelJ = json_object_get(rootJ, "logLevel");
                if(logLevelJ) RSGlobal.logLevel = json_integer_value(logLevelJ);

                for(int theme = 0; theme < RSGlobal.themeCount; theme++) {
                    json_t* themeJ = json_object_get(rootJ, ("theme" + std::to_string(theme)).c_str());
                    if(themeJ) {
                        json_t* bghueJ = json_object_get(themeJ, "bghue");
                        if(bghueJ) RSGlobal.themes[theme].bghsl.hue = json_real_value(bghueJ);
                        json_t* bgsatJ = json_object_get(themeJ, "bgsat");
                        if(bgsatJ) RSGlobal.themes[theme].bghsl.sat = json_real_value(bgsatJ);
                        json_t* bglumJ = json_object_get(themeJ, "bglum");
                        if(bglumJ) RSGlobal.themes[theme].bghsl.lum = json_real_value(bglumJ);

                        json_t* lbhueJ = json_object_get(themeJ, "lbhue");
                        if(lbhueJ) RSGlobal.themes[theme].lbhsl.hue = json_real_value(lbhueJ);
                        json_t* lbsatJ = json_object_get(themeJ, "lbsat");
                        if(lbsatJ) RSGlobal.themes[theme].lbhsl.sat = json_real_value(lbsatJ);
                        json_t* lblumJ = json_object_get(themeJ, "lblum");
                        if(lblumJ) RSGlobal.themes[theme].lbhsl.lum = json_real_value(lblumJ);

                        json_t* sshueJ = json_object_get(themeJ, "sshue");
                        if(sshueJ) RSGlobal.themes[theme].sshsl.hue = json_real_value(sshueJ);
                        json_t* sssatJ = json_object_get(themeJ, "sssat");
                        if(sssatJ) RSGlobal.themes[theme].sshsl.sat = json_real_value(sssatJ);
                        json_t* sslumJ = json_object_get(themeJ, "sslum");
                        if(sslumJ) RSGlobal.themes[theme].sshsl.lum = json_real_value(sslumJ);

                        json_t* ledAhueJ = json_object_get(themeJ, "ledAhue");
                        if(ledAhueJ) RSGlobal.themes[theme].ledAh = json_real_value(ledAhueJ);
                        json_t* ledBhueJ = json_object_get(themeJ, "ledBhue");
                        if(ledBhueJ) RSGlobal.themes[theme].ledBh = json_real_value(ledBhueJ);
                        
                        updateRSTheme(theme);
                    }
                }
            }
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
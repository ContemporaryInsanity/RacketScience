#pragma once
#include <rack.hpp>

struct rshsl {
    float hue;
    float sat;
    float lum;
};

struct rstheme {
    // We store HSL so we can set HSL knobs when switching theme without messing around converting to RGB etc
    struct rshsl bghsl;
    struct rshsl lbhsl;
    struct rshsl sshsl;
    float ledAh;
    float ledBh;
    // Then we also store the actual colors, which are updated using updateRSTheme
    NVGcolor bgColor;
    NVGcolor lbColor;
    NVGcolor ssColor;
    NVGcolor lAColor;
    NVGcolor lBColor;
};

struct rsglobal {
    static const int themeCount = 16;
    int themeIdx;
    rstheme themes[themeCount];
    int logLevel;
    int rateDivider;
};

extern rsglobal RSGlobal;

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin *pluginInstance;

// Declare each Model, defined in each module source file
extern Model *modelRSVectorVictor;

extern Model *modelRSGroundControl;
extern Model *modelRSMajorTom;
extern Model *modelRSHeat;
extern Model *modelRSReheat;
extern Model *modelRSCVHeat;
extern Model *modelRSBoogieBay;
extern Model *modelRSBoogieBayH8;
extern Model *modelRSMFH;
extern Model *modelRSBlank;
extern Model *modelRSShades;
extern Model *modelRSFido316;
extern Model *modelRSPhaseOne;
extern Model *modelRSMissionControl;
extern Model *modelRSLaunchControl;

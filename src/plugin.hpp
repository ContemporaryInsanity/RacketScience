#pragma once
#include <rack.hpp>

struct rshsl {
    float hue;
    float sat;
    float lum;
};

struct rstheme {
    struct rshsl bgColor;
    struct rshsl lbColor;
    struct rshsl ssColor;
};

struct rsglobal {
    NVGcolor bgColor; // Background
    NVGcolor lbColor; // Labels
    NVGcolor ssColor; // Scribble strips
    static const int themeCount = 4;
    int themeIdx;
    rstheme themes[themeCount];
};

extern rsglobal RSGlobal;

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin *pluginInstance;

// Declare each Model, defined in each module source file
extern Model *modelRSVectorVictor;
extern Model *modelRSBoogieBay;
extern Model *modelRSBoogieBayH8;
extern Model *modelRSMFH;
extern Model *modelRSHeat;
extern Model *modelRSReheat;
extern Model *modelRSCVHeat;
extern Model *modelRSGroundControl;

extern Model *modelRSScratch;

extern Model *modelRSSkeleton;
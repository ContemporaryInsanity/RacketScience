#pragma once
#include <rack.hpp>

extern int RSTheme;
extern int RSThemes;

struct rsglobal {
	int x;
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

extern Model *modelRSScratch;

extern Model *modelRSSkeleton;
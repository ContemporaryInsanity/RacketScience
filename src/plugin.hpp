#pragma once
#include <rack.hpp>

struct rsglobal {
	int theme;
    int themeCount;
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
#include "plugin.hpp"

#include "RSModule.hpp" 

rsglobal RSGlobal;

Plugin *pluginInstance;

void init(Plugin *p) {
	pluginInstance = p;

	// Add modules here
	// Order dicates order they will show in module browser
	p->addModel(modelRSGroundControl);
	p->addModel(modelRSHeat);
	p->addModel(modelRSReheat);
	p->addModel(modelRSCVHeat);
	p->addModel(modelRSBoogieBay);
	p->addModel(modelRSBoogieBayH8);
	p->addModel(modelRSMFH);
	p->addModel(modelRSBlank);
	p->addModel(modelRSScratch);

	p->addModel(modelRSVectorVictor);
	p->addModel(modelRSSkeleton);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.

	loadRSGlobal();
}


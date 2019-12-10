#include "plugin.hpp"
#include "RSUtils.hpp"

rsglobal RSGlobal;

Plugin *pluginInstance;

void init(Plugin *p) {
	pluginInstance = p;

	// Add modules here
    p->addModel(modelRSVectorVictor);
    p->addModel(modelRSBoogieBay);
	p->addModel(modelRSBoogieBayH8);
	p->addModel(modelRSMFH);
	p->addModel(modelRSHeat);
	p->addModel(modelRSReheat);
	p->addModel(modelRSCVHeat);

	p->addModel(modelRSScratch);

	p->addModel(modelRSSkeleton);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.

	RSGlobal.theme = loadDefaultTheme();
	RSGlobal.themeCount = 5;
}

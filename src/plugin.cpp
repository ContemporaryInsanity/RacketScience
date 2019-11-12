#include "plugin.hpp"


Plugin *pluginInstance;


void init(Plugin *p) {
	pluginInstance = p;

	// Add modules here
	// p->addModel(modelMyModule);
    p->addModel(modelRSVectorVictor);
    p->addModel(modelRSBoogieBay);
	p->addModel(modelRSBoogieBayH8);
	p->addModel(modelRSMFH);

	p->addModel(modelRSScratch);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}

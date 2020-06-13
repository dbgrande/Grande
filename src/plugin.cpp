#include "plugin.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;

	// Add modules here
	// p->addModel(modelMyModule);
	p->addModel(modelBlank1HP);
	p->addModel(modelQuant);
	p->addModel(modelQuant31);
	p->addModel(modelSampleDelays);
	p->addModel(modelScale);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}

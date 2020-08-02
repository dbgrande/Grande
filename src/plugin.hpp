#pragma once
#include <rack.hpp>
#include "components.hpp"


using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Declare each Model, defined in each module source file
// extern Model* modelMyModule;
extern Model* modelBlank1HP;
extern Model* modelCVBigInts;
extern Model* modelCVInts;
extern Model* modelFrequency;
extern Model* modelNoteMT;
extern Model* modelQuant;
extern Model* modelQuant31;
extern Model* modelQuantMT;
extern Model* modelQuantMTIntervals;
extern Model* modelQuantMTInt11;
extern Model* modelSampleDelays;
extern Model* modelScale;

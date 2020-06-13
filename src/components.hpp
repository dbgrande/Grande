using namespace rack;

extern Plugin *pluginInstance;

struct WhiteButtonRoot : SvgSwitch {
  WhiteButtonRoot() {
    momentary = false;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/WhiteButton1.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/WhiteButton1.svg")));
    fb->removeChild(shadow);
    delete shadow;
  }
};

struct WhiteButton : SvgSwitch {
  WhiteButton() {
    momentary = false;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/WhiteButton0.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/WhiteButton1.svg")));
    fb->removeChild(shadow);
    delete shadow;
  }
};

struct BlackButton : SvgSwitch {
  BlackButton() {
    momentary = false;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BlackButton0.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BlackButton1.svg")));
    fb->removeChild(shadow);
    delete shadow;
  }
};

struct PentaButtonRoot : SvgSwitch {
  PentaButtonRoot() {
    momentary = false;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PentaButton1.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PentaButton1.svg")));
    fb->removeChild(shadow);
    delete shadow;
  }
};

struct PentaButton : SvgSwitch {
  PentaButton() {
    momentary = false;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PentaButton0.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PentaButton1.svg")));
    fb->removeChild(shadow);
    delete shadow;
  }
};

struct RoundSmallRotarySwitch : RoundSmallBlackKnob {
	RoundSmallRotarySwitch() {
		//minAngle = -0.83*M_PI;
		//maxAngle = 0.83*M_PI;
		snap = true;
		smooth = false;
	}

	// handle the manually entered values
	void onChange(const event::Change &e) override {
		RoundSmallBlackKnob::onChange(e);
		paramQuantity->setValue(roundf(paramQuantity->getValue()));
	}

	// override the base randomizer as it sets switches to invalid values.
	void randomize() override {
		RoundSmallBlackKnob::randomize();
		paramQuantity->setValue(roundf(paramQuantity->getValue()));
	}
};

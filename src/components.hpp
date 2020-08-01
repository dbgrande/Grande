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

struct RectButton : SvgSwitch {
  RectButton() {
    momentary = false;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/RectButton0.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/RectButton1.svg")));
    fb->removeChild(shadow);
    delete shadow;
  }
};

struct ConfigButton : SvgSwitch {
  ConfigButton() {
    momentary = true;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ConfigButton0.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ConfigButton1.svg")));
    fb->removeChild(shadow);
    delete shadow;
  }
};

struct TL1105Momentary : SvgSwitch {
  TL1105Momentary() {
    momentary = true;
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TL1105_0.svg")));
    addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/TL1105_1.svg")));
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

struct RoundBlackRotarySwitch : RoundBlackKnob {
	RoundBlackRotarySwitch() {
		//minAngle = -0.83*M_PI;
		//maxAngle = 0.83*M_PI;
		snap = true;
		smooth = false;
	}

	// handle the manually entered values
	void onChange(const event::Change &e) override {
		RoundBlackKnob::onChange(e);
		paramQuantity->setValue(roundf(paramQuantity->getValue()));
	}

	// override the base randomizer as it sets switches to invalid values.
	void randomize() override {
		RoundBlackKnob::randomize();
		paramQuantity->setValue(roundf(paramQuantity->getValue()));
	}
};

struct RoundLargeRotarySwitch : RoundLargeBlackKnob {
	RoundLargeRotarySwitch() {
		//minAngle = -0.83*M_PI;
		//maxAngle = 0.83*M_PI;
		snap = true;
		smooth = false;
	}

	// handle the manually entered values
	void onChange(const event::Change &e) override {
		RoundLargeBlackKnob::onChange(e);
		paramQuantity->setValue(roundf(paramQuantity->getValue()));
	}

	// override the base randomizer as it sets switches to invalid values.
	void randomize() override {
		RoundLargeBlackKnob::randomize();
		paramQuantity->setValue(roundf(paramQuantity->getValue()));
	}
};

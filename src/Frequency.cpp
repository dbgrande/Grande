#include "plugin.hpp"


struct Frequency : Module {
	enum ParamIds {
		THRESH_PARAM,
		SMOOTH_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		ANALOG_INPUT,
		THRESH_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		VOCT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Frequency() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(THRESH_PARAM, -10.f, 10.f, 0.f, "Threshold", "V");
		configParam(SMOOTH_PARAM, 1, 16, 4, "Smooth", "");
	}

	float last_sample = 0;
	float count = 0;
	float cvfreq[16] = { 0.f };

	void process(const ProcessArgs &args) override {
		// comparator threshold
		int channels = inputs[THRESH_INPUT].getChannels();
		float thresh = 0.f;
		if (channels > 0)
			thresh = inputs[THRESH_INPUT].getVoltage();
		else
			thresh = params[THRESH_PARAM].getValue();

		// number of periods to average
		int smooth = clamp((int)(params[SMOOTH_PARAM].getValue()), 1, 16);

		// main process
		float sample = inputs[ANALOG_INPUT].getVoltage();

		if (last_sample < thresh && sample >= thresh && count >= 1) {
			// interpolate zero crossing within sample (positive to right)
			float fracPeriod = (thresh - last_sample)/(sample - last_sample) - 0.5f;
			float freq = (float) args.sampleRate / (count + 1 + fracPeriod);
			float cvavg = 0;
			for (int i = 0; i < smooth - 1; i++) {
				cvfreq[i] = cvfreq[i + 1];
				cvavg += cvfreq[i];
			}
			cvfreq[smooth - 1] = std::log2f(freq / dsp::FREQ_C4);
			cvavg += cvfreq[smooth - 1];
			outputs[VOCT_OUTPUT].setVoltage(cvavg / smooth);
			count = -fracPeriod;
		} else {
			count += 1;
		}
		last_sample = sample;
	}
};

struct FrequencyWidget : ModuleWidget {
	FrequencyWidget(Frequency* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Frequency.svg")));

		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.08, 27.00)), module, Frequency::ANALOG_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.08, 42.00)), module, Frequency::VOCT_OUTPUT));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(5.08, 56.25)), module, Frequency::THRESH_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.08, 66.50)), module, Frequency::THRESH_INPUT));

		addParam(createParamCentered<RoundSmallRotarySwitch>(mm2px(Vec(5.08, 80.75)), module, Frequency::SMOOTH_PARAM));
	}
};


Model* modelFrequency = createModel<Frequency, FrequencyWidget>("Frequency");

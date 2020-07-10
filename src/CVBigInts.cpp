#include "plugin.hpp"


struct CVBigInts : Module {
	enum ParamIds {
		A_PARAM,
		B_PARAM,
		C_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		A1_OUTPUT,
		A2_OUTPUT,
		B1_OUTPUT,
		B2_OUTPUT,
		C1_OUTPUT,
		C2_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	CVBigInts() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(A_PARAM, 0, 32, 10, "A Value", "");
		configParam(B_PARAM, 0, 32, 5, "B Value", "");
		configParam(C_PARAM, 0, 32, 1, "C Value", "");
	}

	void process(const ProcessArgs &args) override {

		int a = clamp((int)(params[A_PARAM].getValue()), 0, 32);
		int b = clamp((int)(params[B_PARAM].getValue()), 0, 32);
		int c = clamp((int)(params[C_PARAM].getValue()), 0, 32);

		outputs[A1_OUTPUT].setVoltage(a);
		outputs[A2_OUTPUT].setVoltage(-a);
		outputs[B1_OUTPUT].setVoltage(b);
		outputs[B2_OUTPUT].setVoltage(-b);
		outputs[C1_OUTPUT].setVoltage(c);
		outputs[C2_OUTPUT].setVoltage(-c);
	}
};

struct CVBigIntsWidget : ModuleWidget {
	CVBigIntsWidget(CVBigInts* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CVBigInts.svg")));

		//addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		//addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundSmallRotarySwitch>(mm2px(Vec(5.08, 17.50)), module, CVBigInts::A_PARAM));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.08, 29.75)), module, CVBigInts::A1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.08, 42.00)), module, CVBigInts::A2_OUTPUT));

		addParam(createParamCentered<RoundSmallRotarySwitch>(mm2px(Vec(5.08, 54.25)), module, CVBigInts::B_PARAM));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.08, 66.50)), module, CVBigInts::B1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.08, 78.75)), module, CVBigInts::B2_OUTPUT));


		addParam(createParamCentered<RoundSmallRotarySwitch>(mm2px(Vec(5.08, 91.00)), module, CVBigInts::C_PARAM));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.08, 103.25)), module, CVBigInts::C1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.08, 115.50)), module, CVBigInts::C2_OUTPUT));
	}
};


Model* modelCVBigInts = createModel<CVBigInts, CVBigIntsWidget>("CVBigInts");

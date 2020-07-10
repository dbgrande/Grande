#include "plugin.hpp"


struct CVInts : Module {
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

	CVInts() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(A_PARAM, 0, 10, 10, "A Value", "");
		configParam(B_PARAM, 0, 10, 5, "B Value", "");
		configParam(C_PARAM, 0, 10, 1, "C Value", "");
	}

	void process(const ProcessArgs &args) override {

		int a = clamp((int)(params[A_PARAM].getValue()), 0, 10);
		int b = clamp((int)(params[B_PARAM].getValue()), 0, 10);
		int c = clamp((int)(params[C_PARAM].getValue()), 0, 10);

		outputs[A1_OUTPUT].setVoltage(a);
		outputs[A2_OUTPUT].setVoltage(-a);
		outputs[B1_OUTPUT].setVoltage(b);
		outputs[B2_OUTPUT].setVoltage(-b);
		outputs[C1_OUTPUT].setVoltage(c);
		outputs[C2_OUTPUT].setVoltage(-c);
	}
};

struct CVIntsWidget : ModuleWidget {
	CVIntsWidget(CVInts* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CVInts.svg")));

		//addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		//addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundSmallRotarySwitch>(mm2px(Vec(5.08, 17.50)), module, CVInts::A_PARAM));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.08, 29.75)), module, CVInts::A1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.08, 42.00)), module, CVInts::A2_OUTPUT));

		addParam(createParamCentered<RoundSmallRotarySwitch>(mm2px(Vec(5.08, 54.25)), module, CVInts::B_PARAM));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.08, 66.50)), module, CVInts::B1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.08, 78.75)), module, CVInts::B2_OUTPUT));


		addParam(createParamCentered<RoundSmallRotarySwitch>(mm2px(Vec(5.08, 91.00)), module, CVInts::C_PARAM));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.08, 103.25)), module, CVInts::C1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(5.08, 115.50)), module, CVInts::C2_OUTPUT));
	}
};


Model* modelCVInts = createModel<CVInts, CVIntsWidget>("CVInts");

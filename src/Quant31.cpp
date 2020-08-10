#include "plugin.hpp"


struct Quant31 : Module {
	enum ParamIds {
		ROUNDING_PARAM,
		EQUI_PARAM,
		SET_ALL_PARAM,
		CLEAR_ALL_PARAM,
		ENUMS(NOTE_PARAMS, 31),
		NUM_PARAMS
	};
	enum InputIds {
		ROOT_INPUT,
		CV_IN_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		CV_OUT_OUTPUT,
		TRIGGER_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Quant31() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(ROUNDING_PARAM, -1.0, 1.0, 0.0, "Rounding", "");
		configParam(EQUI_PARAM, 0.0, 1.0, 0.0, "Equi-likely notes", "");
		configParam(SET_ALL_PARAM, 0.0, 1.0, 0.0, "Set All", "");
		configParam(CLEAR_ALL_PARAM, 0.0, 1.0, 0.0, "Clear All", "");
		configParam(NOTE_PARAMS, 0.0, 1.0, 1.0, "Note0", "");  // Root note
		configParam(NOTE_PARAMS + 1, 0.0, 1.0, 0.0, "Note1", "");
		configParam(NOTE_PARAMS + 2, 0.0, 1.0, 0.0, "Note2", "");
		configParam(NOTE_PARAMS + 3, 0.0, 1.0, 0.0, "Note3", "");
		configParam(NOTE_PARAMS + 4, 0.0, 1.0, 0.0, "Note4", "");
		configParam(NOTE_PARAMS + 5, 0.0, 1.0, 1.0, "Note5", "");
		configParam(NOTE_PARAMS + 6, 0.0, 1.0, 0.0, "Note6", "");
		configParam(NOTE_PARAMS + 7, 0.0, 1.0, 0.0, "Note7", "");
		configParam(NOTE_PARAMS + 8, 0.0, 1.0, 0.0, "Note8", "");
		configParam(NOTE_PARAMS + 9, 0.0, 1.0, 0.0, "Note9", "");
		configParam(NOTE_PARAMS + 10, 0.0, 1.0, 1.0, "Note10", "");
		configParam(NOTE_PARAMS + 11, 0.0, 1.0, 0.0, "Note11", "");
		configParam(NOTE_PARAMS + 12, 0.0, 1.0, 0.0, "Note12", "");
		configParam(NOTE_PARAMS + 13, 0.0, 1.0, 1.0, "Note13", "");
		configParam(NOTE_PARAMS + 14, 0.0, 1.0, 0.0, "Note14", "");
		configParam(NOTE_PARAMS + 15, 0.0, 1.0, 0.0, "Note15", "");
		configParam(NOTE_PARAMS + 16, 0.0, 1.0, 0.0, "Note16", "");
		configParam(NOTE_PARAMS + 17, 0.0, 1.0, 0.0, "Note17", "");
		configParam(NOTE_PARAMS + 18, 0.0, 1.0, 1.0, "Note18", "");
		configParam(NOTE_PARAMS + 19, 0.0, 1.0, 0.0, "Note19", "");
		configParam(NOTE_PARAMS + 20, 0.0, 1.0, 0.0, "Note20", "");
		configParam(NOTE_PARAMS + 21, 0.0, 1.0, 0.0, "Note21", "");
		configParam(NOTE_PARAMS + 22, 0.0, 1.0, 0.0, "Note22", "");
		configParam(NOTE_PARAMS + 23, 0.0, 1.0, 1.0, "Note23", "");
		configParam(NOTE_PARAMS + 24, 0.0, 1.0, 0.0, "Note24", "");
		configParam(NOTE_PARAMS + 25, 0.0, 1.0, 0.0, "Note25", "");
		configParam(NOTE_PARAMS + 26, 0.0, 1.0, 0.0, "Note26", "");
		configParam(NOTE_PARAMS + 27, 0.0, 1.0, 0.0, "Note27", "");
		configParam(NOTE_PARAMS + 28, 0.0, 1.0, 1.0, "Note28", "");
		configParam(NOTE_PARAMS + 29, 0.0, 1.0, 0.0, "Note29", "");
		configParam(NOTE_PARAMS + 30, 0.0, 1.0, 0.0, "Note30", "");
	}

	dsp::PulseGenerator pulseGenerators[16];

	int param_timer = 0;
	int rounding_mode;
	int equi_likely;
	int scale[32];
	int note_per_oct;
	int lower[31];
	int upper[31];
	float transpose[16];
	float cv_out[16];
	float last_cv_out[16] = { 0.f };

	void process(const ProcessArgs &args) override {
		if (param_timer == 0) {
			// read parameters
			param_timer = 50;  // how often to update params (audio cycles)

			// rounding mode (-1 = down, 0 = nearest, 1 = up)
			rounding_mode = std::round(params[ROUNDING_PARAM].getValue());

			// equally-likely note mode (0 = off, 1 = on)
			// makes the input voltage range for each note equivalent
			equi_likely = std::round(params[EQUI_PARAM].getValue());

			// read set and clear buttons
			int set_all = clamp((int)(params[SET_ALL_PARAM].getValue()), 0, 1);
			int clear_all = clamp((int)(params[CLEAR_ALL_PARAM].getValue()), 0, 1);

			// these will take effect next time around
			if (set_all == 1) {
				for (int i = 0; i < 31; i++)
					params[NOTE_PARAMS + i].setValue(1);
			}
			else if (clear_all == 1) {  // except for root value
				params[NOTE_PARAMS + 0].setValue(1);
				for (int i = 1; i < 31; i++)
					params[NOTE_PARAMS + i].setValue(0);
			}

			// scale is set by left buttons (root on bottom)
			float input_scale[32];
			input_scale[0] = std::round(params[NOTE_PARAMS].getValue());
			for (int i = 1; i < 31; i++) 
				input_scale[i] = std::round(params[NOTE_PARAMS + i].getValue());

			// generate scale[] with enabled notes only
			note_per_oct = 0;
			for (int i = 0, j = 0; i < 31; i++) {
				if (input_scale[i] > 0.5f) {
					scale[j++] = i;
					note_per_oct++;
				}
			}
			// zero notes enabled, equal to just root selected
			if (note_per_oct == 0) {
				note_per_oct = 1;
				scale[0] = 0;
			}
			scale[note_per_oct] = scale[0] + 31;  // for rounding

			// define lookup tables for rounding modes
			int j = (scale[0] == 0) ? 0 : -1;  // adjustment if 1st note not root
			for (int i = 0; i < 31; i++) {
				if (i >= scale[j + 1])
					j++;
				lower[i] = (j < 0) ? scale[note_per_oct - 1] - 31 : scale[j];
				upper[i] = scale[j + 1];
			}

			// transpose to specified root
			int channels = inputs[ROOT_INPUT].getChannels();
			if (channels == 0)  // nothing plugged in, reset all channels to 0
				for (int c = 0; c < 16; c++)
					transpose[c] = 0.f;
			else if (channels == 1) { // mono tranpose, apply to all channels
				float fracT = fmodf(inputs[ROOT_INPUT].getVoltage(0), 1.f);
				if (fracT < 0.f ) // round to -∞
					fracT = (abs(fracT) < 1e-7) ? 0.f : fracT + 1.f;
				float qT = floor(31 * fracT + 0.5f) / 31.f; // quantize to chromatic scale
				for (int c = 0; c < 16; c++)
					transpose[c] = qT;
			}
			else { // full poly, separate transpose per channel
				for (int c = 0; c < channels; c++) {
					float fracT = fmodf(inputs[ROOT_INPUT].getVoltage(c), 1.f);
					if (fracT < 0.f ) // round to -∞
						fracT = (abs(fracT) < 1e-7) ? 0.f : fracT + 1.f;
					transpose[c] = floor(31 * fracT + 0.5f) / 31.f; // quantize to chromatic scale
				}
				for (int c = channels; c < 16; c++)  // zero out remaining
					transpose[c] = 0.f;
			}
		}
		else
			param_timer--;  // just decrement timer

		// quantize cv input (polyphonic)
		int channels = inputs[CV_IN_INPUT].getChannels();
		for (int c = 0; c < channels; c++) {
			float intPart;
			float fracPart = modff(inputs[CV_IN_INPUT].getVoltage(c) - transpose[c], &intPart);
			if (intPart < 0.f || fracPart < 0.f) { // round to -∞
				if (abs(fracPart) < 1e-7)
					fracPart = 0.f;
				else {
					fracPart += 1.f;
					intPart -= 1.f;
				}
			}
			int note;
			if (equi_likely == 0) {  // normal mode
				// quantize by half step, to match to VCV QNT and ML Quantum
				fracPart = 31.f * fracPart;
				if (rounding_mode == -1) { // round down
					int n = floor(fracPart);
					if (fracPart > upper[n] - 0.5)
						note = upper[n];
					else
						note = lower[n];
				}
				else if (rounding_mode == 1) { // round up
					int n = floor(fracPart);
					if (fracPart > lower[n] + 0.5)
						note = upper[n];
					else
						note = lower[n];
				}
				else {  // round nearest (down)
					int n = floor(fracPart);
					float temp = (lower[n] + upper[n]) / 2.f;
					float threshold;
					// if threshold between notes, good to go
					if (abs(temp - floor(temp)) > 0.45)
						threshold = temp;
					else // up half a step
						threshold = temp + 0.5;
					if (fracPart > threshold)
						note = upper[n];
					else
						note = lower[n];
				}
			} else {  // equi-likely mode
				// in this case, can't serialize quantizers, so don't need window
				if (rounding_mode == -1) { // round down
					note = scale[(int)(floor(note_per_oct * fracPart))];
				}
				else if (rounding_mode == 1) { // round up
					note = scale[(int)(ceil(note_per_oct * fracPart))];
				}
				else {  // round nearest (down)
					note = scale[(int)(floor(note_per_oct * fracPart + 0.5f))];
				}
			}
			if (note == 31) {
				intPart++;
				note = 0;
			}
			cv_out[c] = intPart + (note / 31.f) + transpose[c];
			outputs[CV_OUT_OUTPUT].setVoltage(cv_out[c], c);
			// generate trigger pulse on note change
			if (cv_out[c] != last_cv_out[c]) {
				pulseGenerators[c].trigger(1e-3f);
				last_cv_out[c] = cv_out[c];
			}
			bool pulse = pulseGenerators[c].process(args.sampleTime);
			outputs[TRIGGER_OUTPUT].setVoltage((pulse ? 10.f : 0.f), c);
			}
		outputs[CV_OUT_OUTPUT].setChannels(channels);
		outputs[TRIGGER_OUTPUT].setChannels(channels);
	}
};


struct Quant31Widget : ModuleWidget {
	Quant31Widget(Quant31* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Quant31.svg")));

		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<ConfigButton>(mm2px(Vec(5.70, 2.50)), module, Quant31::SET_ALL_PARAM));
		addParam(createParam<ConfigButton>(mm2px(Vec(10.00, 2.50)), module, Quant31::CLEAR_ALL_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24.0, 38.0)), module, Quant31::ROOT_INPUT));

		addParam(createParam<CKSSThree>(mm2px(Vec(21.75, 49.0)), module, Quant31::ROUNDING_PARAM));

		addParam(createParam<CKSS>(mm2px(Vec(21.75, 67.0)), module, Quant31::EQUI_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24.0, 85.0)), module, Quant31::CV_IN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(24.0, 100.0)), module, Quant31::CV_OUT_OUTPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(24.0, 115.0)), module, Quant31::TRIGGER_OUTPUT));

		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 6.160)), module, Quant31::NOTE_PARAMS + 30));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 10.031)), module, Quant31::NOTE_PARAMS + 29));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 13.902)), module, Quant31::NOTE_PARAMS + 28));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 17.773)), module, Quant31::NOTE_PARAMS + 27));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 21.644)), module, Quant31::NOTE_PARAMS + 26));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 25.515)), module, Quant31::NOTE_PARAMS + 25));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 29.386)), module, Quant31::NOTE_PARAMS + 24));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 33.257)), module, Quant31::NOTE_PARAMS + 23));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 37.128)), module, Quant31::NOTE_PARAMS + 22));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 40.999)), module, Quant31::NOTE_PARAMS + 21));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 44.870)), module, Quant31::NOTE_PARAMS + 20));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 48.741)), module, Quant31::NOTE_PARAMS + 19));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 52.612)), module, Quant31::NOTE_PARAMS + 18));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 56.483)), module, Quant31::NOTE_PARAMS + 17));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 60.354)), module, Quant31::NOTE_PARAMS + 16));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 64.225)), module, Quant31::NOTE_PARAMS + 15));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 68.096)), module, Quant31::NOTE_PARAMS + 14));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 71.967)), module, Quant31::NOTE_PARAMS + 13));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 75.838)), module, Quant31::NOTE_PARAMS + 12));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 79.709)), module, Quant31::NOTE_PARAMS + 11));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 83.580)), module, Quant31::NOTE_PARAMS + 10));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 87.451)), module, Quant31::NOTE_PARAMS + 9));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 91.322)), module, Quant31::NOTE_PARAMS + 8));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 95.193)), module, Quant31::NOTE_PARAMS + 7));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 99.064)), module, Quant31::NOTE_PARAMS + 6));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 102.935)), module, Quant31::NOTE_PARAMS + 5));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 106.806)), module, Quant31::NOTE_PARAMS + 4));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 110.677)), module, Quant31::NOTE_PARAMS + 3));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 114.548)), module, Quant31::NOTE_PARAMS + 2));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 118.419)), module, Quant31::NOTE_PARAMS + 1));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 122.289)), module, Quant31::NOTE_PARAMS));
	}
};


Model* modelQuant31 = createModel<Quant31, Quant31Widget>("Quant31");

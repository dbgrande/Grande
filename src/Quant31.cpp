#include "plugin.hpp"


struct Quant31 : Module {
	enum ParamIds {
		ROUNDING_PARAM,
		EQUI_PARAM,
		NOTE0_PARAM,
		NOTE1_PARAM,
		NOTE2_PARAM,
		NOTE3_PARAM,
		NOTE4_PARAM,
		NOTE5_PARAM,
		NOTE6_PARAM,
		NOTE7_PARAM,
		NOTE8_PARAM,
		NOTE9_PARAM,
		NOTE10_PARAM,
		NOTE11_PARAM,
		NOTE12_PARAM,
		NOTE13_PARAM,
		NOTE14_PARAM,
		NOTE15_PARAM,
		NOTE16_PARAM,
		NOTE17_PARAM,
		NOTE18_PARAM,
		NOTE19_PARAM,
		NOTE20_PARAM,
		NOTE21_PARAM,
		NOTE22_PARAM,
		NOTE23_PARAM,
		NOTE24_PARAM,
		NOTE25_PARAM,
		NOTE26_PARAM,
		NOTE27_PARAM,
		NOTE28_PARAM,
		NOTE29_PARAM,
		NOTE30_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		TRANSPOSE_INPUT,
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
		configParam(NOTE0_PARAM, 1.0, 1.0, 0.0, "Note0", "");  // Root note always selected
		configParam(NOTE1_PARAM, 0.0, 1.0, 0.0, "Note1", "");
		configParam(NOTE2_PARAM, 0.0, 1.0, 0.0, "Note2", "");
		configParam(NOTE3_PARAM, 0.0, 1.0, 0.0, "Note3", "");
		configParam(NOTE4_PARAM, 0.0, 1.0, 0.0, "Note4", "");
		configParam(NOTE5_PARAM, 0.0, 1.0, 1.0, "Note5", "");
		configParam(NOTE6_PARAM, 0.0, 1.0, 0.0, "Note6", "");
		configParam(NOTE7_PARAM, 0.0, 1.0, 0.0, "Note7", "");
		configParam(NOTE8_PARAM, 0.0, 1.0, 0.0, "Note8", "");
		configParam(NOTE9_PARAM, 0.0, 1.0, 0.0, "Note9", "");
		configParam(NOTE10_PARAM, 0.0, 1.0, 1.0, "Note10", "");
		configParam(NOTE11_PARAM, 0.0, 1.0, 0.0, "Note11", "");
		configParam(NOTE12_PARAM, 0.0, 1.0, 0.0, "Note12", "");
		configParam(NOTE13_PARAM, 0.0, 1.0, 1.0, "Note13", "");
		configParam(NOTE14_PARAM, 0.0, 1.0, 0.0, "Note14", "");
		configParam(NOTE15_PARAM, 0.0, 1.0, 0.0, "Note15", "");
		configParam(NOTE16_PARAM, 0.0, 1.0, 0.0, "Note16", "");
		configParam(NOTE17_PARAM, 0.0, 1.0, 0.0, "Note17", "");
		configParam(NOTE18_PARAM, 0.0, 1.0, 1.0, "Note18", "");
		configParam(NOTE19_PARAM, 0.0, 1.0, 0.0, "Note19", "");
		configParam(NOTE20_PARAM, 0.0, 1.0, 0.0, "Note20", "");
		configParam(NOTE21_PARAM, 0.0, 1.0, 0.0, "Note21", "");
		configParam(NOTE22_PARAM, 0.0, 1.0, 0.0, "Note22", "");
		configParam(NOTE23_PARAM, 0.0, 1.0, 1.0, "Note23", "");
		configParam(NOTE24_PARAM, 0.0, 1.0, 0.0, "Note24", "");
		configParam(NOTE25_PARAM, 0.0, 1.0, 0.0, "Note25", "");
		configParam(NOTE26_PARAM, 0.0, 1.0, 0.0, "Note26", "");
		configParam(NOTE27_PARAM, 0.0, 1.0, 0.0, "Note27", "");
		configParam(NOTE28_PARAM, 0.0, 1.0, 1.0, "Note28", "");
		configParam(NOTE29_PARAM, 0.0, 1.0, 0.0, "Note29", "");
		configParam(NOTE30_PARAM, 0.0, 1.0, 0.0, "Note30", "");
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

			// scale is set by left buttons (root on bottom)
			float input_scale[32];
			input_scale[0] = std::round(params[NOTE0_PARAM].getValue());
			input_scale[1] = std::round(params[NOTE1_PARAM].getValue());
			input_scale[2] = std::round(params[NOTE2_PARAM].getValue());
			input_scale[3] = std::round(params[NOTE3_PARAM].getValue());
			input_scale[4] = std::round(params[NOTE4_PARAM].getValue());
			input_scale[5] = std::round(params[NOTE5_PARAM].getValue());
			input_scale[6] = std::round(params[NOTE6_PARAM].getValue());
			input_scale[7] = std::round(params[NOTE7_PARAM].getValue());
			input_scale[8] = std::round(params[NOTE8_PARAM].getValue());
			input_scale[9] = std::round(params[NOTE9_PARAM].getValue());
			input_scale[10] = std::round(params[NOTE10_PARAM].getValue());
			input_scale[11] = std::round(params[NOTE11_PARAM].getValue());
			input_scale[12] = std::round(params[NOTE12_PARAM].getValue());
			input_scale[13] = std::round(params[NOTE13_PARAM].getValue());
			input_scale[14] = std::round(params[NOTE14_PARAM].getValue());
			input_scale[15] = std::round(params[NOTE15_PARAM].getValue());
			input_scale[16] = std::round(params[NOTE16_PARAM].getValue());
			input_scale[17] = std::round(params[NOTE17_PARAM].getValue());
			input_scale[18] = std::round(params[NOTE18_PARAM].getValue());
			input_scale[19] = std::round(params[NOTE19_PARAM].getValue());
			input_scale[20] = std::round(params[NOTE20_PARAM].getValue());
			input_scale[21] = std::round(params[NOTE21_PARAM].getValue());
			input_scale[22] = std::round(params[NOTE22_PARAM].getValue());
			input_scale[23] = std::round(params[NOTE23_PARAM].getValue());
			input_scale[24] = std::round(params[NOTE24_PARAM].getValue());
			input_scale[25] = std::round(params[NOTE25_PARAM].getValue());
			input_scale[26] = std::round(params[NOTE26_PARAM].getValue());
			input_scale[27] = std::round(params[NOTE27_PARAM].getValue());
			input_scale[28] = std::round(params[NOTE28_PARAM].getValue());
			input_scale[29] = std::round(params[NOTE29_PARAM].getValue());
			input_scale[30] = std::round(params[NOTE30_PARAM].getValue());

			scale[0] = 0;  // always enable root note
			note_per_oct = 1;
			int j = 1; 
			for (int i = 1; i < 31; i++) {
				if (input_scale[i] > 0.5) {
					scale[j++] = i;
					note_per_oct++;
				}
			}
			scale[note_per_oct] = 31;  // for rounding

			// define lookup tables for rounding modes
			for (int i = 0, j = 0; i < 31; i++) {
				if (i >= scale[j + 1])
					j++;
				lower[i] = scale[j];
				upper[i] = scale[j + 1];
			}

			// transpose
			int channels = inputs[TRANSPOSE_INPUT].getChannels();
			if (channels == 0)  // nothing plugged in, reset all channels to 0
				for (int c = 0; c < 16; c++)
					transpose[c] = 0.f;
			else if (channels == 1) { // mono tranpose, apply to all channels
				float t = inputs[TRANSPOSE_INPUT].getVoltage(0);
				for (int c = 0; c < 16; c++)
					transpose[c] = t;
			}
			else { // full poly, separate transpose per channel
				for (int c = 0; c < channels; c++)
					transpose[c] = inputs[TRANSPOSE_INPUT].getVoltage(c);
				for (int c = channels; c < 16; c++)  // zero out remaining
					transpose[c] = 0.f;
			}
		}
		else
			param_timer--;  // just decrement timer

		// quantize cv input (polyphonic)
		int channels = inputs[CV_IN_INPUT].getChannels();
		for (int c = 0; c < channels; c++) {
			float rawnote = inputs[CV_IN_INPUT].getVoltage(c);
			int octave = floor(rawnote);
			float freq = 31.f * (rawnote - octave);
			int n = floor(freq);
			int note;
			if (rounding_mode == -1)  // round down
				if (equi_likely == 1)
					note = scale[(int)(floor(note_per_oct * (rawnote - octave)))];
				else
					note = lower[n];
			else if (rounding_mode == 1)  // round up
				if (equi_likely == 1)
					note = scale[(int)(ceil(note_per_oct * (rawnote - octave)))];
				else
					note = upper[n];
			else {  // round nearest
				if (equi_likely == 1)
					note = scale[(int)(floor(note_per_oct * (rawnote - octave) + 0.5f))];
				else {
					float thresh = (lower[n] + upper[n]) / 2.f;
					if (freq >= thresh)
						note = upper[n];
					else
						note = lower[n];
				}
			}
			if (note == 31) {
				octave++;
				note = 0;
			}
			// output
			cv_out[c] = octave + (note / 31.f);
			outputs[CV_OUT_OUTPUT].setVoltage(cv_out[c] + transpose[c], c);
			// generate trigger pulse on note change, but don't consider transpose
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

		// addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		// addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24.0, 38.0)), module, Quant31::TRANSPOSE_INPUT));

		addParam(createParam<CKSSThree>(mm2px(Vec(21.75, 49.0)), module, Quant31::ROUNDING_PARAM));

		addParam(createParam<CKSS>(mm2px(Vec(21.75, 67.0)), module, Quant31::EQUI_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24.0, 85.0)), module, Quant31::CV_IN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(24.0, 100.0)), module, Quant31::CV_OUT_OUTPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(24.0, 115.0)), module, Quant31::TRIGGER_OUTPUT));

		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 6.160)), module, Quant31::NOTE30_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 10.031)), module, Quant31::NOTE29_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 13.902)), module, Quant31::NOTE28_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 17.773)), module, Quant31::NOTE27_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 21.644)), module, Quant31::NOTE26_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 25.515)), module, Quant31::NOTE25_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 29.386)), module, Quant31::NOTE24_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 33.257)), module, Quant31::NOTE23_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 37.128)), module, Quant31::NOTE22_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 40.999)), module, Quant31::NOTE21_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 44.870)), module, Quant31::NOTE20_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 48.741)), module, Quant31::NOTE19_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 52.612)), module, Quant31::NOTE18_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 56.483)), module, Quant31::NOTE17_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 60.354)), module, Quant31::NOTE16_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 64.225)), module, Quant31::NOTE15_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 68.096)), module, Quant31::NOTE14_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 71.967)), module, Quant31::NOTE13_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 75.838)), module, Quant31::NOTE12_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 79.709)), module, Quant31::NOTE11_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 83.580)), module, Quant31::NOTE10_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 87.451)), module, Quant31::NOTE9_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 91.322)), module, Quant31::NOTE8_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 95.193)), module, Quant31::NOTE7_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 99.064)), module, Quant31::NOTE6_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 102.935)), module, Quant31::NOTE5_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 106.806)), module, Quant31::NOTE4_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 110.677)), module, Quant31::NOTE3_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 114.548)), module, Quant31::NOTE2_PARAM));
		addParam(createParam<PentaButton>(mm2px(Vec(5.8, 118.419)), module, Quant31::NOTE1_PARAM));
		addParam(createParam<PentaButtonRoot>(mm2px(Vec(5.8, 122.289)), module, Quant31::NOTE0_PARAM));
	}
};


Model* modelQuant31 = createModel<Quant31, Quant31Widget>("Quant31");

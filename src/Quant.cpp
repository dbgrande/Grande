#include "plugin.hpp"


struct Quant : Module {
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
		NUM_PARAMS
	};
	enum InputIds {
		ROOT_INPUT,
		SCALE_INPUT,
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

	Quant() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(ROUNDING_PARAM, -1.0, 1.0, 0.0, "Rounding", "");
		configParam(EQUI_PARAM, 0.0, 1.0, 0.0, "Equi-likely notes", "");
		configParam(NOTE0_PARAM, 1.0, 1.0, 1.0, "Note0", "");  // Root note always selected
		configParam(NOTE1_PARAM, 0.0, 1.0, 0.0, "Note1", "");
		configParam(NOTE2_PARAM, 0.0, 1.0, 1.0, "Note2", "");
		configParam(NOTE3_PARAM, 0.0, 1.0, 0.0, "Note3", "");
		configParam(NOTE4_PARAM, 0.0, 1.0, 1.0, "Note4", "");
		configParam(NOTE5_PARAM, 0.0, 1.0, 1.0, "Note5", "");
		configParam(NOTE6_PARAM, 0.0, 1.0, 0.0, "Note6", "");
		configParam(NOTE7_PARAM, 0.0, 1.0, 1.0, "Note7", "");
		configParam(NOTE8_PARAM, 0.0, 1.0, 0.0, "Note8", "");
		configParam(NOTE9_PARAM, 0.0, 1.0, 1.0, "Note9", "");
		configParam(NOTE10_PARAM, 0.0, 1.0, 0.0, "Note10", "");
		configParam(NOTE11_PARAM, 0.0, 1.0, 1.0, "Note11", "");
	}

	dsp::PulseGenerator pulseGenerators[16];

	int param_timer = 0;
	int rounding_mode;
	int equi_likely;
	int scale[13];
	int note_per_oct;
	int lower[12];
	int upper[12];
	float input_scale[12];
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

			// scale can either be set by left buttons (root on bottom)
			// or sent on 12 poly channels: 1 (root) to 12
			int schannels = inputs[SCALE_INPUT].getChannels();
			if (schannels > 12)
				schannels = 12;
			if (schannels > 0) {  // read scale input
				for (int c = 0; c < schannels; c++)
					input_scale[c] = inputs[SCALE_INPUT].getVoltage(c);
				// display external scale on buttons
				params[NOTE1_PARAM].setValue((input_scale[1] >= 1.f) ? 1 : 0);
				params[NOTE2_PARAM].setValue((input_scale[2] >= 1.f) ? 1 : 0);
				params[NOTE3_PARAM].setValue((input_scale[3] >= 1.f) ? 1 : 0);
				params[NOTE4_PARAM].setValue((input_scale[4] >= 1.f) ? 1 : 0);
				params[NOTE5_PARAM].setValue((input_scale[5] >= 1.f) ? 1 : 0);
				params[NOTE6_PARAM].setValue((input_scale[6] >= 1.f) ? 1 : 0);
				params[NOTE7_PARAM].setValue((input_scale[7] >= 1.f) ? 1 : 0);
				params[NOTE8_PARAM].setValue((input_scale[8] >= 1.f) ? 1 : 0);
				params[NOTE9_PARAM].setValue((input_scale[9] >= 1.f) ? 1 : 0);
				params[NOTE10_PARAM].setValue((input_scale[10] >= 1.f) ? 1 : 0);
				params[NOTE11_PARAM].setValue((input_scale[11] >= 1.f) ? 1 : 0);
			}
			else {
				// no external scale, so read buttons (root on bottom)
				input_scale[0] = 10 * std::round(params[NOTE0_PARAM].getValue());
				input_scale[1] = 10 * std::round(params[NOTE1_PARAM].getValue());
				input_scale[2] = 10 * std::round(params[NOTE2_PARAM].getValue());
				input_scale[3] = 10 * std::round(params[NOTE3_PARAM].getValue());
				input_scale[4] = 10 * std::round(params[NOTE4_PARAM].getValue());
				input_scale[5] = 10 * std::round(params[NOTE5_PARAM].getValue());
				input_scale[6] = 10 * std::round(params[NOTE6_PARAM].getValue());
				input_scale[7] = 10 * std::round(params[NOTE7_PARAM].getValue());
				input_scale[8] = 10 * std::round(params[NOTE8_PARAM].getValue());
				input_scale[9] = 10 * std::round(params[NOTE9_PARAM].getValue());
				input_scale[10] = 10 * std::round(params[NOTE10_PARAM].getValue());
				input_scale[11] = 10 * std::round(params[NOTE11_PARAM].getValue());
			}
			scale[0] = 0;  // always enable root note
			note_per_oct = 1;
			int j = 1; 
			for (int i = 1; i < 12; i++) {
				if (input_scale[i] > 2.5) {
					scale[j++] = i;
					note_per_oct++;
				}
			}
			scale[note_per_oct] = 12;  // for rounding

			// define lookup tables for rounding modes
			for (int i = 0, j = 0; i < 12; i++) {
				if (i >= scale[j + 1])
					j++;
				lower[i] = scale[j];
				upper[i] = scale[j + 1];
			}

			// transpose to specified root
			int channels = inputs[ROOT_INPUT].getChannels();
			if (channels == 0)  // nothing plugged in, reset all channels to 0
				for (int c = 0; c < 16; c++)
					transpose[c] = 0.f;
			else if (channels == 1) { // mono tranpose, apply to all channels
				float t = inputs[ROOT_INPUT].getVoltage(0);
				for (int c = 0; c < 16; c++)
					transpose[c] = t;
			}
			else { // full poly, separate transpose per channel
				for (int c = 0; c < channels; c++)
					transpose[c] = inputs[ROOT_INPUT].getVoltage(c);
				for (int c = channels; c < 16; c++)  // zero out remaining
					transpose[c] = 0.f;
			}
		}
		else
			param_timer--;  // just decrement timer

		// quantize cv input (polyphonic)
		int channels = inputs[CV_IN_INPUT].getChannels();
		for (int c = 0; c < channels; c++) {
			float rawnote = inputs[CV_IN_INPUT].getVoltage(c) - transpose[c];
			int octave = floor(rawnote);
			float freq = 12.f * (rawnote - octave);
			int n = floor(freq);
			int note;
			if (equi_likely == 0) {  // normal mode
				if (rounding_mode == -1)  // round down
					note = lower[n];
				else if (rounding_mode == 1)  // round up
					note = upper[n];
				else {  // round nearest
					if (freq >= (lower[n] + upper[n]) / 2.f)  // threshold
						note = upper[n];
					else
						note = lower[n];
				}
			} else {  // equi-likely mode
				if (rounding_mode == -1)  // round down
					note = scale[(int)(floor(note_per_oct * (rawnote - octave)))];
				else if (rounding_mode == 1)  // round up
					note = scale[(int)(ceil(note_per_oct * (rawnote - octave)))];
				else {  // round nearest
					note = scale[(int)(floor(note_per_oct * (rawnote - octave) + 0.5f))];
				}
			}
			if (note == 12) {
				octave++;
				note = 0;
			}
			// output
			cv_out[c] = octave + (note / 12.f) + transpose[c];
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


struct QuantWidget : ModuleWidget {
	QuantWidget(Quant* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Quant.svg")));

		//addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(0, 0)));
		//addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		//addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		//addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 1 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.45, 23.0)), module, Quant::SCALE_INPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.45, 38.0)), module, Quant::ROOT_INPUT));

		addParam(createParam<CKSSThree>(mm2px(Vec(12.20, 49.0)), module, Quant::ROUNDING_PARAM));

		addParam(createParam<CKSS>(mm2px(Vec(12.20, 67.0)), module, Quant::EQUI_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(14.45, 85.0)), module, Quant::CV_IN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(14.45, 100.0)), module, Quant::CV_OUT_OUTPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(14.45, 115.0)), module, Quant::TRIGGER_OUTPUT));

		addParam(createParam<WhiteButton>(mm2px(Vec(1.58, 25.0)), module, Quant::NOTE11_PARAM));
		addParam(createParam<BlackButton>(mm2px(Vec(1.58, 33.0)), module, Quant::NOTE10_PARAM));
		addParam(createParam<WhiteButton>(mm2px(Vec(1.58, 41.0)), module, Quant::NOTE9_PARAM));
		addParam(createParam<BlackButton>(mm2px(Vec(1.58, 49.0)), module, Quant::NOTE8_PARAM));
		addParam(createParam<WhiteButton>(mm2px(Vec(1.58, 57.0)), module, Quant::NOTE7_PARAM));
		addParam(createParam<BlackButton>(mm2px(Vec(1.58, 65.0)), module, Quant::NOTE6_PARAM));
		addParam(createParam<WhiteButton>(mm2px(Vec(1.58, 73.0)), module, Quant::NOTE5_PARAM));
		addParam(createParam<WhiteButton>(mm2px(Vec(1.58, 81.0)), module, Quant::NOTE4_PARAM));
		addParam(createParam<BlackButton>(mm2px(Vec(1.58, 89.0)), module, Quant::NOTE3_PARAM));
		addParam(createParam<WhiteButton>(mm2px(Vec(1.58, 97.0)), module, Quant::NOTE2_PARAM));
		addParam(createParam<BlackButton>(mm2px(Vec(1.58, 105.0)), module, Quant::NOTE1_PARAM));
		addParam(createParam<WhiteButtonRoot>(mm2px(Vec(1.58, 113.0)), module, Quant::NOTE0_PARAM));
	}
};


Model* modelQuant = createModel<Quant, QuantWidget>("Quant");

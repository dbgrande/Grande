#include "plugin.hpp"


struct QuantMT : Module {
	enum ParamIds {
		ROUNDING_PARAM,
		EQUI_PARAM,
		SIZE_PARAM,
		SEL_ALL_PARAM,
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
		ENUMS(OCTAVE_LIGHTS, 32),
		NUM_LIGHTS
	};

	QuantMT() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(ROUNDING_PARAM, -1.0, 1.0, 0.0, "Rounding", "");
		configParam(EQUI_PARAM, 0.0, 1.0, 0.0, "Equi-likely notes", "");
		configParam(SIZE_PARAM, 1, 31, 12, "Notes per Octave", "");
		configParam(NOTE_PARAMS, 0.0, 1.0, 1.0, "Note0", "");  // Root
		configParam(NOTE_PARAMS + 1, 0.0, 1.0, 0.0, "Note1", "");
		configParam(NOTE_PARAMS + 2, 0.0, 1.0, 1.0, "Note2", "");
		configParam(NOTE_PARAMS + 3, 0.0, 1.0, 0.0, "Note3", "");
		configParam(NOTE_PARAMS + 4, 0.0, 1.0, 1.0, "Note4", "");
		configParam(NOTE_PARAMS + 5, 0.0, 1.0, 1.0, "Note5", "");
		configParam(NOTE_PARAMS + 6, 0.0, 1.0, 0.0, "Note6", "");
		configParam(NOTE_PARAMS + 7, 0.0, 1.0, 1.0, "Note7", "");
		configParam(NOTE_PARAMS + 8, 0.0, 1.0, 0.0, "Note8", "");
		configParam(NOTE_PARAMS + 9, 0.0, 1.0, 1.0, "Note9", "");
		configParam(NOTE_PARAMS + 10, 0.0, 1.0, 0.0, "Note10", "");
		configParam(NOTE_PARAMS + 11, 0.0, 1.0, 1.0, "Note11", "");
		configParam(NOTE_PARAMS + 12, 0.0, 1.0, 0.0, "Note12", "");
		configParam(NOTE_PARAMS + 13, 0.0, 1.0, 0.0, "Note13", "");
		configParam(NOTE_PARAMS + 14, 0.0, 1.0, 0.0, "Note14", "");
		configParam(NOTE_PARAMS + 15, 0.0, 1.0, 0.0, "Note15", "");
		configParam(NOTE_PARAMS + 16, 0.0, 1.0, 0.0, "Note16", "");
		configParam(NOTE_PARAMS + 17, 0.0, 1.0, 0.0, "Note17", "");
		configParam(NOTE_PARAMS + 18, 0.0, 1.0, 0.0, "Note18", "");
		configParam(NOTE_PARAMS + 19, 0.0, 1.0, 0.0, "Note19", "");
		configParam(NOTE_PARAMS + 20, 0.0, 1.0, 0.0, "Note20", "");
		configParam(NOTE_PARAMS + 21, 0.0, 1.0, 0.0, "Note21", "");
		configParam(NOTE_PARAMS + 22, 0.0, 1.0, 0.0, "Note22", "");
		configParam(NOTE_PARAMS + 23, 0.0, 1.0, 0.0, "Note23", "");
		configParam(NOTE_PARAMS + 24, 0.0, 1.0, 0.0, "Note24", "");
		configParam(NOTE_PARAMS + 25, 0.0, 1.0, 0.0, "Note25", "");
		configParam(NOTE_PARAMS + 26, 0.0, 1.0, 0.0, "Note26", "");
		configParam(NOTE_PARAMS + 27, 0.0, 1.0, 0.0, "Note27", "");
		configParam(NOTE_PARAMS + 28, 0.0, 1.0, 0.0, "Note28", "");
		configParam(NOTE_PARAMS + 29, 0.0, 1.0, 0.0, "Note29", "");
		configParam(NOTE_PARAMS + 30, 0.0, 1.0, 0.0, "Note30", "");
		configParam(SEL_ALL_PARAM, 0.0, 1.0, 0.0, "Set All", "");
		configParam(CLEAR_ALL_PARAM, 0.0, 1.0, 0.0, "Clear All", "");
	}

	dsp::PulseGenerator pulseGenerators[16];

	int param_timer = 0;
	int rounding_mode;
	int equi_likely;
	int equal_temp;
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

			// equal temperament size
			equal_temp = clamp((int)(params[SIZE_PARAM].getValue()), 1, 31);

			// set all and clear all buttons
			int sel_all = clamp((int)(params[SEL_ALL_PARAM].getValue()), 0, 1);
			int clear_all = clamp((int)(params[CLEAR_ALL_PARAM].getValue()), 0, 1);
			if (sel_all == 1) {
				for (int i = 0; i < 31; i++)
					params[NOTE_PARAMS + i].setValue(1);
			}
			else if (clear_all == 1) {  // except for root value
				params[NOTE_PARAMS + 0].setValue(1);
				for (int i = 1; i < 31; i++)
					params[NOTE_PARAMS + i].setValue(0);
			}

			// scale is set by interval buttons
			float input_scale[32];
			for (int i = 0; i < 31; i++)
				input_scale[i] = std::round(params[NOTE_PARAMS + i].getValue());

			// lights show root and top note
			for (int i = 0; i < 32; i++)
				if (i == 0 || i == equal_temp)
					lights[OCTAVE_LIGHTS + i].setBrightness(true);
				else
					lights[OCTAVE_LIGHTS + i].setBrightness(false);

			// generate scale[] with enabled notes up to equal_temp size
			note_per_oct = 0;
			for (int i = 0, j = 0; i < equal_temp; i++) {
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
			scale[note_per_oct] = scale[0] + equal_temp;  // for rounding

			// define lookup tables for rounding modes
			int j = (scale[0] == 0) ? 0 : -1;  // adjustment if 1st note not root
			for (int i = 0; i < equal_temp; i++) {
				if (i >= scale[j + 1])
					j++;
				lower[i] = (j < 0) ? scale[note_per_oct - 1] - equal_temp : scale[j];
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
				float qT = floor(equal_temp * fracT + 0.5f) / equal_temp; // quantize to chromatic scale
				for (int c = 0; c < 16; c++)
					transpose[c] = qT;
			}
			else { // full poly, separate transpose per channel
				for (int c = 0; c < channels; c++) {
					float fracT = fmodf(inputs[ROOT_INPUT].getVoltage(c), 1.f);
					if (fracT < 0.f ) // round to -∞
						fracT = (abs(fracT) < 1e-7) ? 0.f : fracT + 1.f;
					transpose[c] = floor(equal_temp * fracT + 0.5f) / equal_temp; // quantize to chromatic scale
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
				fracPart = equal_temp * fracPart;
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
			if (note == equal_temp) {
				intPart++;
				note = 0;
			}
			cv_out[c] = intPart + ((float) note / equal_temp) + transpose[c];
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


struct QuantMTWidget : ModuleWidget {
	QuantMTWidget(QuantMT* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/QuantMT.svg")));

		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 3 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackRotarySwitch>(mm2px(Vec(22.24, 38.50)), module, QuantMT::SIZE_PARAM));

		addParam(createParam<TL1105>(mm2px(Vec(18.5-2.709, 23.5-2.709)), module, QuantMT::SEL_ALL_PARAM));
		addParam(createParam<TL1105>(mm2px(Vec(26.0-2.709, 23.5-2.709)), module, QuantMT::CLEAR_ALL_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.24, 54.0)), module, QuantMT::ROOT_INPUT));

		addParam(createParam<CKSSThree>(mm2px(Vec(16.25, 65.5)), module, QuantMT::ROUNDING_PARAM));
		addParam(createParam<CKSS>(mm2px(Vec(23.75, 67.0)), module, QuantMT::EQUI_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.24, 85.0)), module, QuantMT::CV_IN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.24, 100.0)), module, QuantMT::CV_OUT_OUTPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.24, 115.0)), module, QuantMT::TRIGGER_OUTPUT));

		addParam(createParam<RectButton>(mm2px(Vec(5.2, 6.160)), module, QuantMT::NOTE_PARAMS + 30));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 10.031)), module, QuantMT::NOTE_PARAMS + 29));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 13.902)), module, QuantMT::NOTE_PARAMS + 28));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 17.773)), module, QuantMT::NOTE_PARAMS + 27));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 21.644)), module, QuantMT::NOTE_PARAMS + 26));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 25.515)), module, QuantMT::NOTE_PARAMS + 25));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 29.386)), module, QuantMT::NOTE_PARAMS + 24));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 33.257)), module, QuantMT::NOTE_PARAMS + 23));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 37.128)), module, QuantMT::NOTE_PARAMS + 22));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 40.999)), module, QuantMT::NOTE_PARAMS + 21));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 44.870)), module, QuantMT::NOTE_PARAMS + 20));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 48.741)), module, QuantMT::NOTE_PARAMS + 19));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 52.612)), module, QuantMT::NOTE_PARAMS + 18));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 56.483)), module, QuantMT::NOTE_PARAMS + 17));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 60.354)), module, QuantMT::NOTE_PARAMS + 16));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 64.225)), module, QuantMT::NOTE_PARAMS + 15));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 68.096)), module, QuantMT::NOTE_PARAMS + 14));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 71.967)), module, QuantMT::NOTE_PARAMS + 13));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 75.838)), module, QuantMT::NOTE_PARAMS + 12));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 79.709)), module, QuantMT::NOTE_PARAMS + 11));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 83.580)), module, QuantMT::NOTE_PARAMS + 10));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 87.451)), module, QuantMT::NOTE_PARAMS + 9));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 91.322)), module, QuantMT::NOTE_PARAMS + 8));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 95.193)), module, QuantMT::NOTE_PARAMS + 7));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 99.064)), module, QuantMT::NOTE_PARAMS + 6));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 102.935)), module, QuantMT::NOTE_PARAMS + 5));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 106.806)), module, QuantMT::NOTE_PARAMS + 4));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 110.677)), module, QuantMT::NOTE_PARAMS + 3));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 114.548)), module, QuantMT::NOTE_PARAMS + 2));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 118.419)), module, QuantMT::NOTE_PARAMS + 1));
		addParam(createParam<RectButton>(mm2px(Vec(5.2, 122.289)), module, QuantMT::NOTE_PARAMS + 0));

		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 2.289+3.60)), module, QuantMT::OCTAVE_LIGHTS + 31));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 6.160+3.60)), module, QuantMT::OCTAVE_LIGHTS + 30));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 10.031+3.60)), module, QuantMT::OCTAVE_LIGHTS + 29));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 13.902+3.60)), module, QuantMT::OCTAVE_LIGHTS + 28));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 17.773+3.60)), module, QuantMT::OCTAVE_LIGHTS + 27));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 21.644+3.60)), module, QuantMT::OCTAVE_LIGHTS + 26));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 25.515+3.60)), module, QuantMT::OCTAVE_LIGHTS + 25));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 29.386+3.60)), module, QuantMT::OCTAVE_LIGHTS + 24));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 33.257+3.60)), module, QuantMT::OCTAVE_LIGHTS + 23));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 37.128+3.60)), module, QuantMT::OCTAVE_LIGHTS + 22));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 40.999+3.60)), module, QuantMT::OCTAVE_LIGHTS + 21));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 44.870+3.60)), module, QuantMT::OCTAVE_LIGHTS + 20));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 48.741+3.60)), module, QuantMT::OCTAVE_LIGHTS + 19));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 52.612+3.60)), module, QuantMT::OCTAVE_LIGHTS + 18));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 56.483+3.60)), module, QuantMT::OCTAVE_LIGHTS + 17));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 60.354+3.60)), module, QuantMT::OCTAVE_LIGHTS + 16));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 64.225+3.60)), module, QuantMT::OCTAVE_LIGHTS + 15));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 68.096+3.60)), module, QuantMT::OCTAVE_LIGHTS + 14));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 71.967+3.60)), module, QuantMT::OCTAVE_LIGHTS + 13));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 75.838+3.60)), module, QuantMT::OCTAVE_LIGHTS + 12));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 79.709+3.60)), module, QuantMT::OCTAVE_LIGHTS + 11));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 83.580+3.60)), module, QuantMT::OCTAVE_LIGHTS + 10));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 87.451+3.60)), module, QuantMT::OCTAVE_LIGHTS + 9));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 91.322+3.60)), module, QuantMT::OCTAVE_LIGHTS + 8));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 95.193+3.60)), module, QuantMT::OCTAVE_LIGHTS + 7));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 99.064+3.60)), module, QuantMT::OCTAVE_LIGHTS + 6));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 102.935+3.60)), module, QuantMT::OCTAVE_LIGHTS + 5));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 106.806+3.60)), module, QuantMT::OCTAVE_LIGHTS + 4));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 110.677+3.60)), module, QuantMT::OCTAVE_LIGHTS + 3));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 114.548+3.60)), module, QuantMT::OCTAVE_LIGHTS + 2));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 118.419+3.60)), module, QuantMT::OCTAVE_LIGHTS + 1));
		addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(13.60, 122.289+3.60)), module, QuantMT::OCTAVE_LIGHTS));
	}
};

Model* modelQuantMT = createModel<QuantMT, QuantMTWidget>("QuantMT");

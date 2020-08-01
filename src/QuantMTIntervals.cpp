#include "plugin.hpp"


struct QuantMTIntervals : Module {
	enum ParamIds {
		ROUNDING_PARAM,
		EQUI_PARAM,
		SIZE_PARAM,
		ENUMS(INTERVAL_PARAMS, 32),
		TOLERANCE_PARAM,
		SEL_ALL_PARAM,
		CLEAR_ALL_PARAM,
		SEL_ENABLED_PARAM,
		SHOW_ALLOWED_PARAM,
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
		ENUMS(INTERVAL_LIGHTS, 32),
		NUM_LIGHTS
	};

	QuantMTIntervals() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(ROUNDING_PARAM, -1.0, 1.0, 0.0, "Rounding", "");
		configParam(EQUI_PARAM, 0.0, 1.0, 0.0, "Equi-likely notes", "");
		configParam(SIZE_PARAM, 1, 31, 12, "Notes per Octave", "");
		configParam(INTERVAL_PARAMS, 0.0, 1.0, 1.0, "0.0", "");  // Unison
		configParam(INTERVAL_PARAMS + 1, 0.0, 1.0, 0.0, "70.7", "");
		configParam(INTERVAL_PARAMS + 2, 0.0, 1.0, 0.0, "111.7", "");
		configParam(INTERVAL_PARAMS + 3, 0.0, 1.0, 0.0, "133.2", "");
		configParam(INTERVAL_PARAMS + 4, 0.0, 1.0, 0.0, "182.4", "");
		configParam(INTERVAL_PARAMS + 5, 0.0, 1.0, 1.0, "203.9", "");
		configParam(INTERVAL_PARAMS + 6, 0.0, 1.0, 0.0, "231.2", "");
		configParam(INTERVAL_PARAMS + 7, 0.0, 1.0, 0.0, "266.9", "");
		configParam(INTERVAL_PARAMS + 8, 0.0, 1.0, 0.0, "315.6", "");
		configParam(INTERVAL_PARAMS + 9, 0.0, 1.0, 1.0, "386.3", "");
		configParam(INTERVAL_PARAMS + 10, 0.0, 1.0, 0.0, "435.1", "");
		configParam(INTERVAL_PARAMS + 11, 0.0, 1.0, 1.0, "498.0", "");
		configParam(INTERVAL_PARAMS + 12, 0.0, 1.0, 0.0, "519.6", "");
		configParam(INTERVAL_PARAMS + 13, 0.0, 1.0, 0.0, "568.7", "");
		configParam(INTERVAL_PARAMS + 14, 0.0, 1.0, 0.0, "582.5", "");
		configParam(INTERVAL_PARAMS + 15, 0.0, 1.0, 0.0, "617.5", "");
		configParam(INTERVAL_PARAMS + 16, 0.0, 1.0, 0.0, "680.4", "");
		configParam(INTERVAL_PARAMS + 17, 0.0, 1.0, 1.0, "702.0", "");
		configParam(INTERVAL_PARAMS + 18, 0.0, 1.0, 0.0, "764.9", "");
		configParam(INTERVAL_PARAMS + 19, 0.0, 1.0, 0.0, "772.6", "");
		configParam(INTERVAL_PARAMS + 20, 0.0, 1.0, 0.0, "813.7", "");
		configParam(INTERVAL_PARAMS + 21, 0.0, 1.0, 1.0, "884.4", "");
		configParam(INTERVAL_PARAMS + 22, 0.0, 1.0, 0.0, "933.1", "");
		configParam(INTERVAL_PARAMS + 23, 0.0, 1.0, 0.0, "968.8", "");
		configParam(INTERVAL_PARAMS + 24, 0.0, 1.0, 0.0, "996.1", "");
		configParam(INTERVAL_PARAMS + 25, 0.0, 1.0, 0.0, "1017.6", "");
		configParam(INTERVAL_PARAMS + 26, 0.0, 1.0, 1.0, "1088.3", "");
		configParam(INTERVAL_PARAMS + 27, 0.0, 1.0, 0.0, "1137.0", "");
		configParam(INTERVAL_PARAMS + 28, 0.0, 1.0, 0.0, "1151.2", "");
		configParam(INTERVAL_PARAMS + 29, 0.0, 1.0, 0.0, "Interval29", "");
		configParam(INTERVAL_PARAMS + 30, 0.0, 1.0, 0.0, "Interval30", "");
		configParam(INTERVAL_PARAMS + 31, 0.0, 1.0, 0.0, "Interval31", "");
		configParam(TOLERANCE_PARAM, 0.0, 50.0, 20.0, "Tolerance", "¢");
		configParam(SEL_ALL_PARAM, 0.0, 1.0, 0.0, "Set All", "");
		configParam(CLEAR_ALL_PARAM, 0.0, 1.0, 0.0, "Clear All", "");
		configParam(SHOW_ALLOWED_PARAM, 0.0, 1.0, 0.0, "Show Valid", "");
		configParam(SEL_ENABLED_PARAM, 0.0, 1.0, 0.0, "Clear Invalid", "");
	}

	dsp::PulseGenerator pulseGenerators[16];

	int param_timer = 0;
	int rounding_mode;
	int equi_likely;
	int equal_temp;
	float step_size;
	float tolerance;
	int scale[32];
	int note_per_oct;
	int lower[31];
	int upper[31];
	float transpose[16];
	float cv_out[16];
	float last_cv_out[16] = { 0.f };
	// 1:1, 25:24, 16:15, 27:25, 10:9, 9:8, 8:7, 7:6,
	// 6:5, 5:4, 9:7, 4:3, 27:20, 25:18, 7:5, 10:7, 40:27, 3:2,
	// 14:9, 25:16, 8:5, 5:3, 12:7, 7:4, 16:9, 9:5,
	// 15:8, 27:14, 35:18
	float interval_lu[30] = { 0.f, 0.0588937f, 0.0931094f, 0.1110313f, 0.1520031f, 0.1699250f, 0.1926451f, 0.2223924f,
		0.2630344f, 0.3219281f, 0.3625701f, 0.4150375f, 0.4329594f, 0.4739312f, 0.4854268f, 0.5145732f, 0.5670406f, 0.5849625f,
		0.6374299f, 0.6438562f, 0.6780719f, 0.7369656f, 0.7776076f, 0.8073549f, 0.8300750f, 0.8479969f,
		0.9068906f, 0.9475326f, 0.9593580f, 1.f };
	int num_intervals = 29;

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
			step_size = 1.f / equal_temp;

			// tolerance of note matching interval
			tolerance = params[TOLERANCE_PARAM].getValue();  // read in 0-50¢
			tolerance = tolerance / 1200;  // convert ¢ to V

			// read interval buttons
			int interval_inputs[32];
			// break out bit 0 to avoid uninitialized warning
			interval_inputs[0] = clamp((int)(params[INTERVAL_PARAMS + 0].getValue()), 0, 1);
			for (int i = 1; i < num_intervals; i++)
				interval_inputs[i] = clamp((int)(params[INTERVAL_PARAMS + i].getValue()), 0, 1);
			interval_inputs[num_intervals] = interval_inputs[0];  // map unison to octave

			int sel_all = clamp((int)(params[SEL_ALL_PARAM].getValue()), 0, 1);
			int clear_all = clamp((int)(params[CLEAR_ALL_PARAM].getValue()), 0, 1);
			int sel_enabled = clamp((int)(params[SEL_ENABLED_PARAM].getValue()), 0, 1);
			int show_allowed = clamp((int)(params[SHOW_ALLOWED_PARAM].getValue()), 0, 1);

			// initialize
			int note_used[32];  // include octave
			for (int i = 0; i < 32; i++)
				note_used[i] = -1;  // -1 == unused, 0 - 31 == pointer to interval

			float interval_used[32];
			for (int i = 0; i < num_intervals + 1; i++)
				interval_used[i] = -1.f;  // < -0.5f == unused, 0.f - min(tolerance, step_size) == current error

			// sweep through intervals
			// find closest note and error (guaranteed error < step_size)
			// if also within tolerance,
			//     if note currently unused update note and error
			//     if used, only update if closer, and remove previous interval
			// feed notes into input_scale
			// light up used interval lights, with brightness based on error (greater error == dimmer, over tolerance == off)

			for (int i = 0; i < num_intervals + 1; i++) {
				if (interval_inputs[i] == 1) {
					int closest_note = floorf((interval_lu[i] / step_size) + 0.5f);
					float error = fabsf(interval_lu[i] - closest_note * step_size);
					if (error <= tolerance) {
						int prev_int = note_used[closest_note];
						if (prev_int >= 0 && error < interval_used[prev_int]) {
							interval_used[prev_int] = -1.f;  // zero out previous interval
							note_used[closest_note] = i;
							interval_used[i] = error;
						}
						else if (prev_int < 0) {
							note_used[closest_note] = i;
							interval_used[i] = error;
						}
					}
				}
			}

			// these will take effect next time around
			if (sel_all == 1) {
				for (int i = 0; i < num_intervals; i++)
					params[INTERVAL_PARAMS + i].setValue(1);
			}
			else if (clear_all == 1) {  // except for root value
				params[INTERVAL_PARAMS + 0].setValue(1);
				for (int i = 1; i < num_intervals; i++)
					params[INTERVAL_PARAMS + i].setValue(0);
			}
			else if (sel_enabled == 1) {
				for (int i = 0; i < num_intervals; i++)
					if (interval_used[i] > -0.5f)
						params[INTERVAL_PARAMS + i].setValue(1);
					else
						params[INTERVAL_PARAMS + i].setValue(0);
			}

			// scale is defined by used notes from above
			float input_scale[31];
			for (int i = 0; i < 31; i++)
				input_scale[i] = (note_used[i] >= 0) ? 1 : 0;

			// show all allowed intervals by simulating all intervals selected
			if (show_allowed == 1) {
				int d_note_used[32];  // include octave
				for (int i = 0; i < 32; i++)
					d_note_used[i] = -1;  // -1 == unused, 0 - 31 == pointer to interval

				float d_interval_used[32];
				for (int i = 0; i < num_intervals; i++)
					d_interval_used[i] = -1.f;  // < -0.5f == unused, 0.f - min(tolerance, step_size) == current error

				for (int i = 0; i < num_intervals + 1; i++) {
					int closest_note = floorf((interval_lu[i] / step_size) + 0.5f);
					float error = fabsf(interval_lu[i] - closest_note * step_size);
					if (error <= tolerance) {
						int prev_int = d_note_used[closest_note];
						if (prev_int >= 0 && error < d_interval_used[prev_int]) {
							d_interval_used[prev_int] = -1.f;  // zero out previous interval
							d_note_used[closest_note] = i;
							d_interval_used[i] = error;
						}
						else if (prev_int < 0) {
							d_note_used[closest_note] = i;
							d_interval_used[i] = error;
						}
					}
				}
				// show all allowed lights, with error amount
				for (int i = 0; i < 32; i++) {
					if (d_interval_used[i] > -0.5) {  //interval used
					float error = d_interval_used[i] * 1200.f / 6.f;  // 6¢
						if (error < 1.f)
							lights[INTERVAL_LIGHTS + i].setBrightness(1.f);
						else
							lights[INTERVAL_LIGHTS + i].setBrightness(1.f / error);
					}
					else
						lights[INTERVAL_LIGHTS + i].setBrightness(0.f);
				}
			}
			else {
				// show normal lights, which give error for selected intervals
				for (int i = 0; i < 32; i++) {
					if (interval_used[i] > -0.5) {  //interval used
					float error = interval_used[i] * 1200.f / 6.f;  // 6¢
						if (error < 1.f)
							lights[INTERVAL_LIGHTS + i].setBrightness(1.f);
						else
							lights[INTERVAL_LIGHTS + i].setBrightness(1.f / error);
					}
					else
						lights[INTERVAL_LIGHTS + i].setBrightness(0.f);
				}
			}
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


struct QuantMTIntervalsWidget : ModuleWidget {
	QuantMTIntervalsWidget(QuantMTIntervals* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/QuantMTIntervals.svg")));

		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


		addParam(createParam<TL1105Momentary>(mm2px(Vec(20.93, 19.17)), module, QuantMTIntervals::SEL_ALL_PARAM));
		addParam(createParam<TL1105Momentary>(mm2px(Vec(32.43, 19.17)), module, QuantMTIntervals::CLEAR_ALL_PARAM));

		addParam(createParam<TL1105Momentary>(mm2px(Vec(20.93, 30.67)), module, QuantMTIntervals::SHOW_ALLOWED_PARAM));
		addParam(createParam<TL1105Momentary>(mm2px(Vec(32.43, 30.67)), module, QuantMTIntervals::SEL_ENABLED_PARAM));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(29.39, 47.00)), module, QuantMTIntervals::TOLERANCE_PARAM));
		addParam(createParamCentered<RoundLargeRotarySwitch>(mm2px(Vec(29.39, 65.00)), module, QuantMTIntervals::SIZE_PARAM));

		addParam(createParam<CKSSThree>(mm2px(Vec(21.39, 78.50)), module, QuantMTIntervals::ROUNDING_PARAM));
		addParam(createParam<CKSS>(mm2px(Vec(32.89, 80.00)), module, QuantMTIntervals::EQUI_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.64, 100.0)), module, QuantMTIntervals::CV_IN_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35.14, 100.0)), module, QuantMTIntervals::CV_OUT_OUTPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(23.64, 115.0)), module, QuantMTIntervals::ROOT_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35.14, 115.0)), module, QuantMTIntervals::TRIGGER_OUTPUT));

		//addParam(createParam<RectButton>(mm2px(Vec(4.00, 2.289)), module, QuantMTIntervals::INTERVAL_PARAMS + 31));
		//addParam(createParam<RectButton>(mm2px(Vec(4.00, 6.160)), module, QuantMTIntervals::INTERVAL_PARAMS + 30));
		//addParam(createParam<RectButton>(mm2px(Vec(4.00, 10.031)), module, QuantMTIntervals::INTERVAL_PARAMS + 29));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 13.902)), module, QuantMTIntervals::INTERVAL_PARAMS + 28));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 17.773)), module, QuantMTIntervals::INTERVAL_PARAMS + 27));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 21.644)), module, QuantMTIntervals::INTERVAL_PARAMS + 26));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 25.515)), module, QuantMTIntervals::INTERVAL_PARAMS + 25));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 29.386)), module, QuantMTIntervals::INTERVAL_PARAMS + 24));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 33.257)), module, QuantMTIntervals::INTERVAL_PARAMS + 23));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 37.128)), module, QuantMTIntervals::INTERVAL_PARAMS + 22));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 40.999)), module, QuantMTIntervals::INTERVAL_PARAMS + 21));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 44.870)), module, QuantMTIntervals::INTERVAL_PARAMS + 20));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 48.741)), module, QuantMTIntervals::INTERVAL_PARAMS + 19));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 52.612)), module, QuantMTIntervals::INTERVAL_PARAMS + 18));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 56.483)), module, QuantMTIntervals::INTERVAL_PARAMS + 17));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 60.354)), module, QuantMTIntervals::INTERVAL_PARAMS + 16));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 64.225)), module, QuantMTIntervals::INTERVAL_PARAMS + 15));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 68.096)), module, QuantMTIntervals::INTERVAL_PARAMS + 14));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 71.967)), module, QuantMTIntervals::INTERVAL_PARAMS + 13));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 75.838)), module, QuantMTIntervals::INTERVAL_PARAMS + 12));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 79.709)), module, QuantMTIntervals::INTERVAL_PARAMS + 11));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 83.580)), module, QuantMTIntervals::INTERVAL_PARAMS + 10));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 87.451)), module, QuantMTIntervals::INTERVAL_PARAMS + 9));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 91.322)), module, QuantMTIntervals::INTERVAL_PARAMS + 8));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 95.193)), module, QuantMTIntervals::INTERVAL_PARAMS + 7));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 99.064)), module, QuantMTIntervals::INTERVAL_PARAMS + 6));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 102.935)), module, QuantMTIntervals::INTERVAL_PARAMS + 5));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 106.806)), module, QuantMTIntervals::INTERVAL_PARAMS + 4));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 110.677)), module, QuantMTIntervals::INTERVAL_PARAMS + 3));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 114.548)), module, QuantMTIntervals::INTERVAL_PARAMS + 2));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 118.419)), module, QuantMTIntervals::INTERVAL_PARAMS + 1));
		addParam(createParam<RectButton>(mm2px(Vec(4.00, 122.289)), module, QuantMTIntervals::INTERVAL_PARAMS + 0));

		//addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(3.00, 2.289+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 31));
		//addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(3.00, 6.160+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 30));
		//addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(3.00, 10.031+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 29));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 13.902+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 28));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 17.773+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 27));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 21.644+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 26));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 25.515+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 25));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 29.386+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 24));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 33.257+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 23));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 37.128+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 22));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 40.999+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 21));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 44.870+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 20));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 48.741+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 19));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 52.612+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 18));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 56.483+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 17));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 60.354+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 16));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 64.225+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 15));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 68.096+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 14));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 71.967+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 13));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 75.838+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 12));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 79.709+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 11));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 83.580+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 10));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 87.451+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 9));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 91.322+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 8));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 95.193+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 7));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 99.064+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 6));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 102.935+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 5));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 106.806+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 4));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 110.677+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 3));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 114.548+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 2));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 118.419+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS + 1));
		addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(2.50, 122.289+1.75)), module, QuantMTIntervals::INTERVAL_LIGHTS));
	}
};


Model* modelQuantMTIntervals = createModel<QuantMTIntervals, QuantMTIntervalsWidget>("QuantMTIntervals");

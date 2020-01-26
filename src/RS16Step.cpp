#include "plugin.hpp"

#include "RS.hpp"

struct RS16Step : RSModule {
	static const int patterns = 100;
	static const int steps = 16;
	static const int rows = 4;

	enum ParamIds {
		THEME_BUTTON,

		RATE_DIVIDER,

		PATTERN_KNOB,
		COPY_PATTERN_BUTTON,
		PASTE_PATTERN_BUTTON,

		NEXT_PATTERN_BUTTON,
		PREV_PATTERN_BUTTON,

		ENUMS(NEXT_STEP_BUTTONS, rows),
		ENUMS(PREV_STEP_BUTTONS, rows),
		ENUMS(RAND_STEP_BUTTONS, rows),

		ENUMS(WRITE_BUTTONS, rows),

		ENUMS(RAND_ALL_BUTTONS, rows),
		ENUMS(RAND_STEPS_BUTTONS, rows),
		ENUMS(RAND_DOORS_BUTTONS, rows),
		ENUMS(RAND_PULSES_BUTTONS, rows),

		ENUMS(RESET_STEP_BUTTONS, rows),
		ENUMS(RESET_STEPS_BUTTONS, rows),
		ENUMS(RESET_DOORS_BUTTONS, rows),
		ENUMS(RESET_PULSES_BUTTONS, rows),

		ENUMS(COPY_ROW_BUTTONS, rows),
		ENUMS(PASTE_ROW_BUTTONS, rows),

		ENUMS(STEP_KNOBS, rows * steps),
		ENUMS(DOOR_BUTTONS, rows * steps),
		ENUMS(PULSE_BUTTONS, rows * steps),

		ENUMS(SCALE_KNOBS, rows),
		ENUMS(OFFSET_KNOBS, rows),
		NUM_PARAMS
	};
	enum InputIds {
		NEXT_PATTERN_IN,
		PREV_PATTERN_IN,

		ENUMS(NEXT_STEP_INS, rows),
		ENUMS(PREV_STEP_INS, rows),
		ENUMS(PHASE_STEP_INS, rows),
		ENUMS(RAND_STEP_INS, rows),

		ENUMS(CV_INS, rows),
		ENUMS(WRITE_INS, rows),

		ENUMS(RAND_ALL_INS, rows),
		ENUMS(RAND_STEPS_INS, rows),
		ENUMS(RAND_DOORS_INS, rows),
		ENUMS(RAND_PULSES_INS, rows),

		ENUMS(RESET_STEP_INS, rows),
		ENUMS(RESET_STEPS_INS, rows),
		ENUMS(RESET_DOORS_INS, rows),
		ENUMS(RESET_PULSES_INS, rows),

		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(DOORS_OUTS, rows * steps),
		ENUMS(PULSES_OUTS, rows * steps),

		ENUMS(CV_OUTS, rows),
		ENUMS(EOC_OUTS, rows),
		ENUMS(DOOR_OUTS, rows),
		ENUMS(PULSE_OUTS, rows),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(STEP_LIGHTS, rows * steps),
		NUM_LIGHTS
	};

	dsp::SchmittTrigger themeTrigger;

	dsp::ClockDivider rateDivider;


	int stepIdx[rows];
	int priorStepIdx[rows];
	float phaseIn, priorPhaseIn;
	int phaseRow, priorPhaseRow;

	struct StepBuffer {
		float step;
		bool door;
		bool pulse;
	};

	struct RowBuffer {
		StepBuffer step[steps];
		float scale;
		float offset;
	};

	struct PatternBuffer {
		char description[30];
		RowBuffer rowBuffer[rows];
	};

	// Copy & paste buffers
	RowBuffer rowBuffer;
	PatternBuffer patternBuffer;

	//Pattern storage
	PatternBuffer patternStore[patterns];
	RSScribbleStrip *ssPatternDescription;
	int pattern, priorPattern;
	

	dsp::BooleanTrigger copyPatternTrigger;
	dsp::BooleanTrigger pastePatternTrigger;

	dsp::BooleanTrigger prevPatternTrigger;
	dsp::BooleanTrigger nextPatternTrigger;

	dsp::BooleanTrigger prevStepTrigger[rows];
	dsp::BooleanTrigger nextStepTrigger[rows];
	dsp::BooleanTrigger randStepTrigger[rows];
	
	dsp::SchmittTrigger randAllTrigger[rows];
	dsp::SchmittTrigger randStepsTrigger[rows];
	dsp::SchmittTrigger randGatesTrigger[rows];
	dsp::SchmittTrigger randPulsesTrigger[rows];

	dsp::SchmittTrigger resetAllTrigger[rows];
	dsp::SchmittTrigger resetStepsTrigger[rows];
	dsp::SchmittTrigger resetGatesTrigger[rows];
	dsp::SchmittTrigger resetPulsesTrigger[rows];

	dsp::BooleanTrigger copyRowTrigger[rows];
	dsp::BooleanTrigger pasteRowTrigger[rows];

	dsp::PulseGenerator rowPulse[rows][steps];
	bool pulse[rows][steps];

	dsp::PulseGenerator stepPulse[rows][steps];

	dsp::PulseGenerator eocPulse[rows];
	bool eoc[rows];

	

	RS16Step() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(THEME_BUTTON, 0.f, 1.f, 0.f, "THEME");

		configParam(PATTERN_KNOB, 0.f, (float)patterns - 1, 0.f, "PATTERN SELECT");
		configParam(COPY_PATTERN_BUTTON, 0.f, 1.f, 0.f, "COPY PATTERN");
		configParam(PASTE_PATTERN_BUTTON, 0.f, 1.f, 0.f, "PASTE PATTERN");

		configParam(PREV_PATTERN_BUTTON, 0.f, 1.f, 0.f, "PREV PATTERN");
		configParam(NEXT_PATTERN_BUTTON, 0.f, 1.f, 0.f, "NEXT PATTERN");

		for(int row = 0; row < rows; row++) {
			stepIdx[row] = 0; priorStepIdx[row] = 0;
			eoc[row] = false;

			configParam(NEXT_STEP_BUTTONS + row, 0.f, 1.f, 0.f, "NEXT STEP");
			configParam(PREV_STEP_BUTTONS + row, 0.f, 1.f, 0.f, "PREV STEP");
			configParam(RAND_STEP_BUTTONS + row, 0.f, 1.f, 0.f, "RAND STEP");

			configParam(WRITE_BUTTONS + row, 0.f, 1.f, 0.f, "WRITE CV");

			configParam(RAND_ALL_BUTTONS + row, 0.f, 1.f, 0.f, "RANDOMIZE ALL");
			configParam(RAND_STEPS_BUTTONS + row, 0.f, 1.f, 0.f, "RANDOMIZE STEPS");
			configParam(RAND_DOORS_BUTTONS + row, 0.f, 1.f, 0.f, "RANDOMIZE DOORS");
			configParam(RAND_PULSES_BUTTONS + row, 0.f, 1.f, 0.f, "RANDOMIZE PULSES");

			configParam(RESET_STEP_BUTTONS + row, 0.f, 1.f, 0.f, "RESET STEP");
			configParam(RESET_STEPS_BUTTONS + row, 0.f, 1.f, 0.f, "RESET STEPS");
			configParam(RESET_DOORS_BUTTONS + row, 0.f, 1.f, 0.f, "RESET DOORS");
			configParam(RESET_PULSES_BUTTONS + row, 0.f, 1.f, 0.f, "RESET PULSES");

			configParam(COPY_ROW_BUTTONS + row, 0.f, 1.f, 0.f, "COPY ROW");
			configParam(PASTE_ROW_BUTTONS + row, 0.f, 1.f, 0.f, "PASTE ROW");

			for(int step = 0; step < steps; step++) {
				configParam(STEP_KNOBS + (row * steps) + step, -10.f, 10.f, 0.f, "STEP");
				configParam(DOOR_BUTTONS + (row * steps) + step, 0.f, 1.f, 0.f, "DOOR");
				configParam(PULSE_BUTTONS + (row * steps) + step, 0.f, 1.f, 0.f, "PULSE");
				pulse[row][step] = false;
			}

			configParam(SCALE_KNOBS + row, -.5f, .5f, 0.25f, "SCALE");
			configParam(OFFSET_KNOBS + row, -2.5f, 2.5f, 0.f, "OFFSET");
		}

		for(int step = 0; step < steps; step++) {
			rowBuffer.step[step].step = 0.f;
			rowBuffer.step[step].door = false;
			rowBuffer.step[step].pulse = false;
		}
		rowBuffer.scale = .25f;
		rowBuffer.offset = 0.f;

		for(int row = 0; row < rows; row++) {
			for(int step = 0; step < steps; step++) {
				patternBuffer.rowBuffer[row].step[step].step = 0.f;
				patternBuffer.rowBuffer[row].step[step].door = false;
				patternBuffer.rowBuffer[row].step[step].pulse = false;
			}
			patternBuffer.rowBuffer[row].scale = .25f;
			patternBuffer.rowBuffer[row].offset = 0.f;
		}

		for(int pattern = 0; pattern < patterns; pattern++) {
			sprintf(patternStore[pattern].description, "Pattern %i", pattern);
			for(int row = 0; row < rows; row++) {
				for(int step = 0; step < steps; step++) {
					patternStore[pattern].rowBuffer[row].step[step].step = 0.f;
					patternStore[pattern].rowBuffer[row].step[step].door = false;
					patternStore[pattern].rowBuffer[row].step[step].pulse = false;
				}
				patternStore[pattern].rowBuffer[row].scale = 0.25f;
				patternStore[pattern].rowBuffer[row].offset = 0.f;
			}
		}

		pattern = 0;
		priorPattern = 0;

		rateDivider.setDivision(4);
	}

	void process(const ProcessArgs &args) override {
		if(themeTrigger.process(params[THEME_BUTTON].getValue())) {
			RSTheme++;
			if(RSTheme > RSGlobal.themeCount) RSTheme = 1;
		}

		if(rateDivider.process()) {
			// Process prev / next pattern
			if(inputs[PREV_PATTERN_IN].isConnected()) {
				if(prevPatternTrigger.process(inputs[PREV_PATTERN_IN].getVoltage())) prevPattern();
			}
			else if(prevPatternTrigger.process(params[PREV_PATTERN_BUTTON].getValue())) prevPattern();

			if(inputs[NEXT_PATTERN_IN].isConnected()) {
				if(nextPatternTrigger.process(inputs[NEXT_PATTERN_IN].getVoltage())) nextPattern();
			}
			else if(nextPatternTrigger.process(params[NEXT_PATTERN_BUTTON].getValue())) nextPattern();

			for(int row = 0; row < rows; row++) {
				// Process phase step
				if(inputs[PHASE_STEP_INS + row].isConnected()) {
					phaseIn = RSclamp(inputs[PHASE_STEP_INS + row].getVoltage(), 0.f, 10.f);
					phaseRow = (int)RSscale(phaseIn, 0.f, 10.f, 0.f, 16.f);
					if(phaseRow < 16) stepIdx[row] = phaseRow; // Defensive, square waves overrun

					if(phaseIn < priorPhaseIn) eocPulse[row].trigger(); // Fucks up when more than one row phase driven?
					priorPhaseIn = phaseIn;

					if(phaseRow != priorPhaseRow) {
						if(params[PULSE_BUTTONS + (row * steps) + stepIdx[row]].getValue()) rowPulse[row][stepIdx[row]].trigger();
					}
					else rowPulse[row][stepIdx[row]].reset();
					priorPhaseRow = phaseRow;
				}
				else {
					// Process prev step
					if(inputs[PREV_STEP_INS + row].isConnected()) {
						if(prevStepTrigger[row].process(inputs[PREV_STEP_INS + row].getVoltage())) prevStep(row);
					}
					else if(prevStepTrigger[row].process(params[PREV_STEP_BUTTONS + row].getValue())) prevStep(row);

					// Process next step
					if(inputs[NEXT_STEP_INS + row].isConnected()) {
						if(nextStepTrigger[row].process(inputs[NEXT_STEP_INS + row].getVoltage())) nextStep(row);
					}
					else if(nextStepTrigger[row].process(params[NEXT_STEP_BUTTONS + row].getValue())) nextStep(row);

					// Process rand step
					if(inputs[RAND_STEP_INS + row].isConnected()) {
						if(randStepTrigger[row].process(inputs[RAND_STEP_INS + row].getVoltage())) randStep(row);
					}
					else if(randStepTrigger[row].process(params[RAND_STEP_BUTTONS + row].getValue())) randStep(row);

					// Process pulses
					if(stepIdx[row] != priorStepIdx[row]) {
						if(params[PULSE_BUTTONS + (row * steps) + stepIdx[row]].getValue()) rowPulse[row][stepIdx[row]].trigger();
					}
					else rowPulse[row][stepIdx[row]].reset();
					priorStepIdx[row] = stepIdx[row];
				}

				// Process write CV in
				if(inputs[WRITE_INS + row].getVoltage()) {  // or WRITE_BUTTONS
					params[STEP_KNOBS + (row * steps) + stepIdx[row]].setValue(RSclamp(inputs[CV_INS + row].getVoltage(), -10.f, 10.f));
				}

				// Process randomize all
				if(inputs[RAND_ALL_INS + row].isConnected()) {
					if(randAllTrigger[row].process(inputs[RAND_ALL_INS + row].getVoltage())) randomizeAll(row);
				}
				else if(randAllTrigger[row].process(params[RAND_ALL_BUTTONS + row].getValue())) randomizeAll(row);

				// Process randomize steps
				if(inputs[RAND_STEPS_INS + row].isConnected()) {
					if(randStepsTrigger[row].process(inputs[RAND_STEPS_INS + row].getVoltage())) randomizeSteps(row);
				}
				else if(randStepsTrigger[row].process(params[RAND_STEPS_BUTTONS + row].getValue())) randomizeSteps(row);

				// Process randomize doors
				if(inputs[RAND_DOORS_INS + row].isConnected()) {
					if(randGatesTrigger[row].process(inputs[RAND_DOORS_INS + row].getVoltage())) randomizeDoors(row);
				}
				else if(randGatesTrigger[row].process(params[RAND_DOORS_BUTTONS + row].getValue())) randomizeDoors(row);

				// Process randomize pulses
				if(inputs[RAND_PULSES_INS + row].isConnected()) {
					if(randPulsesTrigger[row].process(inputs[RAND_PULSES_INS + row].getVoltage())) randomizePulses(row);
				}
				else if(randPulsesTrigger[row].process(params[RAND_PULSES_BUTTONS + row].getValue())) randomizePulses(row);

				// Process reset step
				if(inputs[RESET_STEP_INS + row].isConnected()) {
					if(resetAllTrigger[row].process(inputs[RESET_STEP_INS + row].getVoltage())) resetStep(row);
				}
				else if(resetAllTrigger[row].process(params[RESET_STEP_BUTTONS + row].getValue())) resetStep(row);

				// Process reset steps
				if(inputs[RESET_STEPS_INS + row].isConnected()) {
					if(resetStepsTrigger[row].process(inputs[RESET_STEPS_INS + row].getVoltage())) resetSteps(row);
				}
				else if(resetStepsTrigger[row].process(params[RESET_STEPS_BUTTONS + row].getValue())) resetSteps(row);

				// Process reset doors
				if(inputs[RESET_DOORS_INS + row].isConnected()) {
					if(resetGatesTrigger[row].process(inputs[RESET_DOORS_INS + row].getVoltage())) resetDoors(row);
				}
				else if(resetGatesTrigger[row].process(params[RESET_DOORS_BUTTONS + row].getValue())) resetDoors(row);

				// Process reset pulses
				if(inputs[RESET_PULSES_INS + row].isConnected()) {
					if(resetPulsesTrigger[row].process(inputs[RESET_PULSES_INS + row].getVoltage())) resetPulses(row);
				}
				else if(resetPulsesTrigger[row].process(params[RESET_PULSES_BUTTONS + row].getValue())) resetPulses(row);
				
				// Set lights
				for(int step = 0; step < steps; step++) {
					lights[STEP_LIGHTS + (row * steps) + step].setBrightness(step == stepIdx[row] ? 1.f : 0.f);
				}

				float cvOut = params[STEP_KNOBS + (row * steps) + stepIdx[row]].getValue();
				// Ouput raw CV


				// Output scaled CV
				cvOut = cvOut * params[SCALE_KNOBS + row].getValue() + params[OFFSET_KNOBS + row].getValue();
				cvOut = RSclamp(cvOut, -10.f, 10.f);
				outputs[CV_OUTS + row].setVoltage(cvOut);

				// Output EOC
				eoc[row] = eocPulse[row].process(1.f / args.sampleRate);
				outputs[EOC_OUTS + row].setVoltage(eoc[row] ? 10.f : 0.f);

				// Output row doors
				outputs[DOOR_OUTS + row].setVoltage(params[DOOR_BUTTONS + (row * steps) + stepIdx[row]].getValue() ? 10.f : 0.f);

				// Output row pulses
				pulse[row][stepIdx[row]] = rowPulse[row][stepIdx[row]].process(1.f / args.sampleRate);
				outputs[PULSE_OUTS + row].setVoltage(pulse[row][stepIdx[row]] ? 10.f : 0.f);

				// Output step doors & pulses
				for(int step = 0; step < steps; step++) {
					if(step == stepIdx[row]) {
						outputs[DOORS_OUTS + (row * steps) + step].setVoltage(params[DOOR_BUTTONS + (row * steps) + step].getValue() ? 10.f : 0.f);
						if(params[PULSE_BUTTONS + (row * steps) + step].getValue()) stepPulse[row][step].trigger();
					}
					else {
						outputs[DOORS_OUTS + (row * steps) + step].setVoltage(0.f);
					}

					outputs[PULSES_OUTS + (row * steps) + step].setVoltage(stepPulse[row][step].process(1.f /  args.sampleRate));
				}
			}
		}
	}

	void nextPattern() {
		if(pattern == patterns - 1) pattern = 0;
		else pattern++;
		params[PATTERN_KNOB].setValue(pattern);
	}

	void prevPattern() {
		if(pattern == 0) pattern = patterns - 1;
		else pattern--;
		params[PATTERN_KNOB].setValue(pattern);
	}

	void nextStep(int row) {
		stepIdx[row]++;
		if(stepIdx[row] == steps) {
			stepIdx[row] = 0;
			eocPulse[row].trigger();
		}
	}

	void prevStep(int row) {
		stepIdx[row]--;
		if(stepIdx[row] < 0) {
			stepIdx[row] = steps - 1;
			eocPulse[row].trigger();
		}
	}

	void randStep(int row) {
		stepIdx[row] = rand() % steps;
	}

	void randomizeAll(int row) {
		randomizeSteps(row);
		randomizeDoors(row);
		randomizePulses(row);
	}

	void randomizeSteps(int row) {
		for(int step = 0; step < steps; step++) params[STEP_KNOBS + (row * steps) + step].setValue((((float)rand() / (float)RAND_MAX) * 20.f) - 10.f);
	}

	void randomizeDoors(int row) {
		for(int step = 0; step < steps; step++) params[DOOR_BUTTONS + (row * steps) + step].setValue(rand() %2);
	}

	void randomizePulses(int row) {
		for(int step = 0; step < steps; step++) params[PULSE_BUTTONS + (row * steps) + step].setValue(rand() %2);
	}

	void onReset() override {
		for(int row = 0; row < rows; row++) {
			resetStep(row);
			resetSteps(row);
			resetDoors(row);
			resetPulses(row);
		}
	}

	void resetStep(int row) {
		stepIdx[row] = 0;
		eocPulse[row].trigger();
	}

	void resetSteps(int row) {
		for(int step = 0; step < steps; step++) params[STEP_KNOBS + (row * steps) + step].setValue(0.f);
	}

	void resetDoors(int row) {
		for(int step = 0; step < steps; step++) params[DOOR_BUTTONS + (row * steps) + step].setValue(0.f);
	}

	void resetPulses(int row) {
		for(int step = 0; step < steps; step++) params[PULSE_BUTTONS + (row * steps) + step].setValue(0.f);
	}

	void copyRow(int row) {
		for(int step = 0; step < steps; step++) {
			rowBuffer.step[step].step = params[STEP_KNOBS + (row * steps) + step].getValue();
			rowBuffer.step[step].door = params[DOOR_BUTTONS + (row * steps) + step].getValue();
			rowBuffer.step[step].pulse = params[PULSE_BUTTONS + (row * steps) + step].getValue();
		}
		rowBuffer.scale = params[SCALE_KNOBS + row].getValue();
		rowBuffer.offset = params[OFFSET_KNOBS + row].getValue();		
	}

	void pasteRow(int row) {
		for(int step = 0; step < steps; step++) {
			params[STEP_KNOBS + (row * steps) + step].setValue(rowBuffer.step[step].step);
			params[DOOR_BUTTONS + (row * steps) + step].setValue(rowBuffer.step[step].door);
			params[PULSE_BUTTONS + (row * steps) + step].setValue(rowBuffer.step[step].pulse);
		}
		params[SCALE_KNOBS + row].setValue(rowBuffer.scale);
		params[OFFSET_KNOBS + row].setValue(rowBuffer.offset);
	}
	
	void savePattern(int pattern) {
		strcpy(patternStore[pattern].description, ssPatternDescription->text.c_str());
		for(int row = 0; row < rows; row++) {
			for(int step = 0; step < steps; step++) {
				patternStore[pattern].rowBuffer[row].step[step].step  = params[STEP_KNOBS + (row * steps) + step].getValue();
				patternStore[pattern].rowBuffer[row].step[step].door  = params[DOOR_BUTTONS + (row * steps) + step].getValue();
				patternStore[pattern].rowBuffer[row].step[step].pulse  = params[PULSE_BUTTONS + (row * steps) + step].getValue();
			}
			patternStore[pattern].rowBuffer[row].scale = params[SCALE_KNOBS + row].getValue();
			patternStore[pattern].rowBuffer[row].offset = params[OFFSET_KNOBS + row].getValue();
		}
	}

	void loadPattern(int pattern) {
		ssPatternDescription->setText(patternStore[pattern].description);
		for(int row = 0; row < rows; row++) {
			for(int step = 0; step < steps; step++) {
				params[STEP_KNOBS + (row * steps) + step].setValue(patternStore[pattern].rowBuffer[row].step[step].step);
				params[DOOR_BUTTONS + (row * steps) + step].setValue(patternStore[pattern].rowBuffer[row].step[step].door);
				params[PULSE_BUTTONS + (row * steps) + step].setValue(patternStore[pattern].rowBuffer[row].step[step].pulse);
			}
			params[SCALE_KNOBS + row].setValue(patternStore[pattern].rowBuffer[row].scale);
			params[OFFSET_KNOBS + row].setValue(patternStore[pattern].rowBuffer[row].offset);
		}
	}

	void copyPattern() {
		strcpy(patternBuffer.description, ssPatternDescription->text.c_str());
		for(int row = 0; row < rows; row++) {
			for(int step = 0; step < steps; step++) {
				patternBuffer.rowBuffer[row].step[step].step = params[STEP_KNOBS + (row * steps) + step].getValue();
				patternBuffer.rowBuffer[row].step[step].door = params[DOOR_BUTTONS + (row * steps) + step].getValue();
				patternBuffer.rowBuffer[row].step[step].pulse = params[PULSE_BUTTONS + (row * steps) + step].getValue();
			}
			patternBuffer.rowBuffer[row].scale = params[SCALE_KNOBS + row].getValue();
			patternBuffer.rowBuffer[row].offset = params[OFFSET_KNOBS + row].getValue();
		}
	}

	void pastePattern() {
		ssPatternDescription->setText(patternBuffer.description);
		for(int row = 0; row < rows; row++) {
			for(int step = 0; step < steps; step++) {
				params[STEP_KNOBS + (row * steps) + step].setValue(patternBuffer.rowBuffer[row].step[step].step);
				params[DOOR_BUTTONS + (row * steps) + step].setValue(patternBuffer.rowBuffer[row].step[step].door);
				params[PULSE_BUTTONS + (row * steps) + step].setValue(patternBuffer.rowBuffer[row].step[step].pulse);
			}
			params[SCALE_KNOBS + row].setValue(patternBuffer.rowBuffer[row].scale);
			params[OFFSET_KNOBS + row].setValue(patternBuffer.rowBuffer[row].offset);
		}
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
        json_object_set_new(rootJ, "theme", json_integer(RSTheme));

		// If the pattern number hasn't changed since the pattern was modified, so widget step hasn't had a chance to save params to pattern array,
		//   we need to ensure that the current pattern params are stored in the pattern array
		//savePattern(pattern);

		// json_t* descriptionArray = json_array();
		// json_t* stepArray = json_array();
		// json_t* doorArray = json_array();
		// json_t* pulseArray = json_array();
		// json_t* scaleArray = json_array();
		// json_t* offsetArray = json_array();

		// for(int pattern = 0; pattern < patterns; pattern++)
		// 	json_array_append_new(descriptionArray, json_string(patternStore[pattern].description));

		// for(int pattern = 0; pattern < patterns; pattern++)
		// 	for(int row = 0; row < rows; row++)
		// 		for(int step = 0; step < steps; step++)
		// 			json_array_append_new(stepArray, json_real(patternStore[pattern].rowBuffer[row].step[step].step));

		// for(int pattern = 0; pattern < patterns; pattern++)
		// 	for(int row = 0; row < rows; row++)
		// 		for(int step = 0; step < steps; step++)
		// 			json_array_append_new(doorArray, json_boolean(patternStore[pattern].rowBuffer[row].step[step].door));

		// for(int pattern = 0; pattern < patterns; pattern++)
		// 	for(int row = 0; row < rows; row++)
		// 		for(int step = 0; step < steps; step++)
		// 			json_array_append_new(pulseArray, json_boolean(patternStore[pattern].rowBuffer[row].step[step].pulse));

		// for(int pattern = 0; pattern < patterns; pattern++)
		// 	for(int row = 0; row < rows; row++)
		// 		json_array_append(scaleArray, json_real(patternStore[pattern].rowBuffer[row].scale));

		// for(int pattern = 0; pattern < patterns; pattern++)
		// 	for(int row = 0; row < rows; row++)
		// 		json_array_append(offsetArray, json_real(patternStore[pattern].rowBuffer[row].offset));

		// json_object_set_new(rootJ, "descriptions", descriptionArray);
		// json_object_set_new(rootJ, "steps", stepArray);
		// json_object_set_new(rootJ, "doors", doorArray);
		// json_object_set_new(rootJ, "pulses", pulseArray);
		// json_object_set_new(rootJ, "scales", scaleArray);
		// json_object_set_new(rootJ, "offsets", offsetArray);

		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
        json_t* themeJ = json_object_get(rootJ, "theme");
        if(themeJ) RSTheme = json_integer_value(themeJ);
		
		// json_t* descriptionArray = json_object_get(rootJ, "descriptions");
		// json_t* stepArray = json_object_get(rootJ, "steps");
		// json_t* doorArray = json_object_get(rootJ, "doors");
		// json_t* pulseArray = json_object_get(rootJ, "pulses");
		// json_t* scaleArray = json_object_get(rootJ, "scales");
		// json_t* offsetArray = json_object_get(rootJ, "offsets");
		
		// if(descriptionArray)
		// 	for(int pattern = 0, i = 0; pattern < patterns; pattern++, i++)
		// 		strcpy(patternStore[pattern].description, json_string_value(json_array_get(descriptionArray, i)));

		// if(stepArray)
		// 	for(int pattern = 0, i = 0; pattern < patterns; pattern++, i++)
		// 		for(int row = 0; row < rows; row++, i++)
		// 			for(int step = 0; step < steps; step++, i++)
		// 				patternStore[pattern].rowBuffer[row].step[step].step = json_real_value(json_array_get(stepArray, i));
		
		// if(doorArray)
		// 	for(int pattern = 0, i = 0; pattern < patterns; pattern++, i++)
		// 		for(int row = 0; row < rows; row++, i++)
		// 			for(int step = 0; step < steps; step++, i++)
		// 				patternStore[pattern].rowBuffer[row].step[step].door = json_boolean_value(json_array_get(doorArray, i));

		// if(pulseArray)
		// 	for(int pattern = 0, i = 0; pattern < patterns; pattern++, i++)
		// 		for(int row = 0; row < rows; row++, i++)
		// 			for(int step = 0; step < steps; step++, i++)
		// 				patternStore[pattern].rowBuffer[row].step[step].pulse = json_boolean_value(json_array_get(pulseArray, i));

		// if(scaleArray)
		// 	for(int pattern = 0, i = 0; pattern < patterns; pattern++, i++)
		// 		for(int row = 0; row < rows; row++, i++)
		// 			patternStore[pattern].rowBuffer[row].scale = json_real_value(json_array_get(scaleArray, i));
		
		// if(offsetArray)
		// 	for(int pattern = 0, i = 0; pattern < patterns; pattern++, i++)
		// 		for(int row = 0; row < rows; row++, i++)
		// 			patternStore[pattern].rowBuffer[row].offset = json_real_value(json_array_get(offsetArray, i));
		
		// pattern = 0; priorPattern = patterns - 1; // So widget step() updates
	}
};


struct RS16StepWidget : ModuleWidget {
	RS16Step* module;

	int x, y, smlGap, lrgGap, labOfs;

	RSLabelCentered *patternLabel;

	RS16StepWidget(RS16Step *module) {
		setModule(module);
		this->module = module;

		box.size = Vec(RACK_GRID_WIDTH * 110, RACK_GRID_HEIGHT); // Amend so number of steps scales accordingly
		int middle = box.size.x / 2 + 1;

		addParam(createParamCentered<RSButtonMomentaryInvisible>(Vec(box.pos.x + 5, box.pos.y + 5), module, RS16Step::THEME_BUTTON));

		addChild(new RSLabelCentered(middle, box.pos.y + 13, "Phydeaux III", 14, module));
		addChild(new RSLabelCentered(middle, box.size.y - 4, "Racket Science", 12, module));

		x = 60; y = 50;
		smlGap = 35; lrgGap = 65;
		labOfs = 20;

		// Pattern section
		addChild(new RSLabelCentered(x, y - (labOfs * 1.5), "PATTERN", 10, module));
		y += 20;

		// Pattern knob
		addParam(createParamCentered<RSKnobDetentLrg>(Vec(x, y), module, RS16Step::PATTERN_KNOB));
		patternLabel = new RSLabelCentered(x, y, "0", 22, module);
		addChild(patternLabel);
		y += smlGap + 10;

		// Pattern name scribble strip
		if(module) {
			addChild(module->ssPatternDescription = new RSScribbleStrip(x - 50, y, 100));
			module->ssPatternDescription->setText(module->patternStore[(int)module->params[RS16Step::PATTERN_KNOB].getValue()].description);
		}
		y += smlGap;

		// Pattern copy & paste
		addParam(createParamCentered<RSButtonMomentary>(Vec(x - 20, y), module, RS16Step::COPY_PATTERN_BUTTON));
		addChild(new RSLabelCentered(x - 20, y + 3, "COPY", 10, module));

		addParam(createParamCentered<RSButtonMomentary>(Vec(x + 20, y), module, RS16Step::PASTE_PATTERN_BUTTON));
		addChild(new RSLabelCentered(x + 20, y + 3, "PASTE", 10, module));

		y += smlGap;

		// Pattern prev / next ins
		addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x - 20, y), module, RS16Step::PREV_PATTERN_BUTTON));
		addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x - 20, y), module, RS16Step::PREV_PATTERN_IN));
		addChild(new RSLabelCentered(x - 20, y + 25, "PREV", 10, module));

		addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x + 20, y), module, RS16Step::NEXT_PATTERN_BUTTON));
		addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x + 20, y), module, RS16Step::NEXT_PATTERN_IN));
		addChild(new RSLabelCentered(x + 20, y + 25, "NEXT", 10, module));

		// Want a reset all steps on all rows option



		x = 130; y = 50;

		// Left side labels
		addChild(new RSLabelCentered(x + (smlGap / 2), y - (labOfs * 1.5), "STEP", 10, module));
		addChild(new RSLabelCentered(x, y - labOfs, "PREV", 10, module));
		x += smlGap;
		addChild(new RSLabelCentered(x, y - labOfs, "NEXT", 10, module));
		x -= smlGap; y += 55;
		addChild(new RSLabelCentered(x, y, "PHASE", 10, module));
		x += smlGap;
		addChild(new RSLabelCentered(x, y, "RAND", 10, module));
		x += smlGap; y -= 55;

		addChild(new RSLabelCentered(x, y - labOfs, "CV IN", 10, module));
		y += 55;
		addChild(new RSLabelCentered(x, y, "WRITE", 10, module));
		x += smlGap; y -= 55;

		addChild(new RSLabelCentered(x + (smlGap / 2), y - (labOfs * 1.5), "RANDOMIZE", 10, module));
		addChild(new RSLabelCentered(x, y - labOfs, "ALL", 10, module));
		x += smlGap;
		addChild(new RSLabelCentered(x, y - labOfs, "STEPS", 10, module));
		x -= smlGap; y += 55;
		addChild(new RSLabelCentered(x, y, "DOORS", 10, module));
		x += smlGap;
		addChild(new RSLabelCentered(x, y, "PULSES", 10, module));
		x += smlGap; y -= 55;

		addChild(new RSLabelCentered(x + (smlGap / 2), y - (labOfs * 1.5), "RESET", 10, module));
		addChild(new RSLabelCentered(x, y - labOfs, "STEP", 10, module));
		x += smlGap;
		addChild(new RSLabelCentered(x, y - labOfs, "STEPS", 10, module));
		x -= smlGap; y += 55;
		addChild(new RSLabelCentered(x, y, "DOORS", 10, module));
		x += smlGap;
		addChild(new RSLabelCentered(x, y, "PULSES", 10, module));
		x += smlGap; y -= 55;

		// Skip over steps
		x += smlGap * 2;
		x += (lrgGap * module->steps);

		// Right side labels

		// scale & offset here
		addChild(new RSLabelCentered(x, y - labOfs, "SCALE", 10, module));
		y += 55;
		addChild(new RSLabelCentered(x, y, "OFFSET", 10, module));
		x += smlGap; y -= 55;

		addChild(new RSLabelCentered(x, y - labOfs, "CV OUT", 10, module));
		x += smlGap;
		addChild(new RSLabelCentered(x, y - labOfs, "EOC", 10, module));
		x -= smlGap; y += 55;
		addChild(new RSLabelCentered(x, y, "DOOR", 10, module));
		x += smlGap;
		addChild(new RSLabelCentered(x, y, "PULSE", 10, module));
		x += smlGap; y -= 55;

		x = 130; y = 50;
		for(int row = 0, rowGap = 85; row < module->rows; row++, y += rowGap) addStepSeqRow(row, x, y);
	}

	void addStepSeqRow(int row, int x, int y) {
		// Add prev step in
		addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RS16Step::PREV_STEP_BUTTONS + row));
		addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RS16Step::PREV_STEP_INS + row));
		x += smlGap;

		// Add next step in
		addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RS16Step::NEXT_STEP_BUTTONS + row));
		addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RS16Step::NEXT_STEP_INS + row));
		x -= smlGap; y += 30;

		// Add phase step in
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y), module, RS16Step::PHASE_STEP_INS + row));
		x += smlGap;

		// Add rand step in
		addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RS16Step::RAND_STEP_BUTTONS + row));
		addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RS16Step::RAND_STEP_INS + row));
		x += smlGap; y -= 30;

		// Add CV in
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y), module, RS16Step::CV_INS + row));
		y += 30;

		// Add write in
		addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RS16Step::WRITE_BUTTONS + row));
		addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RS16Step::WRITE_INS + row));
		x += smlGap; y -= 30;
		
		// Add randomize all in
		addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RS16Step::RAND_ALL_BUTTONS + row));
		addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RS16Step::RAND_ALL_INS + row));
		x += smlGap;
		
		// Add randomize steps in
		addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RS16Step::RAND_STEPS_BUTTONS + row));
		addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RS16Step::RAND_STEPS_INS  + row ));
		x -= smlGap; y += 30;

		// Add randomize doors in
		addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RS16Step::RAND_DOORS_BUTTONS + row));
		addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RS16Step::RAND_DOORS_INS  + row ));
		x += smlGap;

		// Add randomize pulses in
		addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RS16Step::RAND_PULSES_BUTTONS + row));
		addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RS16Step::RAND_PULSES_INS  + row ));
		x += smlGap; y -= 30;

		// Add reset step in
		addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RS16Step::RESET_STEP_BUTTONS + row));
		addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RS16Step::RESET_STEP_INS + row));
		x += smlGap;

		// Add reset steps in
		addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RS16Step::RESET_STEPS_BUTTONS + row));
		addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RS16Step::RESET_STEPS_INS + row));
		x -= smlGap; y += 30;

		// Add reset doors in
		addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RS16Step::RESET_DOORS_BUTTONS + row));
		addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RS16Step::RESET_DOORS_INS + row));
		x += smlGap;

		// Add reset pulses in
		addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RS16Step::RESET_PULSES_BUTTONS + row));
		addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RS16Step::RESET_PULSES_INS + row));
		x += smlGap; y -= 30;

		// Add copy & paste
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RS16Step::COPY_ROW_BUTTONS + row));
		addChild(new RSLabelCentered(x, y + 3, "COPY", 10, module));
		y += 30;
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RS16Step::PASTE_ROW_BUTTONS + row));
		addChild(new RSLabelCentered(x, y + 3, "PASTE", 10, module));
		x += smlGap; y -= 30;
		
		x += smlGap;

		// Add step knobs & lights, door & pulse buttons, door & pulse outputs
		for(int step = 0; step < module->steps; step++, x += lrgGap) {
			addParam(createParamCentered<RSKnobMed>(Vec(x, y), module, RS16Step::STEP_KNOBS + (row * module->steps) + step));
			addChild(createLightCentered<LargeLight<GreenLight>>(Vec(x, y), module, RS16Step::STEP_LIGHTS + (row * module->steps) + step));

			addParam(createParamCentered<RSButtonToggle>(Vec(x - 15, y + 42), module, RS16Step::DOOR_BUTTONS + (row * module->steps) + step));
			// Add door outs
			addOutput(createOutputCentered<RSStealthJackSmallMonoOut>(Vec(x - 15, y + 42), module, RS16Step::DOORS_OUTS + (row * module->steps) + step));

			addParam(createParamCentered<RSButtonToggle>(Vec(x + 15, y + 42), module, RS16Step::PULSE_BUTTONS + (row * module->steps) + step));
			// Add pulse outs
			addOutput(createOutputCentered<RSStealthJackSmallMonoOut>(Vec(x + 15, y + 42), module, RS16Step::PULSES_OUTS + (row * module->steps) + step));
		}

		// Add scale knob
		addParam(createParamCentered<RSKnobSml>(Vec(x, y), module, RS16Step::SCALE_KNOBS + row));
		y += 30;

		// Add offset knob
		addParam(createParamCentered<RSKnobSml>(Vec(x, y), module, RS16Step::OFFSET_KNOBS + row));
		x += smlGap; y -= 30;

		// Add CV out
		addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y), module, RS16Step::CV_OUTS + row));
		x += smlGap;

		// Add EOC out
		addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y), module, RS16Step::EOC_OUTS + row));
		x -= smlGap; y += 30;

		// Add door out
		addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y), module, RS16Step::DOOR_OUTS + row));
		x += smlGap;

		// add pulse out
		addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y), module, RS16Step::PULSE_OUTS + row));
		x += smlGap; y -= 30;
	}

	void step() override {
		if(!module) return;

		// Pattern selection
		module->pattern = (int)module->params[RS16Step::PATTERN_KNOB].getValue();
		if(module->pattern != module->priorPattern) {
			INFO("Racket Science: New pattern");
			patternLabel->text = std::to_string(module->pattern);
			module->savePattern(module->priorPattern);
			module->loadPattern(module->pattern);
		}
		module->priorPattern = module->pattern;

		// Pattern copy & paste
		if(module->copyPatternTrigger.process(module->params[RS16Step::COPY_PATTERN_BUTTON].getValue())) {
			module->copyPattern();
		}

		if(module->pastePatternTrigger.process(module->params[RS16Step::PASTE_PATTERN_BUTTON].getValue())) {
			module->pastePattern();
		}

		// Row copy & paste
		for(int row = 0; row < module->rows; row++) {
			if(module->copyRowTrigger[row].process(module->params[RS16Step::COPY_ROW_BUTTONS + row].getValue())) {
				module->copyRow(row);
			}

			if(module->pasteRowTrigger[row].process(module->params[RS16Step::PASTE_ROW_BUTTONS + row].getValue())) {
				module->pasteRow(row);
			}
		}

		ModuleWidget::step();
	}

    void customDraw(const DrawArgs& args) {}
	#include "RSModuleWidgetDraw.hpp"
};

Model *modelRS16Step = createModel<RS16Step, RS16StepWidget>("RS16Step");
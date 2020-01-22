#include "plugin.hpp"

#include "RS.hpp"

struct RS16Step : RSModule {
	static const int steps = 16;
	static const int rows = 4;

	enum ParamIds {
		THEME_BUTTON,
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

		ENUMS(STEP_KNOBS, rows * steps),
		ENUMS(DOOR_BUTTONS, rows * steps),
		ENUMS(PULSE_BUTTONS, rows * steps),
		NUM_PARAMS
	};
	enum InputIds {
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

	dsp::BooleanTrigger themeTrigger;

	dsp::ClockDivider rateDivider;


	int stepIdx[rows];
	float phaseIn, priorPhaseIn;
	int phaseRow, priorPhaseRow;

	dsp::SchmittTrigger prevStepTrigger[rows];
	dsp::SchmittTrigger nextStepTrigger[rows];
	dsp::SchmittTrigger randStepTrigger[rows];
	
	dsp::SchmittTrigger writeStepTrigger[rows];

	// Change these to Schmitts, any difference?
	dsp::BooleanTrigger randAllTrigger[rows];
	dsp::BooleanTrigger randStepsTrigger[rows];
	dsp::BooleanTrigger randGatesTrigger[rows];
	dsp::BooleanTrigger randPulsesTrigger[rows];

	dsp::BooleanTrigger resetAllTrigger[rows];
	dsp::BooleanTrigger resetStepsTrigger[rows];
	dsp::BooleanTrigger resetGatesTrigger[rows];
	dsp::BooleanTrigger resetPulsesTrigger[rows];

	dsp::PulseGenerator stepPulse[rows][steps];
	bool pulse[rows][steps];

	dsp::PulseGenerator eocPulse[rows];
	bool eoc[rows];


	RS16Step() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(THEME_BUTTON, 0.f, 1.f, 0.f, "THEME");

		rateDivider.setDivision(48000); // Set to sample rate, should tick every second

		for(int row = 0; row < rows; row++) {
			stepIdx[row] = 0;
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

			for(int step = 0; step < steps; step++) {
				configParam(STEP_KNOBS + (row * steps) + step, -10.f, 10.f, 0.f, "STEP");
				configParam(DOOR_BUTTONS + (row * steps) + step, 0.f, 1.f, 0.f, "DOOR");
				configParam(PULSE_BUTTONS + (row * steps) + step, 0.f, 1.f, 0.f, "PULSE");
				pulse[row][step] = false;
			}
		}
	}

	void process(const ProcessArgs &args) override {
		if(themeTrigger.process(params[THEME_BUTTON].getValue())) {
			RSTheme++;
			if(RSTheme > RSGlobal.themeCount) RSTheme = 1;
		}

		if(rateDivider.process()) INFO("Racket Science: tick");

		for(int row = 0; row < rows; row++) {
			// Process phase step
			if(inputs[PHASE_STEP_INS + row].isConnected()) {
				phaseIn = RSclamp(inputs[PHASE_STEP_INS + row].getVoltage(), 0.f, 10.f);
				phaseRow = (int)RSscale(phaseIn, 0.f, 10.f, 0.f, 16.f);
				if(phaseRow < 16) stepIdx[row] = phaseRow; // Defensive, square waves overrun

				if(phaseIn < priorPhaseIn) eocPulse[row].trigger();
				priorPhaseIn = phaseIn;

				if(phaseRow != priorPhaseRow) {
					if(params[PULSE_BUTTONS + (row * steps) + stepIdx[row]].getValue()) stepPulse[row][stepIdx[row]].trigger();
				}
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
			}

			// Process write CV in
			if(inputs[WRITE_INS + row].isConnected()) {
				if(writeStepTrigger[row].process(inputs[WRITE_INS + row].getVoltage())) {
					params[STEP_KNOBS + (row * steps) + stepIdx[row]].setValue(RSclamp(inputs[CV_INS + row].getVoltage(), -10.f, 10.f));
				}
			}
			else if(writeStepTrigger[row].process(params[WRITE_BUTTONS + row].getValue())) {
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

			// Process reset all
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

			outputs[CV_OUTS + row].setVoltage(params[STEP_KNOBS + (row * steps) + stepIdx[row]].getValue());

			eoc[row] = eocPulse[row].process(1.f / args.sampleRate);
			outputs[EOC_OUTS + row].setVoltage(eoc[row] ? 10.f : 0.f);

			outputs[DOOR_OUTS + row].setVoltage(params[DOOR_BUTTONS + (row * steps) + stepIdx[row]].getValue() > 0.f ? 10.f : 0.f);

			pulse[row][stepIdx[row]] = stepPulse[row][stepIdx[row]].process(1.f / args.sampleRate);
			outputs[PULSE_OUTS + row].setVoltage(pulse[row][stepIdx[row]] ? 10.f : 0.f);
		}
	}

	void nextStep(int row) {
		stepIdx[row]++;
		if(stepIdx[row] == steps) {
			stepIdx[row] = 0;
			eocPulse[row].trigger();
		}
		if(params[PULSE_BUTTONS + (row * steps) + stepIdx[row]].getValue() > 0.f) stepPulse[row][stepIdx[row]].trigger();
	}

	void prevStep(int row) {
		stepIdx[row]--;
		if(stepIdx[row] < 0) {
			stepIdx[row] = steps - 1;
			eocPulse[row].trigger();
		}
		if(params[PULSE_BUTTONS + (row * steps) + stepIdx[row]].getValue() > 0.f) stepPulse[row][stepIdx[row]].trigger();
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

	void resetStep(int row) {
		stepIdx[row] = 0;
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

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
        json_object_set_new(rootJ, "theme", json_integer(RSTheme));

		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
        json_t* themeJ = json_object_get(rootJ, "theme");
        if(themeJ) RSTheme = json_integer_value(themeJ);
	}
};


struct RS16StepWidget : ModuleWidget {
	RS16Step* module;

	int x, y, smlGap, lrgGap, labOfs;

	RS16StepWidget(RS16Step *module) {
		setModule(module);
		this->module = module;

		box.size = Vec(RACK_GRID_WIDTH * 94, RACK_GRID_HEIGHT);
		int middle = box.size.x / 2 + 1;

		addParam(createParamCentered<RSButtonMomentaryInvisible>(Vec(box.pos.x + 5, box.pos.y + 5), module, RS16Step::THEME_BUTTON));

		addChild(new RSLabelCentered(middle, box.pos.y + 13, "4x16", 14, module));
		addChild(new RSLabelCentered(middle, box.size.y - 4, "Racket Science", 12, module));

		x = 30; y = 50;
		smlGap = 35; lrgGap = 65;
		labOfs = 20;

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
		y -= 55;

		// Skip over steps
		x += lrgGap;
		x += (lrgGap * module->steps);

		// Right side labels
		addChild(new RSLabelCentered(x, y - labOfs, "CV OUT", 10, module));
		x += smlGap;
		addChild(new RSLabelCentered(x, y - labOfs, "EOC", 10, module));
		x -= smlGap; y += 55;
		addChild(new RSLabelCentered(x, y, "DOOR", 10, module));
		x += smlGap;
		addChild(new RSLabelCentered(x, y, "PULSE", 10, module));
		x += smlGap; y -= 55;

		x = 30; y = 50;
		for(int row = 0, rowGap = 85; row < module->rows; row++, y += rowGap) addStepSeqRow(row, x, y);
	}

	void addStepSeqRow(int row, int x, int y) {
		// Add prev step in
		addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RS16Step::PREV_STEP_BUTTONS + row));
		addInput(createInputCentered<RSStealthJack>(Vec(x, y), module, RS16Step::PREV_STEP_INS + row));
		x += smlGap;

		// Add next step in
		addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RS16Step::NEXT_STEP_BUTTONS + row));
		addInput(createInputCentered<RSStealthJack>(Vec(x, y), module, RS16Step::NEXT_STEP_INS + row));
		x -= smlGap; y += 30;

		// Add phase step in
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y), module, RS16Step::PHASE_STEP_INS + row));
		x += smlGap;

		// Add rand step in
		addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RS16Step::RAND_STEP_BUTTONS + row));
		addInput(createInputCentered<RSStealthJack>(Vec(x, y), module, RS16Step::RAND_STEP_INS + row));
		x += smlGap; y -= 30;

		// Add CV in
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y), module, RS16Step::CV_INS + row));
		y += 30;

		// Add write in
		addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RS16Step::WRITE_BUTTONS + row));
		addInput(createInputCentered<RSStealthJack>(Vec(x, y), module, RS16Step::WRITE_INS + row));
		x += smlGap; y -= 30;
		
		// Add randomize all in
		addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RS16Step::RAND_ALL_BUTTONS + row));
		addInput(createInputCentered<RSStealthJack>(Vec(x, y), module, RS16Step::RAND_ALL_INS + row));
		x += smlGap;
		
		// Add randomize steps in
		addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RS16Step::RAND_STEPS_BUTTONS + row));
		addInput(createInputCentered<RSStealthJack>(Vec(x, y), module, RS16Step::RAND_STEPS_INS  + row ));
		x -= smlGap; y += 30;

		// Add randomize doors in
		addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RS16Step::RAND_DOORS_BUTTONS + row));
		addInput(createInputCentered<RSStealthJack>(Vec(x, y), module, RS16Step::RAND_DOORS_INS  + row ));
		x += smlGap;

		// Add randomize pulses in
		addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RS16Step::RAND_PULSES_BUTTONS + row));
		addInput(createInputCentered<RSStealthJack>(Vec(x, y), module, RS16Step::RAND_PULSES_INS  + row ));
		x += smlGap; y -= 30;

		// Add reset step in
		addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RS16Step::RESET_STEP_BUTTONS + row));
		addInput(createInputCentered<RSStealthJack>(Vec(x, y), module, RS16Step::RESET_STEP_INS + row));
		x += smlGap;

		// Add reset steps in
		addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RS16Step::RESET_STEPS_BUTTONS + row));
		addInput(createInputCentered<RSStealthJack>(Vec(x, y), module, RS16Step::RESET_STEPS_INS + row));
		x -= smlGap; y += 30;

		// Add reset doors in
		addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RS16Step::RESET_DOORS_BUTTONS + row));
		addInput(createInputCentered<RSStealthJack>(Vec(x, y), module, RS16Step::RESET_DOORS_INS + row));
		x += smlGap;

		// Add reset pulses in
		addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RS16Step::RESET_PULSES_BUTTONS + row));
		addInput(createInputCentered<RSStealthJack>(Vec(x, y), module, RS16Step::RESET_PULSES_INS + row));
		y -= 30;
		
		x += lrgGap;

		// Add step knobs & lights, door & pulse buttons
		for(int step = 0; step < module->steps; step++, x += lrgGap) {
			addParam(createParamCentered<RSKnobMed>(Vec(x, y), module, RS16Step::STEP_KNOBS + (row * module->steps) + step));
			addChild(createLightCentered<LargeLight<GreenLight>>(Vec(x, y), module, RS16Step::STEP_LIGHTS + (row * module->steps) + step));

			addParam(createParamCentered<RSButtonToggle>(Vec(x - 15, y + 42), module, RS16Step::DOOR_BUTTONS + (row * module->steps) + step));
			addParam(createParamCentered<RSButtonToggle>(Vec(x + 15, y + 42), module, RS16Step::PULSE_BUTTONS + (row * module->steps) + step));
		}

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

		ModuleWidget::step();
	}

    void customDraw(const DrawArgs& args) {}
	#include "RSModuleWidgetDraw.hpp"
};

Model *modelRS16Step = createModel<RS16Step, RS16StepWidget>("RS16Step");
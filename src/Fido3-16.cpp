#include "plugin.hpp"

#include "RS.hpp"

struct RSFido316 : RSModule {
    static const int steps = 16;
    static const int rows = 4;

    enum ParamIds {
        THEME_BUTTON,

        RATE_DIVIDER,

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
        RAND_PATTERN_IN,
        PREV_PATTERN_IN,

        RAND_PATTERN_STEPS_IN,
        RAND_PATTERN_DOORS_IN,
        RAND_PATTERN_PULSES_IN,
        RAND_PATTERN_IDXS_IN,
        RAND_PATTERN_SCALINGS_IN,
        RAND_PATTERN_ALL_IN,

        RESET_PATTERN_STEPS_IN,
        RESET_PATTERN_DOORS_IN,
        RESET_PATTERN_PULSES_IN,
        RESET_PATTERN_IDXS_IN,
        RESET_PATTERN_SCALINGS_IN,
        RESET_PATTERN_ALL_IN,

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

        ENUMS(RAW_CV_OUTS, rows),
        ENUMS(SCALED_CV_OUTS, rows),
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

    int stepIdx[rows], priorStepIdx[rows];
    int phaseStep[rows], priorPhaseStep[rows];

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

    float scaleDefault = .25f;
    float offsetDefault = 0.f;

    // Copy & paste buffers
    RowBuffer rowCPBuffer;


    //Pattern storage
    RSScribbleStrip *ssPatternDescription; // Maybe have individual row descriptions

    dsp::BooleanTrigger prevStepTrigger[rows], nextStepTrigger[rows], randStepTrigger[rows];
    
    dsp::SchmittTrigger randAllTrigger[rows], randStepsTrigger[rows], randDoorsTrigger[rows], randPulsesTrigger[rows];

    dsp::SchmittTrigger resetAllTrigger[rows], resetStepsTrigger[rows], resetDoorsTrigger[rows], resetPulsesTrigger[rows];
    bool doorsToggle = false, pulsesToggle = false;

    dsp::BooleanTrigger copyRowTrigger[rows], pasteRowTrigger[rows];

    dsp::PulseGenerator rowPulse[rows][steps];	bool pulse[rows][steps];

    dsp::PulseGenerator stepPulse[rows][steps];

    dsp::PulseGenerator eocPulse[rows];	bool eoc[rows];


    RSFido316() {
        INFO("Racket Science: %i params  %i inputs  %i outputs  %i lights", NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(THEME_BUTTON, 0.f, 1.f, 0.f, "THEME");


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

            configParam(SCALE_KNOBS + row, -.5f, .5f, scaleDefault, "SCALE");
            configParam(OFFSET_KNOBS + row, -2.5f, 2.5f, offsetDefault, "OFFSET");
        }

        for(int step = 0; step < steps; step++) {
            rowCPBuffer.step[step].step = 0.f;
            rowCPBuffer.step[step].door = false;
            rowCPBuffer.step[step].pulse = false;
        }
        rowCPBuffer.scale = scaleDefault;
        rowCPBuffer.offset = offsetDefault;

        rateDivider.setDivision(8);
    }

    void process(const ProcessArgs &args) override {
        if(themeTrigger.process(params[THEME_BUTTON].getValue())) {
            RSTheme++;
            if(RSTheme > RSGlobal.themeCount) RSTheme = 1;
        }

        if(rateDivider.process()) {
            // Process rows
            for(int row = 0; row < rows; row++) {
                // Process write CV in
                // Subtle bug solved, using a door to update a step via CV in & write could touch the next step if this is processed after 
                //   step processing as step index could change before door closes, only took one sample
                if(inputs[CV_INS + row].isConnected())
                    if(inputs[WRITE_INS + row].getVoltage() or params[WRITE_BUTTONS + row].getValue()) 
                        params[STEP_KNOBS + (row * steps) + stepIdx[row]].setValue(RSclamp(inputs[CV_INS + row].getVoltage(), -10.f, 10.f));

                // Process phase step
                if(inputs[PHASE_STEP_INS + row].isConnected()) {
                    float phaseIn = RSclamp(inputs[PHASE_STEP_INS + row].getVoltage(), 0.f, 10.f);
                    phaseStep[row] = (int)RSscale(phaseIn, 0.f, 10.f, 0.f, 16.f);
                    if(phaseStep[row] < 16) stepIdx[row] = phaseStep[row]; // Defensive, square waves overrun

                    // This should now trigger when we cross from first to last step in either direction, hopefully across all rows
                    if((priorPhaseStep[row] == steps - 1 && phaseStep[row] == 0) or
                       (priorPhaseStep[row] == 0 && phaseStep[row] == steps - 1))
                           eocPulse[row].trigger();

                    if(phaseStep[row] != priorPhaseStep[row]) {
                        if(params[PULSE_BUTTONS + (row * steps) + stepIdx[row]].getValue()) {
                            rowPulse[row][stepIdx[row]].trigger();
                            stepPulse[row][stepIdx[row]].trigger();
                        }
                    }
                    else rowPulse[row][stepIdx[row]].reset();

                    priorPhaseStep[row] = phaseStep[row];
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
                        if(randStepTrigger[row].process(inputs[RAND_STEP_INS + row].getVoltage())) randomizeStepIdx(row);
                    }
                    else if(randStepTrigger[row].process(params[RAND_STEP_BUTTONS + row].getValue())) randomizeStepIdx(row);

                    // Process pulses
                    if(stepIdx[row] != priorStepIdx[row]) {
                        if(params[PULSE_BUTTONS + (row * steps) + stepIdx[row]].getValue()) {
                            rowPulse[row][stepIdx[row]].trigger();
                            stepPulse[row][stepIdx[row]].trigger();
                        }
                    }
                    else {
                        rowPulse[row][stepIdx[row]].reset(); // Reset stepPulse too?  Can't remember why I did this.  COMMENT IN FUTURE!
                        stepPulse[row][stepIdx[row]].reset();
                    }
                    // Was it something to do with elongated pulses?  Retrigging before pulse expires?  Think about what we do & don't need to do
                    //   at audio rate
                    priorStepIdx[row] = stepIdx[row];
                }

                // Process randomize all
                if(inputs[RAND_ALL_INS + row].isConnected()) {
                    if(randAllTrigger[row].process(inputs[RAND_ALL_INS + row].getVoltage())) randomizeRow(row);
                }
                else if(randAllTrigger[row].process(params[RAND_ALL_BUTTONS + row].getValue())) randomizeRow(row);

                // Process randomize steps
                if(inputs[RAND_STEPS_INS + row].isConnected()) {
                    if(randStepsTrigger[row].process(inputs[RAND_STEPS_INS + row].getVoltage())) randomizeSteps(row);
                }
                else if(randStepsTrigger[row].process(params[RAND_STEPS_BUTTONS + row].getValue())) randomizeSteps(row);

                // Process randomize doors
                if(inputs[RAND_DOORS_INS + row].isConnected()) {
                    if(randDoorsTrigger[row].process(inputs[RAND_DOORS_INS + row].getVoltage())) randomizeDoors(row);
                }
                else if(randDoorsTrigger[row].process(params[RAND_DOORS_BUTTONS + row].getValue())) randomizeDoors(row);

                // Process randomize pulses
                if(inputs[RAND_PULSES_INS + row].isConnected()) {
                    if(randPulsesTrigger[row].process(inputs[RAND_PULSES_INS + row].getVoltage())) randomizePulses(row);
                }
                else if(randPulsesTrigger[row].process(params[RAND_PULSES_BUTTONS + row].getValue())) randomizePulses(row);

                // Process reset step
                if(inputs[RESET_STEP_INS + row].isConnected()) {
                    if(resetAllTrigger[row].process(inputs[RESET_STEP_INS + row].getVoltage())) resetStepIdx(row);
                }
                else if(resetAllTrigger[row].process(params[RESET_STEP_BUTTONS + row].getValue())) resetStepIdx(row);

                // Process reset steps
                if(inputs[RESET_STEPS_INS + row].isConnected()) {
                    if(resetStepsTrigger[row].process(inputs[RESET_STEPS_INS + row].getVoltage())) resetSteps(row);
                }
                else if(resetStepsTrigger[row].process(params[RESET_STEPS_BUTTONS + row].getValue())) resetSteps(row);

                // Process reset doors
                if(inputs[RESET_DOORS_INS + row].isConnected()) {
                    if(resetDoorsTrigger[row].process(inputs[RESET_DOORS_INS + row].getVoltage())) { resetDoors(row); doorsToggle = !doorsToggle; }
                }
                else if(resetDoorsTrigger[row].process(params[RESET_DOORS_BUTTONS + row].getValue())) { resetDoors(row); doorsToggle = !doorsToggle; }

                // Process reset pulses
                if(inputs[RESET_PULSES_INS + row].isConnected()) {
                    if(resetPulsesTrigger[row].process(inputs[RESET_PULSES_INS + row].getVoltage())) { resetPulses(row); pulsesToggle = !pulsesToggle; }
                }
                else if(resetPulsesTrigger[row].process(params[RESET_PULSES_BUTTONS + row].getValue())) { resetPulses(row); pulsesToggle = !pulsesToggle; }
                
                // Set lights
                for(int step = 0; step < steps; step++) {
                    lights[STEP_LIGHTS + (row * steps) + step].setBrightness(step == stepIdx[row] ? 1.f : 0.f);
                }

                float cvOut = params[STEP_KNOBS + (row * steps) + stepIdx[row]].getValue();
                
                // Ouput raw CV
                outputs[RAW_CV_OUTS + row].setVoltage(cvOut);

                // Output scaled CV
                cvOut = cvOut * params[SCALE_KNOBS + row].getValue() + params[OFFSET_KNOBS + row].getValue();
                cvOut = RSclamp(cvOut, -10.f, 10.f);
                outputs[SCALED_CV_OUTS + row].setVoltage(cvOut);

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
                    if(step == stepIdx[row]) outputs[DOORS_OUTS + (row * steps) + step].setVoltage(params[DOOR_BUTTONS + (row * steps) + step].getValue() ? 10.f : 0.f);
                    else outputs[DOORS_OUTS + (row * steps) + step].setVoltage(0.f);
                    outputs[PULSES_OUTS + (row * steps) + step].setVoltage(stepPulse[row][step].process(1.f /  args.sampleRate) ? 10.f : 0.f);
                }
            }
        }
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

    void randomizeStepIdx(int row) { stepIdx[row] = rand() % steps; }

    void randomizeRow(int row) {
        randomizeSteps(row);
        randomizeDoors(row);
        randomizePulses(row);
        randomizeScalings(row);
    }

    void randomizeSteps(int row) { for(int step = 0; step < steps; step++) params[STEP_KNOBS + (row * steps) + step].setValue((((float)rand() / (float)RAND_MAX) * 20.f) - 10.f); }
    void randomizeDoors(int row) { for(int step = 0; step < steps; step++) params[DOOR_BUTTONS + (row * steps) + step].setValue(rand() %2); }
    void randomizePulses(int row) { for(int step = 0; step < steps; step++) params[PULSE_BUTTONS + (row * steps) + step].setValue(rand() %2); }

    void randomizeScalings(int row) {
        params[SCALE_KNOBS + row].setValue((((float)rand() / (float)RAND_MAX) * 1.f) - .5f);
        params[OFFSET_KNOBS + row].setValue((((float)rand() / (float)RAND_MAX) * 5.f) - 2.5f);
    }

    void onReset() override {
        for(int row = 0; row < rows; row++) {
            resetStepIdx(row);
            resetSteps(row);
            resetDoors(row);
            resetPulses(row);
            resetScalings(row);
        }
    }

    void resetStepIdx(int row) { stepIdx[row] = 0; eocPulse[row].trigger(); }
    void resetSteps(int row) { for(int step = 0; step < steps; step++) params[STEP_KNOBS + (row * steps) + step].setValue(0.f); }
    void resetDoors(int row) { for(int step = 0; step < steps; step++) params[DOOR_BUTTONS + (row * steps) + step].setValue(doorsToggle ? 1.f : 0.f); }
    void resetPulses(int row) { for(int step = 0; step < steps; step++) params[PULSE_BUTTONS + (row * steps) + step].setValue(pulsesToggle? 1.f : 0.f); }
    void resetScalings(int row) { params[SCALE_KNOBS + row].setValue(scaleDefault); params[OFFSET_KNOBS + row].setValue(offsetDefault); }

    void copyRow(int row) {
        for(int step = 0; step < steps; step++) {
            rowCPBuffer.step[step].step = params[STEP_KNOBS + (row * steps) + step].getValue();
            rowCPBuffer.step[step].door = params[DOOR_BUTTONS + (row * steps) + step].getValue();
            rowCPBuffer.step[step].pulse = params[PULSE_BUTTONS + (row * steps) + step].getValue();
        }
        rowCPBuffer.scale = params[SCALE_KNOBS + row].getValue();
        rowCPBuffer.offset = params[OFFSET_KNOBS + row].getValue();		
    }

    void pasteRow(int row) {
        for(int step = 0; step < steps; step++) {
            params[STEP_KNOBS + (row * steps) + step].setValue(rowCPBuffer.step[step].step);
            params[DOOR_BUTTONS + (row * steps) + step].setValue(rowCPBuffer.step[step].door);
            params[PULSE_BUTTONS + (row * steps) + step].setValue(rowCPBuffer.step[step].pulse);
        }
        params[SCALE_KNOBS + row].setValue(rowCPBuffer.scale);
        params[OFFSET_KNOBS + row].setValue(rowCPBuffer.offset);
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


struct RSFido316Widget : ModuleWidget {
    RSFido316* module;

    int x, y, smlGap, lrgGap, labOfs;

    RSLabelCentered *patternLabel;

    RSFido316Widget(RSFido316 *module) {
        INFO("Racket Science: RSFido316Widget()");
        
        setModule(module);
        this->module = module;

        box.size = Vec(RACK_GRID_WIDTH * 97, RACK_GRID_HEIGHT);
        int middle = box.size.x / 2 + 1;

        addParam(createParamCentered<RSButtonMomentaryInvisible>(Vec(box.pos.x + 5, box.pos.y + 5), module, RSFido316::THEME_BUTTON));

        addChild(new RSLabelCentered(middle, box.pos.y + 13, "PHYDEAUX III 16", 14, module));
        addChild(new RSLabelCentered(middle, box.size.y - 4, "Racket Science", 12, module));

        x = 30; y = 50;
        smlGap = 30; lrgGap = 65;
        labOfs = 20;

        // Left side row labels
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
        addChild(new RSLabelCentered(x, y - labOfs, "ROW", 10, module));
        x += smlGap;
        addChild(new RSLabelCentered(x, y - labOfs, "STEPS", 10, module));
        x -= smlGap; y += 55;
        addChild(new RSLabelCentered(x, y, "DOORS", 10, module));
        x += smlGap;
        addChild(new RSLabelCentered(x, y, "PULSES", 10, module));
        x += smlGap; y -= 55;

        addChild(new RSLabelCentered(x + (smlGap / 2), y - (labOfs * 1.5), "RESET", 10, module));
        addChild(new RSLabelCentered(x, y - labOfs, "INDEX", 10, module));
        x += smlGap;
        addChild(new RSLabelCentered(x, y - labOfs, "STEPS", 10, module));
        x -= smlGap; y += 55;
        addChild(new RSLabelCentered(x, y, "DOORS", 10, module));
        x += smlGap;
        addChild(new RSLabelCentered(x, y, "PULSES", 10, module));
        x += smlGap; y -= 55;

        // Skip over steps
        x += (smlGap * 2) + (lrgGap * module->steps);

        // Right side labels
        addChild(new RSLabelCentered(x, y - labOfs, "SCALE", 10, module));
        y += 55;
        addChild(new RSLabelCentered(x, y, "OFFSET", 10, module));
        x += smlGap; y -= 55;

        addChild(new RSLabelCentered(x, y - labOfs, "DOOR", 10, module));
        x += smlGap;
        addChild(new RSLabelCentered(x, y - labOfs, "PULSE", 10, module));
        x += smlGap; 
        addChild(new RSLabelCentered(x, y - labOfs, "EOC", 10, module));
        
        y += 55; x -= smlGap * 2; x += 15;
        addChild(new RSLabelCentered(x, y, "RAW CV", 10, module));
        x += smlGap;
        addChild(new RSLabelCentered(x, y, "SCALED", 10, module));

        x = 30; y = 50;
        for(int row = 0, rowGap = 85; row < module->rows; row++, y += rowGap) addRow(row, x, y);
    }

    void addRow(int row, int x, int y) {
        // Add prev step in
        addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RSFido316::PREV_STEP_BUTTONS + row));
        addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RSFido316::PREV_STEP_INS + row));
        x += smlGap;

        // Add next step in
        addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RSFido316::NEXT_STEP_BUTTONS + row));
        addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RSFido316::NEXT_STEP_INS + row));
        x -= smlGap; y += 30;

        // Add phase step in
        addInput(createInputCentered<RSJackMonoIn>(Vec(x, y), module, RSFido316::PHASE_STEP_INS + row));
        x += smlGap;

        // Add rand step in
        addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RSFido316::RAND_STEP_BUTTONS + row));
        addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RSFido316::RAND_STEP_INS + row));
        x += smlGap; y -= 30;

        // Add CV in
        addInput(createInputCentered<RSJackMonoIn>(Vec(x, y), module, RSFido316::CV_INS + row));
        y += 30;

        // Add write in
        addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RSFido316::WRITE_BUTTONS + row));
        addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RSFido316::WRITE_INS + row));
        x += smlGap; y -= 30;
        
        // Add randomize all in
        addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RSFido316::RAND_ALL_BUTTONS + row));
        addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RSFido316::RAND_ALL_INS + row));
        x += smlGap;
        
        // Add randomize steps in
        addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RSFido316::RAND_STEPS_BUTTONS + row));
        addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RSFido316::RAND_STEPS_INS  + row ));
        x -= smlGap; y += 30;

        // Add randomize doors in
        addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RSFido316::RAND_DOORS_BUTTONS + row));
        addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RSFido316::RAND_DOORS_INS  + row ));
        x += smlGap;

        // Add randomize pulses in
        addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RSFido316::RAND_PULSES_BUTTONS + row));
        addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RSFido316::RAND_PULSES_INS  + row ));
        x += smlGap; y -= 30;

        // Add reset step in
        addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RSFido316::RESET_STEP_BUTTONS + row));
        addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RSFido316::RESET_STEP_INS + row));
        x += smlGap;

        // Add reset steps in
        addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RSFido316::RESET_STEPS_BUTTONS + row));
        addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RSFido316::RESET_STEPS_INS + row));
        x -= smlGap; y += 30;

        // Add reset doors in
        addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RSFido316::RESET_DOORS_BUTTONS + row));
        addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RSFido316::RESET_DOORS_INS + row));
        x += smlGap;

        // Add reset pulses in
        addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RSFido316::RESET_PULSES_BUTTONS + row));
        addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RSFido316::RESET_PULSES_INS + row));
        x += smlGap; y -= 30;

        // Add copy & paste
        addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSFido316::COPY_ROW_BUTTONS + row));
        addChild(new RSLabelCentered(x, y + 3, "COPY", 10, module));
        y += 30;
        addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSFido316::PASTE_ROW_BUTTONS + row));
        addChild(new RSLabelCentered(x, y + 3, "PASTE", 10, module));
        x += smlGap; y -= 30;
        
        x += smlGap;

        // Add step knobs & lights, door & pulse buttons, door & pulse outputs
        for(int step = 0; step < module->steps; step++, x += lrgGap) {
            addParam(createParamCentered<RSKnobMed>(Vec(x, y), module, RSFido316::STEP_KNOBS + (row * module->steps) + step));
            addChild(createLightCentered<LargeLight<GreenLight>>(Vec(x, y), module, RSFido316::STEP_LIGHTS + (row * module->steps) + step));

            addParam(createParamCentered<RSButtonToggle>(Vec(x - 15, y + 42), module, RSFido316::DOOR_BUTTONS + (row * module->steps) + step));
            addOutput(createOutputCentered<RSStealthJackSmallMonoOut>(Vec(x - 15, y + 42), module, RSFido316::DOORS_OUTS + (row * module->steps) + step));

            addParam(createParamCentered<RSButtonToggle>(Vec(x + 15, y + 42), module, RSFido316::PULSE_BUTTONS + (row * module->steps) + step));
            addOutput(createOutputCentered<RSStealthJackSmallMonoOut>(Vec(x + 15, y + 42), module, RSFido316::PULSES_OUTS + (row * module->steps) + step));
        }

        // Add scale knob
        addParam(createParamCentered<RSKnobSml>(Vec(x, y), module, RSFido316::SCALE_KNOBS + row));
        y += 30;

        // Add offset knob
        addParam(createParamCentered<RSKnobSml>(Vec(x, y), module, RSFido316::OFFSET_KNOBS + row));
        x += smlGap; y -= 30;

        // Add door out
        addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y), module, RSFido316::DOOR_OUTS + row));
        x += smlGap;

        // add pulse out
        addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y), module, RSFido316::PULSE_OUTS + row));
        x += smlGap;

        // Add EOC out
        addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y), module, RSFido316::EOC_OUTS + row));
        x -= smlGap * 2; x += 15; y += 30;

        // Add raw CV out
        addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y), module, RSFido316::RAW_CV_OUTS + row));
        x += smlGap;

        // Add scaled CV out
        addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y), module, RSFido316::SCALED_CV_OUTS + row));
    }

    void step() override {
        if(!module) return;

        // Row copy & paste
        for(int row = 0; row < module->rows; row++) {
            if(module->copyRowTrigger[row].process(module->params[RSFido316::COPY_ROW_BUTTONS + row].getValue())) {
                module->copyRow(row);
            }
            if(module->pasteRowTrigger[row].process(module->params[RSFido316::PASTE_ROW_BUTTONS + row].getValue())) {
                module->pasteRow(row);
            }
        }

        ModuleWidget::step();
    }

    void customDraw(const DrawArgs& args) {}
    #include "RSModuleWidgetDraw.hpp"
};

Model *modelRSFido316 = createModel<RSFido316, RSFido316Widget>("RSFido316");
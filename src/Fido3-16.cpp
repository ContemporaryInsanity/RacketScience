#include "plugin.hpp"

#include "RS.hpp"

struct RSFido316 : RSModule {
    static const int patterns = 16;
    static const int steps = 16;
    static const int rows = 4;

    enum ParamIds {
        THEME_BUTTON,

        RATE_DIVIDER,

        PATTERN_KNOB,
        COPY_PATTERN_BUTTON, 
        PASTE_PATTERN_BUTTON,

        NEXT_PATTERN_BUTTON,
        RAND_PATTERN_BUTTON,
        PREV_PATTERN_BUTTON,

        RAND_PATTERN_STEPS_BUTTON,
        RAND_PATTERN_DOORS_BUTTON,
        RAND_PATTERN_PULSES_BUTTON,
        RAND_PATTERN_IDXS_BUTTON,
        RAND_PATTERN_SCALINGS_BUTTON,
        RAND_PATTERN_ALL_BUTTON,

        RESET_PATTERN_STEPS_BUTTON,
        RESET_PATTERN_DOORS_BUTTON,
        RESET_PATTERN_PULSES_BUTTON,
        RESET_PATTERN_IDXS_BUTTON,
        RESET_PATTERN_SCALINGS_BUTTON,
        RESET_PATTERN_ALL_BUTTON,

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

    struct PatternBuffer {
        char description[30];
        RowBuffer rowBuffer[rows];
    };

    // Copy & paste buffers
    RowBuffer rowBuffer;
    PatternBuffer patternBuffer;

    //Pattern storage
    PatternBuffer patternStore[patterns] = {};
    RSScribbleStrip *ssPatternDescription;
    int pattern, priorPattern;
    

    dsp::BooleanTrigger copyPatternTrigger, pastePatternTrigger;

    dsp::BooleanTrigger prevPatternTrigger, randPatternTrigger, nextPatternTrigger;

    dsp::BooleanTrigger randPatternStepsTrigger, randPatternDoorsTrigger, randPatternPulsesTrigger;
    dsp::BooleanTrigger randPatternIdxsTrigger,  randPatternScalingsTrigger, randPatternAllTrigger;

    dsp::BooleanTrigger resetPatternStepsTrigger, resetPatternDoorsTrigger, resetPatternPulsesTrigger;
    dsp::BooleanTrigger resetPatternIdxsTrigger, resetPatternScalingsTrigger, resetPatternAllTrigger;

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

        configParam(PATTERN_KNOB, 0.f, (float)patterns - 1, 0.f, "PATTERN SELECT");
        configParam(COPY_PATTERN_BUTTON, 0.f, 1.f, 0.f, "COPY PATTERN");
        configParam(PASTE_PATTERN_BUTTON, 0.f, 1.f, 0.f, "PASTE PATTERN");

        configParam(PREV_PATTERN_BUTTON, 0.f, 1.f, 0.f, "PREV PATTERN");
        configParam(RAND_PATTERN_BUTTON, 0.f, 1.f, 0.f, "RAND PATTERN");
        configParam(NEXT_PATTERN_BUTTON, 0.f, 1.f, 0.f, "NEXT PATTERN");

        configParam(RAND_PATTERN_STEPS_BUTTON, 0.f, 1.f, 0.f, "RAND STEPS");
        configParam(RAND_PATTERN_DOORS_BUTTON, 0.f, 1.f, 0.f, "RAND DOORS");
        configParam(RAND_PATTERN_PULSES_BUTTON, 0.f, 1.f, 0.f, "RAND PULSES");
        configParam(RAND_PATTERN_IDXS_BUTTON, 0.f, 1.f, 0.f, "RAND IDXS");
        configParam(RAND_PATTERN_SCALINGS_BUTTON, 0.f, 1.f, 0.f, "RAND SCALINGS");
        configParam(RAND_PATTERN_ALL_BUTTON, 0.f, 1.f, 0.f, "RAND ALL");

        configParam(RESET_PATTERN_STEPS_BUTTON, 0.f, 1.f, 0.f, "RESET STEPS");
        configParam(RESET_PATTERN_DOORS_BUTTON, 0.f, 1.f, 0.f, "RESET DOORS");
        configParam(RESET_PATTERN_PULSES_BUTTON, 0.f, 1.f, 0.f, "RESET PULSES");
        configParam(RESET_PATTERN_IDXS_BUTTON, 0.f, 1.f, 0.f, "RESET IDXS");
        configParam(RESET_PATTERN_SCALINGS_BUTTON, 0.f, 1.f, 0.f, "RESET SCALINGS");
        configParam(RESET_PATTERN_ALL_BUTTON, 0.f, 1.f, 0.f, "RESET ALL");

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
            rowBuffer.step[step].step = 0.f;
            rowBuffer.step[step].door = false;
            rowBuffer.step[step].pulse = false;
        }
        rowBuffer.scale = scaleDefault;
        rowBuffer.offset = offsetDefault;

        for(int row = 0; row < rows; row++) {
            for(int step = 0; step < steps; step++) {
                patternBuffer.rowBuffer[row].step[step].step = 0.f;
                patternBuffer.rowBuffer[row].step[step].door = false;
                patternBuffer.rowBuffer[row].step[step].pulse = false;
            }
            patternBuffer.rowBuffer[row].scale = scaleDefault;
            patternBuffer.rowBuffer[row].offset = offsetDefault;
        }

        for(int pattern = 0; pattern < patterns; pattern++) {
            sprintf(patternStore[pattern].description, "Pattern %i", pattern + 1);
            for(int row = 0; row < rows; row++) {
                for(int step = 0; step < steps; step++) {
                    patternStore[pattern].rowBuffer[row].step[step].step = 0.f;
                    patternStore[pattern].rowBuffer[row].step[step].door = false;
                    patternStore[pattern].rowBuffer[row].step[step].pulse = false;
                }
                patternStore[pattern].rowBuffer[row].scale = scaleDefault;
                patternStore[pattern].rowBuffer[row].offset = offsetDefault;
            }
        }

        pattern = 0; priorPattern = patterns - 1;

        rateDivider.setDivision(8);
    }

    void process(const ProcessArgs &args) override {
        if(themeTrigger.process(params[THEME_BUTTON].getValue())) {
            RSTheme++;
            if(RSTheme > RSGlobal.themeCount) RSTheme = 1;
        }

        if(rateDivider.process()) {
            // Process prev / rand / next pattern
            if(inputs[PREV_PATTERN_IN].isConnected()) {
                if(prevPatternTrigger.process(inputs[PREV_PATTERN_IN].getVoltage())) prevPattern();
            }
            else if(prevPatternTrigger.process(params[PREV_PATTERN_BUTTON].getValue())) prevPattern();

            if(inputs[RAND_PATTERN_IN].isConnected()) {
                if(randPatternTrigger.process(inputs[RAND_PATTERN_IN].getVoltage())) randomPattern();
            }
            else if(randPatternTrigger.process(params[RAND_PATTERN_BUTTON].getValue())) randomPattern();

            if(inputs[NEXT_PATTERN_IN].isConnected()) {
                if(nextPatternTrigger.process(inputs[NEXT_PATTERN_IN].getVoltage())) nextPattern();
            }
            else if(nextPatternTrigger.process(params[NEXT_PATTERN_BUTTON].getValue())) nextPattern();

            // Process pattern randomizations
            if(inputs[RAND_PATTERN_STEPS_IN].isConnected()) {
                if(randPatternStepsTrigger.process(inputs[RAND_PATTERN_STEPS_IN].getVoltage())) randomizePatternSteps();
            }
            else if(randPatternStepsTrigger.process(params[RAND_PATTERN_STEPS_BUTTON].getValue())) randomizePatternSteps();

            if(inputs[RAND_PATTERN_DOORS_IN].isConnected()) {
                if(randPatternDoorsTrigger.process(inputs[RAND_PATTERN_DOORS_IN].getVoltage())) randomizePatternDoors();
            }
            else if(randPatternDoorsTrigger.process(params[RAND_PATTERN_DOORS_BUTTON].getValue())) randomizePatternDoors();

            if(inputs[RAND_PATTERN_PULSES_IN].isConnected()) {
                if(randPatternPulsesTrigger.process(inputs[RAND_PATTERN_PULSES_IN].getVoltage())) randomizePatternPulses();
            }
            else if(randPatternPulsesTrigger.process(params[RAND_PATTERN_PULSES_BUTTON].getValue())) randomizePatternPulses();

            if(inputs[RAND_PATTERN_IDXS_IN].isConnected()) {
                if(randPatternIdxsTrigger.process(inputs[RAND_PATTERN_IDXS_IN].getVoltage())) randomizePatternIdxs();
            }
            else if(randPatternIdxsTrigger.process(params[RAND_PATTERN_IDXS_BUTTON].getValue())) randomizePatternIdxs();

            if(inputs[RAND_PATTERN_SCALINGS_IN].isConnected()) {
                if(randPatternScalingsTrigger.process(inputs[RAND_PATTERN_SCALINGS_IN].getVoltage())) randomizePatternScalings();
            }
            else if(randPatternScalingsTrigger.process(params[RAND_PATTERN_SCALINGS_BUTTON].getValue())) randomizePatternScalings();

            if(inputs[RAND_PATTERN_ALL_IN].isConnected()) {
                if(randPatternAllTrigger.process(inputs[RAND_PATTERN_ALL_IN].getVoltage())) randomizePatternAll();
            }
            else if(randPatternAllTrigger.process(params[RAND_PATTERN_ALL_BUTTON].getValue())) randomizePatternAll();

            // Process pattern resets
            if(inputs[RESET_PATTERN_STEPS_IN].isConnected()) {
                if(resetPatternStepsTrigger.process(inputs[RESET_PATTERN_STEPS_IN].getVoltage())) resetPatternSteps();
            }
            else if(resetPatternStepsTrigger.process(params[RESET_PATTERN_STEPS_BUTTON].getValue())) resetPatternSteps();

            if(inputs[RESET_PATTERN_DOORS_IN].isConnected()) {
                if(resetPatternDoorsTrigger.process(inputs[RESET_PATTERN_DOORS_IN].getVoltage())) { resetPatternDoors(); doorsToggle = !doorsToggle; }
            }
            else if(resetPatternDoorsTrigger.process(params[RESET_PATTERN_DOORS_BUTTON].getValue())) { resetPatternDoors(); doorsToggle = !doorsToggle; }

            if(inputs[RESET_PATTERN_PULSES_IN].isConnected()) {
                if(resetPatternPulsesTrigger.process(inputs[RESET_PATTERN_PULSES_IN].getVoltage())) { resetPatternPulses(); pulsesToggle = !pulsesToggle; }
            }
            else if(resetPatternPulsesTrigger.process(params[RESET_PATTERN_PULSES_BUTTON].getValue())) { resetPatternPulses(); pulsesToggle = !pulsesToggle; }

            if(inputs[RESET_PATTERN_IDXS_IN].isConnected()) {
                if(resetPatternIdxsTrigger.process(inputs[RESET_PATTERN_IDXS_IN].getVoltage())) resetPatternIdxs();
            }
            else if(resetPatternIdxsTrigger.process(params[RESET_PATTERN_IDXS_BUTTON].getValue())) resetPatternIdxs();

            if(inputs[RESET_PATTERN_SCALINGS_IN].isConnected()) {
                if(resetPatternScalingsTrigger.process(inputs[RESET_PATTERN_SCALINGS_IN].getVoltage())) resetPatternScalings();
            }
            else if(resetPatternScalingsTrigger.process(params[RESET_PATTERN_SCALINGS_BUTTON].getValue())) resetPatternScalings();

            if(inputs[RESET_PATTERN_ALL_IN].isConnected()) {
                if(resetPatternAllTrigger.process(inputs[RESET_PATTERN_ALL_IN].getVoltage())) resetPatternAll();
            }
            else if(resetPatternAllTrigger.process(params[RESET_PATTERN_ALL_BUTTON].getValue())) resetPatternAll();

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
                        if(params[PULSE_BUTTONS + (row * steps) + stepIdx[row]].getValue()) rowPulse[row][stepIdx[row]].trigger();
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
                        if(params[PULSE_BUTTONS + (row * steps) + stepIdx[row]].getValue()) rowPulse[row][stepIdx[row]].trigger();
                    }
                    else rowPulse[row][stepIdx[row]].reset();
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

    void randomPattern() {
        pattern = rand() % patterns;
        params[PATTERN_KNOB].setValue(pattern);
    }

    void prevPattern() {
        if(pattern == 0) pattern = patterns - 1;
        else pattern--;
        params[PATTERN_KNOB].setValue(pattern);
    }

    void randomizePatternSteps() { for(int row = 0; row < rows; row++) randomizeSteps(row); }
    void randomizePatternDoors() { for(int row = 0; row < rows; row++) randomizeDoors(row); }
    void randomizePatternPulses() { for(int row = 0; row < rows; row++) randomizePulses(row); }
    void randomizePatternIdxs() { for(int row = 0; row < rows; row++) randomizeStepIdx(row); }
    void randomizePatternScalings() { for(int row = 0; row < rows; row++) randomizeScalings(row); }

    void randomizePatternAll() {
        randomizePatternSteps();
        randomizePatternDoors();
        randomizePatternPulses();
        randomizePatternScalings();
    }

    void resetPatternIdxs() { for(int row = 0; row < rows; row++) resetStepIdx(row); }
    void resetPatternSteps() { for(int row = 0; row < rows; row++) resetSteps(row); }
    void resetPatternDoors() { for(int row = 0; row < rows; row++) resetDoors(row); }
    void resetPatternPulses() { for(int row = 0; row < rows; row++) resetPulses(row); }
    void resetPatternScalings() { for(int row = 0; row < rows; row++) resetScalings(row); }

    void resetPatternAll() {
        resetPatternIdxs();
        resetPatternSteps();
        resetPatternDoors();
        resetPatternPulses();
        resetPatternScalings();
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

        savePattern(pattern);

        for(int pattern = 0; pattern < patterns; pattern++) {
            json_t* patternJ = json_object();
            json_object_set_new(patternJ, "description", json_string(patternStore[pattern].description));

            for(int row = 0; row < rows; row++) {
                json_t* rowJ = json_object();
                json_object_set_new(rowJ, "scale", json_real(patternStore[pattern].rowBuffer[row].scale));
                json_object_set_new(rowJ, "offset", json_real(patternStore[pattern].rowBuffer[row].offset));

                for(int step = 0; step < steps; step++) {
                    json_t* stepJ = json_object();
                    json_object_set_new(stepJ, "step", json_real(patternStore[pattern].rowBuffer[row].step[step].step));
                    json_object_set_new(stepJ, "door", json_boolean(patternStore[pattern].rowBuffer[row].step[step].door));
                    json_object_set_new(stepJ, "pulse", json_boolean(patternStore[pattern].rowBuffer[row].step[step].pulse));
                    json_object_set_new(rowJ, ("step" + std::to_string(step)).c_str(), stepJ);
                }
                json_object_set_new(patternJ, ("row" + std::to_string(row)).c_str(), rowJ);
            }
            json_object_set_new(rootJ, ("pattern" + std::to_string(pattern)).c_str(), patternJ);
        }
        
        return rootJ;
    }

    void dataFromJson(json_t* rootJ) override {
        json_t* themeJ = json_object_get(rootJ, "theme");
        if(themeJ) RSTheme = json_integer_value(themeJ);

        for(int pattern = 0; pattern < patterns; pattern++) {
            json_t* patternJ = json_object_get(rootJ, ("pattern" + std::to_string(pattern)).c_str());
            if(patternJ) {
                json_t* descriptionJ = json_object_get(patternJ, "description");
                if(descriptionJ) strcpy(patternStore[pattern].description, json_string_value(descriptionJ));

                for(int row = 0; row < rows; row++) {
                    json_t* rowJ = json_object_get(patternJ, ("row" + std::to_string(row)).c_str());
                    if(rowJ) {
                        json_t* scaleJ = json_object_get(rowJ, "scale");
                        if(scaleJ) patternStore[pattern].rowBuffer[row].scale = json_real_value(scaleJ);
                        json_t* offsetJ = json_object_get(rowJ, "offset");
                        if(offsetJ) patternStore[pattern].rowBuffer[row].offset = json_real_value(offsetJ);
                    }

                    for(int step = 0; step < steps; step++) {
                        json_t* stepsJ = json_object_get(rowJ, ("step" + std::to_string(step)).c_str());
                        if(stepsJ) {
                            json_t* stepJ = json_object_get(stepsJ, "step");
                            if(stepJ) patternStore[pattern].rowBuffer[row].step[step].step = json_real_value(stepJ);
                            json_t* doorJ = json_object_get(stepsJ, "door");
                            if(doorJ) patternStore[pattern].rowBuffer[row].step[step].door = json_boolean_value(doorJ);
                            json_t* pulseJ = json_object_get(stepsJ, "pulse");
                            if(pulseJ) patternStore[pattern].rowBuffer[row].step[step].pulse = json_boolean_value(pulseJ);
                        }
                    }
                }
            }
        }
        loadPattern(pattern);
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

        box.size = Vec(RACK_GRID_WIDTH * 104, RACK_GRID_HEIGHT);
        int middle = box.size.x / 2 + 1;

        addParam(createParamCentered<RSButtonMomentaryInvisible>(Vec(box.pos.x + 5, box.pos.y + 5), module, RSFido316::THEME_BUTTON));

        addChild(new RSLabelCentered(middle, box.pos.y + 13, "PHYDEAUX III 16", 14, module));
        addChild(new RSLabelCentered(middle, box.size.y - 4, "Racket Science", 12, module));

        x = 60; y = 50;
        smlGap = 30; lrgGap = 65;
        labOfs = 20;

        // Pattern section
        addChild(new RSLabelCentered(x, y - (labOfs * 1.5), "PATTERN", 10, module));

        // Pattern copy & paste
        addParam(createParamCentered<RSButtonMomentary>(Vec(x - 30, y - smlGap + 3), module, RSFido316::COPY_PATTERN_BUTTON));
        addChild(new RSLabelCentered(x - 30, y - smlGap + 6, "COPY", 10, module));

        addParam(createParamCentered<RSButtonMomentary>(Vec(x + 30, y - smlGap + 3), module, RSFido316::PASTE_PATTERN_BUTTON));
        addChild(new RSLabelCentered(x + 30, y - smlGap + 6, "PASTE", 10, module));
        y += smlGap;

        // Pattern knob
        addParam(createParamCentered<RSKnobDetentLrg>(Vec(x, y), module, RSFido316::PATTERN_KNOB));
        patternLabel = new RSLabelCentered(x, y + 5, "0", 22, module);
        addChild(patternLabel);
        y += smlGap + 12;

        // Pattern name scribble strip
        if(module) {
            addChild(module->ssPatternDescription = new RSScribbleStrip(x - 50, y, 100));
            module->ssPatternDescription->setText(module->patternStore[(int)module->params[RSFido316::PATTERN_KNOB].getValue()].description);
        }
        y += smlGap - 2;

        // Pattern prev / rand / next ins
        addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x - 30, y), module, RSFido316::PREV_PATTERN_BUTTON));
        addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x - 30, y), module, RSFido316::PREV_PATTERN_IN));
        addChild(new RSLabelCentered(x - 30, y + 23, "PREV", 10, module));

        addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RSFido316::RAND_PATTERN_BUTTON));
        addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RSFido316::RAND_PATTERN_IN));
        addChild(new RSLabelCentered(x, y + 23, "RAND", 10, module));

        addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x + 30, y), module, RSFido316::NEXT_PATTERN_BUTTON));
        addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x + 30, y), module, RSFido316::NEXT_PATTERN_IN));
        addChild(new RSLabelCentered(x + 30, y + 23, "NEXT", 10, module));

        y += smlGap + 10;

        // Randomize
        addChild(new RSLabelCentered(x, y, "RANDOMIZE", 10, module));
        x = 30; y += 18;

        addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RSFido316::RAND_PATTERN_STEPS_BUTTON));
        addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RSFido316::RAND_PATTERN_STEPS_IN));
        addChild(new RSLabelCentered(x, y + 23, "STEPS", 10, module));
        x += smlGap;

        addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RSFido316::RAND_PATTERN_DOORS_BUTTON));
        addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RSFido316::RAND_PATTERN_DOORS_IN));
        addChild(new RSLabelCentered(x, y + 23, "DOORS", 10, module));
        x += smlGap;

        addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RSFido316::RAND_PATTERN_PULSES_BUTTON));
        addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RSFido316::RAND_PATTERN_PULSES_IN));
        addChild(new RSLabelCentered(x, y + 23, "PULSES", 10, module));
        x = 30; y += smlGap + 10;

        addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RSFido316::RAND_PATTERN_SCALINGS_BUTTON));
        addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RSFido316::RAND_PATTERN_SCALINGS_IN));
        addChild(new RSLabelCentered(x, y + 23, "SCALING", 10, module));
        x += smlGap;

        addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RSFido316::RAND_PATTERN_IDXS_BUTTON));
        addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RSFido316::RAND_PATTERN_IDXS_IN));
        addChild(new RSLabelCentered(x, y + 23, "INDEX", 10, module));
        x += smlGap;

        addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RSFido316::RAND_PATTERN_ALL_BUTTON));
        addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RSFido316::RAND_PATTERN_ALL_IN));
        addChild(new RSLabelCentered(x, y + 23, "ALL", 10, module));
        y += smlGap + 10;

        // Reset
        x = 60;
        addChild(new RSLabelCentered(x, y, "RESET", 10, module));
        x = 30; y += 18;

        addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RSFido316::RESET_PATTERN_STEPS_BUTTON));
        addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RSFido316::RESET_PATTERN_STEPS_IN));
        addChild(new RSLabelCentered(x, y + 23, "STEPS", 10, module));
        x += smlGap;

        addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RSFido316::RESET_PATTERN_DOORS_BUTTON));
        addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RSFido316::RESET_PATTERN_DOORS_IN));
        addChild(new RSLabelCentered(x, y + 23, "DOORS", 10, module));
        x += smlGap;

        addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RSFido316::RESET_PATTERN_PULSES_BUTTON));
        addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RSFido316::RESET_PATTERN_PULSES_IN));
        addChild(new RSLabelCentered(x, y + 23, "PULSES", 10, module));
        x = 30; y += smlGap + 10;

        addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RSFido316::RESET_PATTERN_SCALINGS_BUTTON));
        addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RSFido316::RESET_PATTERN_SCALINGS_IN));
        addChild(new RSLabelCentered(x, y + 23, "SCALING", 10, module));
        x += smlGap;

        addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RSFido316::RESET_PATTERN_IDXS_BUTTON));
        addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RSFido316::RESET_PATTERN_IDXS_IN));
        addChild(new RSLabelCentered(x, y + 23, "INDEX", 10, module));
        x += smlGap;

        addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RSFido316::RESET_PATTERN_ALL_BUTTON));
        addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RSFido316::RESET_PATTERN_ALL_IN));
        addChild(new RSLabelCentered(x, y + 23, "ALL", 10, module));
        x += smlGap;

        // Left side row labels
        x = 130; y = 50;

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
        x += smlGap * 2;
        x += (lrgGap * module->steps);

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

        x = 130; y = 50;
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

        // Pattern selection
        module->pattern = (int)module->params[RSFido316::PATTERN_KNOB].getValue();
        if(module->pattern != module->priorPattern) {
            patternLabel->text = std::to_string(module->pattern + 1);
            module->savePattern(module->priorPattern);
            module->loadPattern(module->pattern);
            module->priorPattern = module->pattern;
        }

        // Pattern copy & paste
        if(module->copyPatternTrigger.process(module->params[RSFido316::COPY_PATTERN_BUTTON].getValue())) {
            module->copyPattern();
        }
        if(module->pastePatternTrigger.process(module->params[RSFido316::PASTE_PATTERN_BUTTON].getValue())) {
            module->pastePattern();
        }

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
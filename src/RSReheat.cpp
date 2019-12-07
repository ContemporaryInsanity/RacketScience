#include "plugin.hpp"

#include "components/RSComponents.hpp"
#include "RSUtils.hpp"

struct RSReheat : Module {
    enum ParamIds {
        THEME_BUTTON,
        RESET_BUTTON,
        GAIN_KNOB,
        LOSS_KNOB,
        NUM_PARAMS
    };
    enum InputIds {
        CV_INPUT,
        GATE_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        COOLEST_SEMI_OUTPUT,
        WARMEST_SEMI_OUTPUT,
        COOLEST_OCT_OUTPUT,
        WARMEST_OCT_OUTPUT,
        QUANTUM_NOTE_OUTPUT,    // These will set ML Quantum to current scale
        QUANTUM_TOGGLE_OUTPUT,
        QUANTUM_RESET_OUTPUT,
        ENUMS(SEMITONE_OUTPUTS, 12),
        ENUMS(OCTAVE_OUTPUTS, 10),
        NUM_OUTPUTS
    };
    enum LightIds {
        ENUMS(SEMITONE_LIGHTS, 12),
        ENUMS(OCTAVE_LIGHTS, 10),
        NUM_LIGHTS
    };

    dsp::ClockDivider logDivider;
    dsp::ClockDivider lightDivider;
    dsp::ClockDivider fadeDivider;

    dsp::BooleanTrigger themeTrigger;
    dsp::SchmittTrigger gateTrigger;
    dsp::BooleanTrigger resetTrigger;

    float semiHeat[12] = {};
    float octHeat[10] = {};
    float warmestSemi = 0.f;
    float coolestSemi = 0.f;
    float warmestOct = 0.f;
    float coolestOct = 0.f;
    float heatGain = 0.1f;
    float heatLoss = 0.1f;

    RSReheat() {
        logDivider.setDivision(4096);

		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(THEME_BUTTON, 0.f, 1.f, 0.f, "THEME");
        configParam(GAIN_KNOB, 0.01f, 5.0f, 0.05f, "GAIN");
        configParam(LOSS_KNOB, 0.01f, 0.5f, 0.05f, "LOSS");
    }

    void process(const ProcessArgs &args) override {
        if(themeTrigger.process(params[THEME_BUTTON].getValue())) {
            RSTheme++;
            if(RSTheme > RSThemes) RSTheme = 0;
            saveDefaultTheme(RSTheme);
        }

        float cvIn = RSclamp(inputs[CV_INPUT].getVoltage(), -10.f, 10.f);
        int noteIdx = note(cvIn);
        int octIdx = clamp(octave(cvIn) + 4, 0, 9);

        heatGain = params[GAIN_KNOB].getValue();
        heatLoss = params[LOSS_KNOB].getValue();
        
        if(gateTrigger.process(inputs[GATE_INPUT].getVoltage())) {
            if(semiHeat[noteIdx] < 10.f) semiHeat[noteIdx] += heatGain;
            if(semiHeat[noteIdx] > 10.f) semiHeat[noteIdx] = 10.f;
            if(octHeat[octIdx] + heatGain < 10.f) octHeat[octIdx] += heatGain;
            if(octHeat[octIdx] > 10.f) octHeat[octIdx] = 10.f;
        }

        if(resetTrigger.process(params[RESET_BUTTON].getValue())) {
            for(int i = 0; i < 12; i++) semiHeat[i] = 0.f;
            for(int i = 0; i < 10; i++) octHeat[i] = 0.f;
        }

        float warmestSemitone = 0.f, coolestSemitone = 0.f;
        float warmestOctave = 0.f, coolestOctave = 0.f;

        // Grab coolest & hottest semitone here
        for(int i = 0; i < 12; i++) {
            outputs[SEMITONE_OUTPUTS + 11 - i].setVoltage(semiHeat[i]);
        }

        // Grab coolest & hottest octave here
        for(int i = 0; i < 10; i++) {
            outputs[OCTAVE_OUTPUTS + 9 - i].setVoltage(octHeat[i]);
        }

        if(logDivider.process()) {
            //INFO("Racket Science: note %i octave %i", noteIdx, octIdx);
        }

        outputs[WARMEST_SEMI_OUTPUT].setVoltage(0);
        outputs[COOLEST_SEMI_OUTPUT].setVoltage(0);
        outputs[WARMEST_OCT_OUTPUT].setVoltage(0);
        outputs[COOLEST_OCT_OUTPUT].setVoltage(0);
    }

    void onReset() override {
        std::memset(semiHeat, 0, sizeof(semiHeat));
        std::memset(octHeat, 0, sizeof(octHeat));
    }

    json_t* dataToJson() override {
        json_t* rootJ = json_object();

        return rootJ;
    }

    void dataFromJson(json_t* rootJ) override {

    }
};

struct RSReheatWidget : ModuleWidget {
    RSReheat* module;
    Widget* panelBorder;

    RSReheatWidget(RSReheat *module) {
        setModule(module);
        this->module = module;

        // Is this key to fixing the problem with module dragging not always working first time? Taken from VCV blank panel
        panelBorder = new PanelBorder;
        addChild(panelBorder);

        box.size.x = mm2px(5.08 * 10);
        int middle = box.size.x / 2;
        int sixth = box.size.x / 6;

        RSTheme = loadDefaultTheme();

        addChild(new RSLabelCentered(middle, box.pos.y + 14, "REHEAT beta", 15));
        //addChild(new RSLabelCentered(middle, box.pos.y + 30, "Module Subtitle", 14));
        addChild(new RSLabelCentered(middle, box.size.y - 4, "Racket Science", 12)); // >= 4HP
        //addChild(new RSLabelCentered(middle, box.size.y - 15, "Racket", 12));
        //addChild(new RSLabelCentered(middle, box.size.y - 4, "Science", 12));

		addParam(createParamCentered<RSButtonMomentaryInvisible>(Vec(box.pos.x + 5, box.pos.y + 5), module, RSReheat::THEME_BUTTON));

        addInput(createInputCentered<RSJackMonoIn>(Vec(sixth, 30), module, RSReheat::CV_INPUT));
        addChild(new RSLabelCentered(sixth, 52, "V/OCT"));
        
        addInput(createInputCentered<RSJackMonoIn>(Vec(sixth, 70), module, RSReheat::GATE_INPUT));
        addChild(new RSLabelCentered(sixth, 92, "GATE"));

        addParam(createParamCentered<RSButtonMomentary>(Vec(sixth, 110), module, RSReheat::RESET_BUTTON));
        addChild(new RSLabelCentered(sixth, 132, "RESET"));

        addParam(createParamCentered<RSKnobSmlBlk>(Vec(sixth * 3, 33), module, RSReheat::GAIN_KNOB));
        addChild(new RSLabelCentered(sixth * 3, 57, "GAIN"));

        addParam(createParamCentered<RSKnobSmlBlk>(Vec(sixth * 5, 33), module, RSReheat::LOSS_KNOB));
        addChild(new RSLabelCentered(sixth * 5, 57, "LOSS"));

        addChild(new RSLabelCentered(sixth * 4, 76, "HOT"));
        addChild(new RSLabelCentered(sixth * 5, 76, "COLD"));

        addChild(new RSLabel(sixth * 2, 92, "SEMITONES"));
        addChild(new RSLabel(sixth * 2.35, 112, "OCTAVES"));

        addOutput(createOutputCentered<RSJackSmallMonoOut>(Vec(sixth * 4, 88), module, RSReheat::WARMEST_SEMI_OUTPUT));
        addOutput(createOutputCentered<RSJackSmallMonoOut>(Vec(sixth * 5, 88), module, RSReheat::COOLEST_SEMI_OUTPUT));
        
        addOutput(createOutputCentered<RSJackSmallMonoOut>(Vec(sixth * 4, 108), module, RSReheat::WARMEST_OCT_OUTPUT));
        addOutput(createOutputCentered<RSJackSmallMonoOut>(Vec(sixth * 5, 108), module, RSReheat::COOLEST_OCT_OUTPUT));
        

        LightWidget *lightWidget;

        // Semitone lights & outputs
        for(int i = 0 ; i < 12; i++) {
            int offset;
            switch(i) {
                case 1: case 3: case 5: case 8: case 10: offset = 7; break;
                default: offset = -7;
            }
            lightWidget = createLightCentered<LargeLight<BlueLight>>(Vec(sixth - offset, 144 + (i * 19)), module, RSReheat::SEMITONE_LIGHTS + i);
            lightWidget->bgColor = nvgRGBA(10, 10, 10, 128);
            addChild(lightWidget);

            addOutput(createOutputCentered<RSJackSmallMonoOut>(Vec(sixth * 3.75 - offset, 144 + (i * 19)), module, RSReheat::SEMITONE_OUTPUTS + i));
        }

        // Octave lights & outputs
        for(int i = 0; i < 10; i++) {
            lightWidget = createLightCentered<LargeLight<GreenLight>>(Vec(sixth * 2 + 7, 144 + (i * 23.25)), module, RSReheat::OCTAVE_LIGHTS + i);
            lightWidget->bgColor = nvgRGBA(10, 10, 10, 128);
            addChild(lightWidget);

            addOutput(createOutputCentered<RSJackSmallMonoOut>(Vec(sixth * 4.75 + 7, 144 + (i * 23.25)), module, RSReheat::OCTAVE_OUTPUTS + i));
        }


    }

    void step() override {
        if(!module) return;
        
        for(int i = 0; i < 12; i++) {
            module->lights[11 - i].setBrightness(module->semiHeat[i] / 10);
        }
        for(int i = 0; i < 10; i++) {
            module->lights[21 - i].setBrightness(module->octHeat[i] / 10);
        }

        for(int i = 0; i < 12; i++) {
            if(module->semiHeat[i] > 0.f) module->semiHeat[i] -= module->heatLoss;
            if(module->semiHeat[i] < 0.f) module->semiHeat[i] = 0.f;
        }
        for(int i = 0; i < 10; i++) {
            if(module->octHeat[i] > 0.f) module->octHeat[i] -= module->heatLoss;
            if(module->octHeat[i] < 0.f) module->octHeat[i] = 0.f;
        }

        ModuleWidget::step();
    }

    void customDraw(const DrawArgs& args) {}
	#include "RSModuleWidgetDraw.hpp"
};

Model *modelRSReheat = createModel<RSReheat, RSReheatWidget>("RSReheat");
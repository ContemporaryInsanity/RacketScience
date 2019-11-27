#include "plugin.hpp"

#include "components/RSComponents.hpp"
#include "RSUtils.hpp"

struct RSReheat : Module {
    enum ParamIds {
        THEME_BUTTON,
        RESET_BUTTON,
        NUM_PARAMS
    };
    enum InputIds {
        CV_INPUT,
        GATE_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
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
    float heatInc = 0.2f;
    float heatDec = 0.005f;

    RSReheat() {
        logDivider.setDivision(4096);

		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(THEME_BUTTON, 0.f, 1.f, 0.f, "THEME");
    }

    void process(const ProcessArgs &args) override {
        float cvIn = clamp10V(inputs[CV_INPUT].getVoltage());
        int noteIdx = note(cvIn);
        int octIdx = clamp(octave(cvIn) + 4, 0, 9);

        if(themeTrigger.process(params[THEME_BUTTON].getValue())) {
            RSTheme++;
            if(RSTheme > RSThemes) RSTheme = 0;
            saveDefaultTheme(RSTheme);
        }

        if(gateTrigger.process(inputs[GATE_INPUT].getVoltage())) {
            if(semiHeat[noteIdx] < 1.f) semiHeat[noteIdx] += heatInc;
            if(octHeat[octIdx] < 1.f) octHeat[octIdx] += heatInc;
        }

        if(resetTrigger.process(params[RESET_BUTTON].getValue())) {
            for(int i = 0; i < 12; i++) semiHeat[i] = 0.f;
            for(int i = 0; i < 10; i++) octHeat[i] = 0.f;
        }

        for(int i = 0; i < 12; i++) {
            outputs[RSReheat::SEMITONE_OUTPUTS + i].setVoltage(semiHeat[i] * 10.f);
        }

        for(int i = 0; i < 10; i++) {
            outputs[RSReheat::OCTAVE_OUTPUTS + i].setVoltage(octHeat[i] * 10.f);
        }

        if(logDivider.process()) {
            //INFO("Racket Science: note %i octave %i", noteIdx, octIdx);
        }
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

    RSReheatWidget(RSReheat *module) {
        setModule(module);
        this->module = module;

        box.size.x = mm2px(5.08 * 10);
        int middle = box.size.x / 2 + 1;
        int third = box.size.x / 3 + 1;
        int quarter = box.size.x / 4 + 1;
        int sixth = box.size.x / 6 + 1;
        int eighth = box.size.x / 8 + 1;

        RSTheme = loadDefaultTheme();

        addChild(new RSLabelCentered(middle, box.pos.y + 14, "REHEAT beta", 15));
        //addChild(new RSLabelCentered(middle, box.pos.y + 30, "Module Subtitle", 14));
        //addChild(new RSLabelCentered(middle, box.size.y - 4, "Racket Science", 12)); // >= 4HP
        addChild(new RSLabelCentered(middle, box.size.y - 15, "Racket", 12));
        addChild(new RSLabelCentered(middle, box.size.y - 4, "Science", 12));

		addParam(createParamCentered<RSButtonMomentaryInvisible>(Vec(box.pos.x + 5, box.pos.y + 5), module, RSReheat::THEME_BUTTON));

        addInput(createInputCentered<RSJackMonoIn>(Vec(middle / 2, 40), module, RSReheat::CV_INPUT));
        addChild(new RSLabelCentered(middle / 2, 65, "V/OCT"));
        
        addInput(createInputCentered<RSJackMonoIn>(Vec(middle + middle / 2, 40), module, RSReheat::GATE_INPUT));
        addChild(new RSLabelCentered(middle + middle / 2, 65, "GATE"));

        addParam(createParamCentered<RSButtonMomentary>(Vec(middle, 40), module, RSReheat::RESET_BUTTON));
        addChild(new RSLabelCentered(middle, 65, "RESET"));

        LightWidget *lightWidget;

        // Semitone lights & outputs
        for(int i = 0 ; i < 12; i++) {
            int offset;
            switch(i) {
                case 1: case 3: case 5: case 8: case 10: offset = 7; break;
                default: offset = -7;
            }
            lightWidget = createLightCentered<LargeLight<RedLight>>(Vec(sixth - offset, 120 + (i * 20)), module, RSReheat::SEMITONE_LIGHTS + i);
            lightWidget->bgColor = nvgRGBA(10, 10, 10, 128);
            addChild(lightWidget);

            addOutput(createOutputCentered<RSJackSmallMonoOut>(Vec(sixth * 4 - offset, 120 + (i * 20)), module, RSReheat::SEMITONE_OUTPUTS + i));
        }

        // Octave lights & outputs
        for(int i = 0; i < 10; i++) {
            lightWidget = createLightCentered<LargeLight<GreenLight>>(Vec(sixth * 2 + 7, 120 + (i * 24.5)), module, RSReheat::OCTAVE_LIGHTS + i);
            lightWidget->bgColor = nvgRGBA(10, 10, 10, 128);
            addChild(lightWidget);

            addOutput(createOutputCentered<RSJackSmallMonoOut>(Vec(sixth * 5 + 7, 120 + (i * 24.5)), module, RSReheat::OCTAVE_OUTPUTS + i));
        }


    }

    void step() override {
        if(!module) return;
        
        for(int i = 0; i < 12; i++) {
            module->lights[11 - i].setBrightness(module->semiHeat[i]);
        }
        for(int i = 0; i < 10; i++) {
            module->lights[21 - i].setBrightness(module->octHeat[i]);
        }

        for(int i = 0; i < 12; i++) {
            if(module->semiHeat[i] > 0.f) module->semiHeat[i] -= module->heatDec;
        }
        for(int i = 0; i < 10; i++) {
            if(module->octHeat[i] > 0.f) module->octHeat[i] -= module->heatDec;
        }

        ModuleWidget::step();
    }

    void customDraw(const DrawArgs& args) {}
	#include "RSModuleWidgetDraw.hpp"
};

Model *modelRSReheat = createModel<RSReheat, RSReheatWidget>("RSReheat");
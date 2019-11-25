#include "plugin.hpp"

#include "components/RSComponents.hpp"
#include "RSUtils.hpp"

struct RSHeat : Module {
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

    RSHeat() {
        logDivider.setDivision(4096);

		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(THEME_BUTTON, 0.f, 1.f, 0.f, "THEME");
    }

    void process(const ProcessArgs &args) override {
        float cvIn = inputs[CV_INPUT].getVoltage();
        int noteIdx = note(cvIn);
        int octIdx = octave(cvIn) + 4;

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

struct RSHeatWidget : ModuleWidget {
    RSHeat* module;

    RSHeatWidget(RSHeat *module) {
        setModule(module);
        this->module = module;

        box.size.x = mm2px(5.08 * 5);
        int middle = box.size.x / 2 + 1;
        int third = box.size.x / 3 + 1;

        RSTheme = loadDefaultTheme();

        addChild(new RSLabelCentered(middle, box.pos.y + 14, "HEAT", 15));
        //addChild(new RSLabelCentered(middle, box.pos.y + 30, "Module Subtitle", 14));
        //addChild(new RSLabelCentered(middle, box.size.y - 4, "Racket Science", 12)); // >= 4HP
        addChild(new RSLabelCentered(middle, box.size.y - 15, "Racket", 12));
        addChild(new RSLabelCentered(middle, box.size.y - 4, "Science", 12));

		addParam(createParamCentered<RSButtonMomentaryInvisible>(Vec(box.pos.x + 5, box.pos.y + 5), module, RSHeat::THEME_BUTTON));

        addInput(createInputCentered<RSJackMonoIn>(Vec(middle, 40), module, RSHeat::CV_INPUT));
        addChild(new RSLabelCentered(middle, 65, "V/OCT"));
        
        addInput(createInputCentered<RSJackMonoIn>(Vec(middle, 85), module, RSHeat::GATE_INPUT));
        addChild(new RSLabelCentered(middle, 110, "GATE"));

        addParam(createParamCentered<RSButtonMomentary>(Vec(middle, 130), module, RSHeat::RESET_BUTTON));
        addChild(new RSLabelCentered(middle, 155, "RESET"));

        LightWidget *lightWidget;

        // Semitone lights
        for(int i = 0 ; i < 12; i++) {
            int offset;
            switch(i) {
                case 1: case 3: case 5: case 8: case 10: offset = 7; break;
                default: offset = -7;
            }
            lightWidget = createLightCentered<LargeLight<RedLight>>(Vec(third - offset, 170 + (i * 16)), module, RSHeat::SEMITONE_LIGHTS + i);
            lightWidget->bgColor = nvgRGBA(10, 10, 10, 128);
            addChild(lightWidget);
        }

        // Octave lights
        for(int i = 0; i < 10; i++) {
            lightWidget = createLightCentered<LargeLight<GreenLight>>(Vec(third * 2 + 7, 172 + (i * 19)), module, RSHeat::OCTAVE_LIGHTS + i);
            lightWidget->bgColor = nvgRGBA(10, 10, 10, 128);
            addChild(lightWidget);
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

Model *modelRSHeat = createModel<RSHeat, RSHeatWidget>("RSHeat");
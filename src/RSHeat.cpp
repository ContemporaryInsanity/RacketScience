#include "plugin.hpp"

#include "RS.hpp"

struct RSHeat : RSModule {
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
        NUM_OUTPUTS
    };
    enum LightIds {
        ENUMS(SEMITONE_LIGHTS, 12),
        ENUMS(OCTAVE_LIGHTS, 10),
        NUM_LIGHTS
    };

    dsp::BooleanTrigger themeTrigger;

    dsp::SchmittTrigger gateTrigger;
    dsp::BooleanTrigger resetTrigger;

    float semiHeat[12] = {};
    float octHeat[10] = {};
    float heatGain = 0.1f;
    float heatLoss = 0.1f;

    RSHeat() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(THEME_BUTTON, 0.f, 1.f, 0.f, "THEME");

        configParam(GAIN_KNOB, 0.01f, 5.0f, 0.05f, "GAIN");
        configParam(LOSS_KNOB, 0.01f, 0.5f, 0.05f, "LOSS");
    }

    void process(const ProcessArgs &args) override {
		if(themeTrigger.process(params[THEME_BUTTON].getValue())) {
			RSTheme++;
			if(RSTheme > RSGlobal.themeCount) RSTheme = 1;
		}

        float vOctIn = RSclamp(inputs[CV_INPUT].getVoltage(), -10.f, 10.f);
        int noteIdx = note(vOctIn);
        int octIdx = clamp(octave(vOctIn) + 4, 0, 9);

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
    }

    void onReset() override {
        std::memset(semiHeat, 0, sizeof(semiHeat));
        std::memset(octHeat, 0, sizeof(octHeat));
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

struct RSHeatWidget : ModuleWidget {
    RSHeat* module;
    Widget* panelBorder;

    RSHeatWidget(RSHeat *module) {
        setModule(module);
        this->module = module;

        panelBorder = new PanelBorder;
        addChild(panelBorder);

        box.size.x = mm2px(5.08 * 5);
        int middle = box.size.x / 2 + 1;
        int third = box.size.x / 3;

		addParam(createParamCentered<RSButtonMomentaryInvisible>(Vec(box.pos.x + 5, box.pos.y + 5), module, RSHeat::THEME_BUTTON));

        addChild(new RSLabelCentered(middle, box.pos.y + 14, "HEAT", 15, module));
        //addChild(new RSLabelCentered(middle, box.pos.y + 30, "Module Subtitle", 14));
        addChild(new RSLabelCentered(middle, box.size.y - 4, "Racket Science", 12, module)); // >= 4HP
        //addChild(new RSLabelCentered(middle, box.size.y - 15, "Racket", 12));
        //addChild(new RSLabelCentered(middle, box.size.y - 4, "Science", 12));

        addInput(createInputCentered<RSJackMonoIn>(Vec(middle / 2, 30), module, RSHeat::CV_INPUT));
        addChild(new RSLabelCentered(middle / 2, 52, "V/OCT", 10, module));

        addInput(createInputCentered<RSJackMonoIn>(Vec(middle + middle / 2, 30), module, RSHeat::GATE_INPUT));
        addChild(new RSLabelCentered(middle + middle / 2, 52, "GATE", 10, module));

        addParam(createParamCentered<RSButtonMomentary>(Vec(middle, 68), module, RSHeat::RESET_BUTTON));
        addChild(new RSLabelCentered(middle, 71, "RESET", 10, module));

        addParam(createParamCentered<RSKnobSml>(Vec(middle / 2, 108), module, RSHeat::GAIN_KNOB));
        addChild(new RSLabelCentered(middle / 2, 132, "GAIN", 10, module));

        addParam(createParamCentered<RSKnobSml>(Vec(middle + middle / 2, 108), module, RSHeat::LOSS_KNOB));
        addChild(new RSLabelCentered(middle + middle / 2, 132, "LOSS", 10, module));

        LightWidget *lightWidget;

        // Semitone lights
        for(int i = 0 ; i < 12; i++) {
            int offset;
            switch(i) {
                case 1: case 3: case 5: case 8: case 10: offset = 7; break;
                default: offset = -7;
            }
            lightWidget = createLightCentered<LargeLight<BlueLight>>(Vec(third - offset, 144 + (i * 19)), module, RSHeat::SEMITONE_LIGHTS + i);
            lightWidget->bgColor = nvgRGBA(10, 10, 10, 128);
            addChild(lightWidget);
        }

        // Octave lights
        for(int i = 0; i < 10; i++) {
            lightWidget = createLightCentered<LargeLight<GreenLight>>(Vec(third * 2 + 7, 144 + (i * 23.25)), module, RSHeat::OCTAVE_LIGHTS + i);
            lightWidget->bgColor = nvgRGBA(10, 10, 10, 128);
            addChild(lightWidget);
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

Model *modelRSHeat = createModel<RSHeat, RSHeatWidget>("RSHeat");
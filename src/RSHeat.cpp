#include "plugin.hpp"

#include "components/RSComponents.hpp"
#include "RSUtils.hpp"

struct RSHeat : Module {
    enum ParamIds {
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
        ENUMS(LIGHTS, 12),
        NUM_LIGHTS
    };

    dsp::ClockDivider logDivider;
    dsp::ClockDivider lightDivider;
    dsp::ClockDivider fadeDivider;

    dsp::SchmittTrigger gateTrigger;
    dsp::BooleanTrigger resetTrigger;

    float heat[12] = {};
    float heatInc = 0.1f;
    float heatDec = 0.01f;

    RSHeat() {
        logDivider.setDivision(16384);
        lightDivider.setDivision(1024);
        fadeDivider.setDivision(8192);

		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }

    void process(const ProcessArgs &args) override {
        float cvIn = inputs[CV_INPUT].getVoltage();
        //float octave;
        //float note = std::modf(cvIn, &octave); octave -= 10.f; cvIn -= 10.f;

        int noteIdx = (int(round((cvIn + 10.f) * 12)) % 12);

        if(gateTrigger.process(inputs[GATE_INPUT].getVoltage())) {
            if(heat[noteIdx] < 1.f) heat[noteIdx] += heatInc;
        }

        if(resetTrigger.process(params[RESET_BUTTON].getValue())) {
            for(int i = 0; i < 12; i++) heat[i] = 0.f;
        }

        if(logDivider.process()) {
            //INFO("Racket Science: note %i", noteIdx;
        }

        if(lightDivider.process()) {
            for(int i = 0; i < 12; i++) {
                lights[11 - i].setBrightness(heat[i]);
            }
        }

        if(fadeDivider.process()) {
            for(int i = 0; i < 12; i++) {
                if(heat[i] > 0.f) heat[i] -= heatDec;
            }
        }
    }

    void onReset() override {
        std::memset(heat, 0, sizeof(heat));
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

    int theme = 0;

    RSHeatWidget(RSHeat *module) {
        setModule(module);
        this->module = module;

        box.size.x = mm2px(5.08 * 3);
        int middle = box.size.x / 2;

        theme = loadDefaultTheme();

        addChild(new RSLabelCentered(middle, box.pos.y + 14, "HEAT", 15));
        //addChild(new RSLabelCentered(middle, box.pos.y + 30, "Module Subtitle", 14));
        //addChild(new RSLabelCentered(middle, box.size.y - 4, "Racket Science", 12)); // >= 4HP
        addChild(new RSLabelCentered(middle, box.size.y - 15, "Racket", 12));
        addChild(new RSLabelCentered(middle, box.size.y - 4, "Science", 12));


        addInput(createInputCentered<RSJackMonoIn>(Vec(middle, 40), module, RSHeat::CV_INPUT));
        addChild(new RSLabelCentered(middle, 65, "V/OCT"));
        
        addInput(createInputCentered<RSJackMonoIn>(Vec(middle, 85), module, RSHeat::GATE_INPUT));
        addChild(new RSLabelCentered(middle, 110, "GATE"));

        addParam(createParamCentered<RSButtonMomentary>(Vec(middle, 130), module, RSHeat::RESET_BUTTON));
        addChild(new RSLabelCentered(middle, 155, "RESET"));

        LightWidget *lightWidget;
        for(int i = 0 ; i < 12; i++) {
            int offset;
            switch(i) {
                case 1: case 3: case 5: case 8: case 10: offset = 7; break;
                default: offset = -7;
            }
            lightWidget = createLightCentered<LargeLight<RedLight>>(Vec(middle - offset, 170 + (i * 16)), module, RSHeat::LIGHTS + i);
            lightWidget->bgColor = nvgRGBA(50, 50, 50, 128);
            addChild(lightWidget);
        }
    }

    void draw(const DrawArgs& args) override {
		nvgStrokeColor(args.vg, COLOR_RS_BRONZE);
		switch(theme) {
            case 0: nvgFillColor(args.vg, COLOR_RS_BG); break;
            case 1: nvgFillColor(args.vg, nvgRGB(0x60, 0x60, 0x00)); break;
            default: nvgFillColor(args.vg, COLOR_BLACK); break;
        }
		nvgStrokeWidth(args.vg, 3);

		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 5);
		//nvgStroke(args.vg);
		nvgFill(args.vg);

        ModuleWidget::draw(args);
    }
};

Model *modelRSHeat = createModel<RSHeat, RSHeatWidget>("RSHeat");
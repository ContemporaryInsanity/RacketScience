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
        ENUMS(SEMITONE_LIGHTS, 12),
        ENUMS(OCTAVE_LIGHTS, 10),
        NUM_LIGHTS
    };

    dsp::ClockDivider logDivider;
    dsp::ClockDivider lightDivider;
    dsp::ClockDivider fadeDivider;

    dsp::SchmittTrigger gateTrigger;
    dsp::BooleanTrigger resetTrigger;

    float semiHeat[12] = {};
    float octHeat[10] = {};
    float heatInc = 0.1f;
    float heatDec = 0.01f;

    RSHeat() {
        logDivider.setDivision(4096);
        lightDivider.setDivision(1024);
        fadeDivider.setDivision(8192);

		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }

    void process(const ProcessArgs &args) override {

        // This DOESN'T need to run at anywhere near audio rate

        float cvIn = inputs[CV_INPUT].getVoltage();
        int noteIdx = note(cvIn); //(int(round((cvIn + 10) * 12)) % 12);
        int octIdx = octave(cvIn) + 5;

        if(gateTrigger.process(inputs[GATE_INPUT].getVoltage())) {
            if(semiHeat[noteIdx] < 1.f) semiHeat[noteIdx] += heatInc;
            if(octHeat[octIdx] < 1.f) octHeat[octIdx] += heatInc;
        }

        if(resetTrigger.process(params[RESET_BUTTON].getValue())) {
            for(int i = 0; i < 12; i++) semiHeat[i] = 0.f;
            for(int i = 0; i < 10; i++) octHeat[i] = 0.f;
        }

        if(logDivider.process()) {
            INFO("Racket Science: note %i octave %i", noteIdx, octIdx);
        }

        if(lightDivider.process()) {
            for(int i = 0; i < 12; i++) {
                lights[11 - i].setBrightness(semiHeat[i]);
            }
            for(int i = 0; i < 10; i++) {
                lights[21 - i].setBrightness(octHeat[i]);
            }
        }

        if(fadeDivider.process()) {
            for(int i = 0; i < 12; i++) {
                if(semiHeat[i] > 0.f) semiHeat[i] -= heatDec;
            }
            for(int i = 0; i < 10; i++) {
                if(octHeat[i] > 0.f) octHeat[i] -= heatDec;
            }
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

    int theme = 0;

    RSHeatWidget(RSHeat *module) {
        setModule(module);
        this->module = module;

        box.size.x = mm2px(5.08 * 5);
        int middle = box.size.x / 2 + 1;
        int third = box.size.x / 3 + 1;

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

    void customDraw(const DrawArgs& args) {}
	#include "RSModuleWidgetDraw.hpp"
};

Model *modelRSHeat = createModel<RSHeat, RSHeatWidget>("RSHeat");
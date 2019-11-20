#include "plugin.hpp"

#include "components/RSComponents.hpp"

struct RSHeat : Module {
    enum ParamIds {
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

    dsp::SchmittTrigger gateTrigger;

    float heat[12] = {};
    float heatInc = 0.01f;

    RSHeat() {
        logDivider.setDivision(16384);
        lightDivider.setDivision(1024);

		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }

    void process(const ProcessArgs &args) override {

        float cvIn = inputs[CV_INPUT].getVoltage() + 10.f;
        float octave;
        float note = std::modf(cvIn, &octave); octave -= 10.f; cvIn -= 10.f;

        int noteIdx = (note + 0.08f) * 12; // Bodge but will do for prototyping purposes

        if(gateTrigger.process(inputs[GATE_INPUT].getVoltage())) {
            heat[noteIdx] += heatInc;
        }

        // Have a divider to slowly decrease the heat levels, so if gates stop everything slowly dims

        // Would be nice to have a colour scale from green through red to yellow to more easily see more frequent notes
        

        if(logDivider.process()) {
            INFO("Racket Science: cvIn=%f octave=%f note=%f noteIdx=%i", cvIn, octave, note, noteIdx);
        }

        if(lightDivider.process()) {
            for(int i = 0; i < 12; i++) {
                lights[11 - i].setBrightness(heat[i]);
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

    //std::shared_ptr<Font> font;

    RSHeatWidget(RSHeat *module) {
        setModule(module);
        this->module = module;

        box.size.x = 104;

        addChild(new RSLabelCentered(box.size.x / 2, box.pos.y + 16, "HEAT", 16));
        //addChild(new RSLabelCentered(box.size.x / 2, box.pos.y + 30, "Module Subtitle", 14));
        addChild(new RSLabelCentered(box.size.x / 2, box.size.y - 6, "Racket Science", 12));

        addInput(createInputCentered<RSJackMonoIn>(Vec(52, 50), module, RSHeat::CV_INPUT));
        addInput(createInputCentered<RSJackMonoIn>(Vec(52, 90), module, RSHeat::GATE_INPUT));

        LightWidget *lightWidget;
        for(int i = 0 ; i < 12; i++) {
            int offset;
            switch(i) {
                case 1: case 3: case 5: case 8: case 10: offset = 10; break;
                default: offset = 0;
            }
            lightWidget = createLightCentered<LargeLight<RedLight>>(Vec(57 - offset, 150 + (i * 17)), module, RSHeat::LIGHTS + i);
            lightWidget->bgColor = nvgRGBA(50, 50, 50, 128);
            addChild(lightWidget);
        }

    }

    void draw(const DrawArgs& args) override {
		nvgStrokeColor(args.vg, COLOR_RS_BRONZE);
		nvgFillColor(args.vg, COLOR_BLACK);
		nvgStrokeWidth(args.vg, 1.5);

		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 5);
		nvgStroke(args.vg);
		nvgFill(args.vg);

        ModuleWidget::draw(args);
    }
};

Model *modelRSHeat = createModel<RSHeat, RSHeatWidget>("RSHeat");
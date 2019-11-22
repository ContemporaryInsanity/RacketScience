#include "plugin.hpp"

#include "components/RSComponents.hpp"

struct RSSkeleton : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        NUM_INPUTS
    };
    enum OutputIds {
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    RSSkeleton() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }

    void process(const ProcessArgs &args) override {

    }

    json_t* dataToJson() override {
        json_t* rootJ = json_object();

        return rootJ;
    }

    void dataFromJson(json_t* rootJ) override {

    }
};

struct RSSkeletonWidget : ModuleWidget {
    RSSkeleton* module;

    //std::shared_ptr<Font> font;

    RSSkeletonWidget(RSSkeleton *module) {
        setModule(module);
        this->module = module;

        box.size.x = 104;

        addChild(new RSLabelCentered(box.size.x / 2, box.pos.y + 16, "MODULE TITLE", 16));
        addChild(new RSLabelCentered(box.size.x / 2, box.pos.y + 30, "Module Subtitle", 14));
        addChild(new RSLabelCentered(box.size.x / 2, box.size.y - 6, "Racket Science", 12));

    }

    void draw(const DrawArgs& args) override {
		nvgStrokeColor(args.vg, COLOR_RS_BRONZE);
		nvgFillColor(args.vg, COLOR_BLACK);
		nvgStrokeWidth(args.vg, 1.5);

		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 5);
		//nvgStroke(args.vg);
		nvgFill(args.vg);

        ModuleWidget::draw(args);
    }
};

Model *modelRSSkeleton = createModel<RSSkeleton, RSSkeletonWidget>("RSSkeleton");
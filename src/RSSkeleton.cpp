#include "plugin.hpp"

#include "components/RSComponents.hpp"
#include "RSUtils.hpp"

struct RSSkeleton : Module {
	enum ParamIds {
		THEME_BUTTON,
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

	dsp::BooleanTrigger themeTrigger;

	RSSkeleton() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(THEME_BUTTON, 0.f, 1.f, 0.f, "THEME");
	}

	void process(const ProcessArgs &args) override {
		if(themeTrigger.process(params[THEME_BUTTON].getValue())) {
			RSTheme++;
			if(RSTheme > RSThemes) RSTheme = 0;
			saveDefaultTheme(RSTheme);
		}

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

	RSSkeletonWidget(RSSkeleton *module) {
		setModule(module);
		this->module = module;

		box.size.x = mm2px(5.08 * 3);
		int middle = box.size.x / 2 + 1;

		RSTheme = loadDefaultTheme();

		addChild(new RSLabelCentered(middle, box.pos.y + 13, "TITLE1", 14));
		addChild(new RSLabelCentered(middle, box.pos.y + 25, "TITLE2", 14));
		addChild(new RSLabelCentered(middle, box.pos.y + 37, "TITLE3", 14));

		addChild(new RSLabelCentered(middle, box.size.y - 15, "Racket", 12));
		addChild(new RSLabelCentered(middle, box.size.y - 4, "Science", 12));

		addParam(createParamCentered<RSButtonMomentaryInvisible>(Vec(box.pos.x + 5, box.pos.y + 5), module, RSSkeleton::THEME_BUTTON));

	}

    void customDraw(const DrawArgs& args) {}
	#include "RSModuleWidgetDraw.hpp"

	void step() override {
		if(!module) return;

		ModuleWidget::step();
	}
};

Model *modelRSSkeleton = createModel<RSSkeleton, RSSkeletonWidget>("RSSkeleton");
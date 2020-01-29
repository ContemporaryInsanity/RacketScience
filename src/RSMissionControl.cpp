#include "plugin.hpp"

#include "RS.hpp"

struct RSMissionControl : RSModule {
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

	RSMissionControl() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(THEME_BUTTON, 0.f, 1.f, 0.f, "THEME");

	}

	void process(const ProcessArgs &args) override {
		if(themeTrigger.process(params[THEME_BUTTON].getValue())) {
			RSTheme++;
			if(RSTheme > RSGlobal.themeCount) RSTheme = 1;
		}

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


struct RSMissionControlWidget : ModuleWidget {
	RSMissionControl* module;

	RSMissionControlWidget(RSMissionControl *module) {
		INFO("Racket Science: RSMissionControlWidget()");

		setModule(module);
		this->module = module;

		box.size = Vec(RACK_GRID_WIDTH * 30, RACK_GRID_HEIGHT);
		int middle = box.size.x / 2 + 1;

		addParam(createParamCentered<RSButtonMomentaryInvisible>(Vec(box.pos.x + 5, box.pos.y + 5), module, RSMissionControl::THEME_BUTTON));

		addChild(new RSLabelCentered(middle, box.pos.y + 13, "MISSION CONTROL", 14, module));
		addChild(new RSLabelCentered(middle, box.size.y - 4, "Racket Science", 12, module));

	}

	void step() override {
		if(!module) return;

		ModuleWidget::step();
	}

    void customDraw(const DrawArgs& args) {}
	#include "RSModuleWidgetDraw.hpp"
};

Model *modelRSMissionControl = createModel<RSMissionControl, RSMissionControlWidget>("RSMissionControl");
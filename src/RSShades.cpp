#include "plugin.hpp"

#include "RS.hpp"

struct RSShades : RSModule {
	enum ParamIds {
		THEME_BUTTON,
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(HUE_IN, 12),
		ENUMS(SAT_IN, 12),
		ENUMS(LUM_IN, 12),
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	dsp::BooleanTrigger themeTrigger;

	RSShades() {
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


struct RSShadesWidget : ModuleWidget {
	RSShades* module;

	RSShadesWidget(RSShades *module) {
		setModule(module);
		this->module = module;

		box.size = Vec(RACK_GRID_WIDTH * 10, RACK_GRID_HEIGHT);
		int middle = box.size.x / 2 + 1;

		addParam(createParamCentered<RSButtonMomentaryInvisible>(Vec(box.pos.x + 5, box.pos.y + 5), module, RSShades::THEME_BUTTON));

		addChild(new RSLabelCentered(middle, box.pos.y + 13, "SHADES", 14, module));

		addChild(new RSLabelCentered( 40, 25, "HUE", 10, module));
		addChild(new RSLabelCentered( 80, 25, "SAT", 10, module));
		addChild(new RSLabelCentered(120, 25, "LUM", 10, module));

		for(int i = 0; i < 12; i++) {
			int offset;
			switch(i) {
				case 1: case 3: case 5: case 8: case 10: offset = 7; break;
				default: offset = -7;
			}
			addChild(new RSLabelCentered(12, 46 + (i * 28), std::to_string(12 - i).c_str(), 10, module));
			addInput(createInputCentered<RSJackMonoIn>(Vec( 40 - offset, 42 + (i * 28)), module, RSShades::HUE_IN + i));
			addInput(createInputCentered<RSJackMonoIn>(Vec( 80 - offset, 42 + (i * 28)), module, RSShades::SAT_IN + i));
			addInput(createInputCentered<RSJackMonoIn>(Vec(120 - offset, 42 + (i * 28)), module, RSShades::LUM_IN + i));
		}

		addChild(new RSLabelCentered(middle, box.size.y - 4, "Racket Science", 12, module));
	}

	void step() override {
		if(!module) return;

		for(int i = 0; i < 12; i++) {
			if(module->inputs[RSShades::HUE_IN_LAST - i].isConnected()) {
				RSGlobal.themes[i + 1].bghsl.hue = RSclamp(module->inputs[RSShades::HUE_IN_LAST - i].getVoltage(), 0.f, 10.f) / 10.f;
			    RSGlobal.themes[i + 1].bgColor = nvgHSL(RSGlobal.themes[i + 1].bghsl.hue,
    			                                        RSGlobal.themes[i + 1].bghsl.sat,
                			                            RSGlobal.themes[i + 1].bghsl.lum);
			}
			if(module->inputs[RSShades::SAT_IN_LAST - i].isConnected()) {
				RSGlobal.themes[i + 1].bghsl.sat = RSclamp(module->inputs[RSShades::SAT_IN_LAST - i].getVoltage(), 0.f, 10.f) / 10.f;
			    RSGlobal.themes[i + 1].bgColor = nvgHSL(RSGlobal.themes[i + 1].bghsl.hue,
    			                                        RSGlobal.themes[i + 1].bghsl.sat,
                			                            RSGlobal.themes[i + 1].bghsl.lum);
			}
			if(module->inputs[RSShades::LUM_IN_LAST - i].isConnected()) {
				RSGlobal.themes[i + 1].bghsl.lum = RSclamp(module->inputs[RSShades::LUM_IN_LAST - i].getVoltage(), 0.f, 10.f) / 10.f;
			    RSGlobal.themes[i + 1].bgColor = nvgHSL(RSGlobal.themes[i + 1].bghsl.hue,
    			                                        RSGlobal.themes[i + 1].bghsl.sat,
                			                            RSGlobal.themes[i + 1].bghsl.lum);
			}
		}

		ModuleWidget::step();
	}

    void customDraw(const DrawArgs& args) {}
	#include "RSModuleWidgetDraw.hpp"
};

Model *modelRSShades = createModel<RSShades, RSShadesWidget>("RSShades");
#include "plugin.hpp"

#include "components/RSComponents.hpp"
#include "RSUtils.hpp"

struct RSGroundControl : Module {
	enum ParamIds {
		BGHUE_KNOB, BGSAT_KNOB, BGLUM_KNOB,
		LBHUE_KNOB, LBSAT_KNOB, LBLUM_KNOB,
		SSHUE_KNOB, SSSAT_KNOB, SSLUM_KNOB,
		LEDA_KNOB, LEDB_KNOB,
		THEME_KNOB,
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

	RSGroundControl() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configParam(BGHUE_KNOB, 0.f, 1.f, 0.5f, "HUE");
		configParam(BGSAT_KNOB, 0.f, 1.f, 0.5f, "SAT");
		configParam(BGLUM_KNOB, 0.f, 1.f, 0.5f, "LUM");
		configParam(LBHUE_KNOB, 0.f, 1.f, 0.5f, "HUE");
		configParam(LBSAT_KNOB, 0.f, 1.f, 0.5f, "SAT");
		configParam(LBLUM_KNOB, 0.f, 1.f, 0.5f, "LUM");
		configParam(SSHUE_KNOB, 0.f, 1.f, 0.5f, "HUE");
		configParam(SSSAT_KNOB, 0.f, 1.f, 0.5f, "SAT");
		configParam(SSLUM_KNOB, 0.f, 1.f, 0.5f, "LUM");
		configParam(LEDA_KNOB,  0.f, 1.f, 0.5f, "HUE");
		configParam(LEDB_KNOB,  0.f, 1.f, 0.5f, "HUE");
		configParam(THEME_KNOB, 0.f, 3.f, 0.0f, "THEME");
	}

	void process(const ProcessArgs &args) override {
	}

	void updateParams() {
		params[RSGroundControl::BGHUE_KNOB].setValue(RSGlobal.themes[RSGlobal.themeIdx].bgColor.hue);
		params[RSGroundControl::BGSAT_KNOB].setValue(RSGlobal.themes[RSGlobal.themeIdx].bgColor.sat);
		params[RSGroundControl::BGLUM_KNOB].setValue(RSGlobal.themes[RSGlobal.themeIdx].bgColor.lum);
		params[RSGroundControl::LBHUE_KNOB].setValue(RSGlobal.themes[RSGlobal.themeIdx].lbColor.hue);
		params[RSGroundControl::LBSAT_KNOB].setValue(RSGlobal.themes[RSGlobal.themeIdx].lbColor.sat);
		params[RSGroundControl::LBLUM_KNOB].setValue(RSGlobal.themes[RSGlobal.themeIdx].lbColor.lum);
		params[RSGroundControl::SSHUE_KNOB].setValue(RSGlobal.themes[RSGlobal.themeIdx].ssColor.hue);
		params[RSGroundControl::SSSAT_KNOB].setValue(RSGlobal.themes[RSGlobal.themeIdx].ssColor.sat);
		params[RSGroundControl::SSLUM_KNOB].setValue(RSGlobal.themes[RSGlobal.themeIdx].ssColor.lum);
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();

		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {

	}
};


struct RSGroundControlWidget : ModuleWidget {
	RSGroundControl* module;

	RSGroundControlWidget(RSGroundControl *module) {
		setModule(module);
		this->module = module;

		box.size.x = mm2px(5.08 * 10);
		int middle = box.size.x / 2 + 1;
		int quarter = middle / 2;

		addChild(new RSLabelCentered(middle, box.pos.y + 13, "GROUND CONTROL", 14));
		addChild(new RSLabelCentered(middle, box.size.y - 4, "Racket Science", 12));

		/*  What else to include?
			Current date & time
			Elapsed time since start
			General purpose CV controllable timer
			Countdown alarm
			General alarm

			Have a panel behind each set of HSL knobs to show color selected

		*/

		{
			int top = 190, left = 40;
			int xsp = 30, ysp = 30, los = 30; // x spacing, y spacing, label offset

			addChild(new RSLabelCentered(middle, top, "ANY COLOUR YOU LIKE", 12));

			addChild(new RSLabelCentered(left + (xsp * 1), top + (ysp / 2), "HUE"));
			addChild(new RSLabelCentered(left + (xsp * 2), top + (ysp / 2), "SAT"));
			addChild(new RSLabelCentered(left + (xsp * 3), top + (ysp / 2), "LUM"));

			addChild(new RSLabelCentered(left - 10, top + (ysp * 1) + 3, "BACKGROUND"));
			addChild(new RSLabelCentered(left - 10, top + (ysp * 2) + 3, "LABELS"));
			addChild(new RSLabelCentered(left - 10, top + (ysp * 3) + 3, "SCRIBBLES"));
			addChild(new RSLabelCentered(left - 10, top + (ysp * 4) + 3, "LEDS A"));
			addChild(new RSLabelCentered(left - 10, top + (ysp * 5) + 3, "LEDS B"));

			addParam(createParamCentered<RSKnobSml>(Vec(left + (xsp * 1), top + (ysp * 1)), module, RSGroundControl::BGHUE_KNOB));
			addParam(createParamCentered<RSKnobSml>(Vec(left + (xsp * 2), top + (ysp * 1)), module, RSGroundControl::BGSAT_KNOB));
			addParam(createParamCentered<RSKnobSml>(Vec(left + (xsp * 3), top + (ysp * 1)), module, RSGroundControl::BGLUM_KNOB));

			addParam(createParamCentered<RSKnobSml>(Vec(left + (xsp * 1), top + (ysp * 2)), module, RSGroundControl::LBHUE_KNOB));
			addParam(createParamCentered<RSKnobSml>(Vec(left + (xsp * 2), top + (ysp * 2)), module, RSGroundControl::LBSAT_KNOB));
			addParam(createParamCentered<RSKnobSml>(Vec(left + (xsp * 3), top + (ysp * 2)), module, RSGroundControl::LBLUM_KNOB));

			addParam(createParamCentered<RSKnobSml>(Vec(left + (xsp * 1), top + (ysp * 3)), module, RSGroundControl::SSHUE_KNOB));
			addParam(createParamCentered<RSKnobSml>(Vec(left + (xsp * 2), top + (ysp * 3)), module, RSGroundControl::SSSAT_KNOB));
			addParam(createParamCentered<RSKnobSml>(Vec(left + (xsp * 3), top + (ysp * 3)), module, RSGroundControl::SSLUM_KNOB));

			addParam(createParamCentered<RSKnobSml>(Vec(left + (xsp * 1), top + (ysp * 4)), module, RSGroundControl::LEDA_KNOB));
			addParam(createParamCentered<RSKnobSml>(Vec(left + (xsp * 1), top + (ysp * 5)), module, RSGroundControl::LEDB_KNOB));

			addParam(createParamCentered<RSKnobDetentMed>(Vec(left + (xsp * 2.5), top + (ysp * 4.5)), module, RSGroundControl::THEME_KNOB));
			addChild(new RSLabelCentered(left + (xsp * 2.5), top + (ysp * 5.8), "THEME"));


		}

		if(!module) return;

		module->params[RSGroundControl::THEME_KNOB].setValue(RSGlobal.themeIdx);

		module->updateParams();
	}

    void customDraw(const DrawArgs& args) {}
	#include "RSModuleWidgetDraw.hpp"

	void step() override {
		if(!module) return;

		static struct rsglobal lastRSGlobal;
		
		static int lastTheme = 0;
		int theme = (int)module->params[RSGroundControl::THEME_KNOB].getValue();		

		if(theme != lastTheme) { // Them has changed
			RSGlobal.themeIdx = lastTheme = theme;
			module->updateParams();
			updateRSTheme();
			saveRSGlobal();
		}

		RSGlobal.themes[RSGlobal.themeIdx].bgColor.hue = module->params[RSGroundControl::BGHUE_KNOB].getValue();
		RSGlobal.themes[RSGlobal.themeIdx].bgColor.sat = module->params[RSGroundControl::BGSAT_KNOB].getValue();
		RSGlobal.themes[RSGlobal.themeIdx].bgColor.lum = module->params[RSGroundControl::BGLUM_KNOB].getValue();

		RSGlobal.themes[RSGlobal.themeIdx].lbColor.hue = module->params[RSGroundControl::LBHUE_KNOB].getValue();
		RSGlobal.themes[RSGlobal.themeIdx].lbColor.sat = module->params[RSGroundControl::LBSAT_KNOB].getValue();
		RSGlobal.themes[RSGlobal.themeIdx].lbColor.lum = module->params[RSGroundControl::LBLUM_KNOB].getValue();

		RSGlobal.themes[RSGlobal.themeIdx].ssColor.hue = module->params[RSGroundControl::SSHUE_KNOB].getValue();
		RSGlobal.themes[RSGlobal.themeIdx].ssColor.sat = module->params[RSGroundControl::SSSAT_KNOB].getValue();
		RSGlobal.themes[RSGlobal.themeIdx].ssColor.lum = module->params[RSGroundControl::SSLUM_KNOB].getValue();

		updateRSTheme();

		// memcmp should suffice, nothing unusual in rsglobal struct
		if(memcmp(&RSGlobal, &lastRSGlobal, sizeof(rsglobal)) != 0) { // Theme settings have changed
			lastRSGlobal = RSGlobal;
			saveRSGlobal();
		}

		ModuleWidget::step();
	}
};

Model *modelRSGroundControl = createModel<RSGroundControl, RSGroundControlWidget>("RSGroundControl");
#include "plugin.hpp"

#include "RS.hpp"

static bool owned = false;

struct RSGroundControl : RSModule {
	bool running = false;

	enum ParamIds {
		BGHUE_KNOB, BGSAT_KNOB, BGLUM_KNOB,
		LBHUE_KNOB, LBSAT_KNOB, LBLUM_KNOB,
		SSHUE_KNOB, SSSAT_KNOB, SSLUM_KNOB,
		LEDA_KNOB, LEDB_KNOB,
		THEME_KNOB,
		NUM_PARAMS
	};
	enum InputIds {
		STEALTH_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	RSGroundControl() {
		if(!owned) {
			owned = true;
			running = true;
		}

		RSTheme = 0;

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

		configParam(THEME_KNOB, 0.f, RSGlobal.themeCount - 1.f, 0.0f, "THEME");
	}

	void process(const ProcessArgs &args) override {
	}

	void updateParams() {
		if(!running) return;

		params[RSGroundControl::BGHUE_KNOB].setValue(RSGlobal.themes[RSGlobal.themeIdx].bghsl.hue);
		params[RSGroundControl::BGSAT_KNOB].setValue(RSGlobal.themes[RSGlobal.themeIdx].bghsl.sat);
		params[RSGroundControl::BGLUM_KNOB].setValue(RSGlobal.themes[RSGlobal.themeIdx].bghsl.lum);
		params[RSGroundControl::LBHUE_KNOB].setValue(RSGlobal.themes[RSGlobal.themeIdx].lbhsl.hue);
		params[RSGroundControl::LBSAT_KNOB].setValue(RSGlobal.themes[RSGlobal.themeIdx].lbhsl.sat);
		params[RSGroundControl::LBLUM_KNOB].setValue(RSGlobal.themes[RSGlobal.themeIdx].lbhsl.lum);
		params[RSGroundControl::SSHUE_KNOB].setValue(RSGlobal.themes[RSGlobal.themeIdx].sshsl.hue);
		params[RSGroundControl::SSSAT_KNOB].setValue(RSGlobal.themes[RSGlobal.themeIdx].sshsl.sat);
		params[RSGroundControl::SSLUM_KNOB].setValue(RSGlobal.themes[RSGlobal.themeIdx].sshsl.lum);
		params[RSGroundControl::LEDA_KNOB].setValue(RSGlobal.themes[RSGlobal.themeIdx].ledAh);
		params[RSGroundControl::LEDB_KNOB].setValue(RSGlobal.themes[RSGlobal.themeIdx].ledBh);
	}

	void onReset() override {
		// How about just removing the RacketScience settings dir & calling SaveRSGlobal?
        float hue = 0.f;
        float hueStep = 1.f / RSGlobal.themeCount;
		// Amend this to 1-13, customise 0, 14 & 15
        for(int i = 0; i < RSGlobal.themeCount; i++, hue += hueStep) {
            RSGlobal.themes[i].bghsl = {hue, .6f, .5f};
            RSGlobal.themes[i].lbhsl = {hue, .8f, .9f};
            RSGlobal.themes[i].sshsl = {hue, .7f, .8f};
            // LEDs here too once complete
            updateRSTheme(i);
        }

        RSGlobal.themeIdx = 0;
		updateParams();
        saveRSGlobal();
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();

		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {

	}

	~RSGroundControl() {
		if(running) {
			owned = false;
		}
	}
};

// Move to RSComponents.hpp once perfected
struct RSLedAWidget : TransparentWidget {
	NVGcolor bgColor = nvgRGBA(0, 0, 0, 0);
	NVGcolor color = nvgRGBA(0, 0, 0, 0);
	NVGcolor borderColor = nvgRGBA(0, 0, 0, 0);

	RSLedAWidget(int x, int y, int size = 15) {
		box.pos = Vec(x, y);
		box.size = Vec(size, size);
	}

	void draw(const DrawArgs& args) override {
		drawLed(args);
		drawHalo(args);
	}

	void drawLed(const DrawArgs& args) {
		nvgStrokeColor(args.vg, COLOR_RS_BRONZE);
		color = RSGlobal.themes[RSGlobal.themeIdx].lAColor;
		nvgFillColor(args.vg, color);
		nvgBeginPath(args.vg);
		// nvgRoundedRect(args.vg, box.pos.x, box.pos.y, box.size.x, box.size.y, 10);
		nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 10);
		nvgStroke(args.vg);
		nvgFill(args.vg);
	}

	void drawHalo(const DrawArgs& args) {
		float radius = box.size.x / 2;
		float oradius = radius * 4.f;

		nvgBeginPath(args.vg);
		nvgRect(args.vg, box.pos.x, box.pos.y, box.size.x, box.size.y);

		NVGpaint paint;
		NVGcolor icol = color::mult(color, 0.7f);
		NVGcolor ocol = nvgRGB(0, 0, 0);
		paint = nvgRadialGradient(args.vg, radius, radius, radius, oradius, icol, ocol);
		nvgFillPaint(args.vg, paint);
		nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
		nvgFill(args.vg);
	}
};

struct RSLedBWidget : TransparentWidget {
	NVGcolor bgColor = nvgRGBA(0, 0, 0, 0);
	NVGcolor color = nvgRGBA(0, 0, 0, 0);
	NVGcolor borderColor = nvgRGBA(0, 0, 0, 0);

	RSLedBWidget(int x, int y, int size = 15) {
		box.pos = Vec(x, y);
		box.size = Vec(size, size);
	}

	void draw(const DrawArgs& args) override {
		drawLed(args);
		drawHalo(args);
	}

	void drawLed(const DrawArgs& args) {
		nvgStrokeColor(args.vg, COLOR_RS_BRONZE);
		color = RSGlobal.themes[RSGlobal.themeIdx].lBColor;
		nvgFillColor(args.vg, color);
		nvgBeginPath(args.vg);
		// nvgRoundedRect(args.vg, box.pos.x, box.pos.y, box.size.x, box.size.y, 10);
		nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 10);
		nvgStroke(args.vg);
		nvgFill(args.vg);
	}

	void drawHalo(const DrawArgs& args) {
		float radius = box.size.x / 2;
		float oradius = radius * 4.f;

		nvgBeginPath(args.vg);
		nvgRect(args.vg, box.pos.x, box.pos.y, box.size.x, box.size.y);

		NVGpaint paint;
		NVGcolor icol = color::mult(color, 0.7f);
		NVGcolor ocol = nvgRGB(0, 0, 0);
		paint = nvgRadialGradient(args.vg, radius, radius, radius, oradius, icol, ocol);
		nvgFillPaint(args.vg, paint);
		nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
		nvgFill(args.vg);
	}
};

struct RSGroundControlWidget : ModuleWidget {
	RSGroundControl* module;

	RSGroundControlWidget(RSGroundControl *module) {
		INFO("Racket Science: RSGroundControlWidget()");

		setModule(module);
		this->module = module;

		box.size = Vec(RACK_GRID_WIDTH * 10, RACK_GRID_HEIGHT);
		int middle = box.size.x / 2 + 1;
		int quarter = middle / 2;

		addChild(new RSLabelCentered(middle, box.pos.y + 13, "GROUND CONTROL", 14));
		addChild(new RSLabelCentered(middle, box.size.y - 4, "Racket Science", 12));
		if(module)
			if(!module->running) {
				addChild(new RSLabelCentered(middle, box.size.y / 2, "DISABLED", 16));	
				addChild(new RSLabelCentered(middle, box.size.y / 2 + 12, "ONLY ONE INSTANCE OF GC REQUIRED"));
				return;
			}

		addChild(new RSLabelCentered(middle, 80, "THIS SPACE AVAILABLE", 16));
		addChild(new RSLabelCentered(middle, 100, "FOR SHORT TERM RENT", 16));

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
			addChild(new RSLabelCentered(left - 20, top + (ysp * 4) + 3, "LEDS A"));
			addChild(new RSLabelCentered(left - 20, top + (ysp * 5) + 3, "LEDS B"));

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

			addChild(new RSLedAWidget(left - 3, top + (ysp * 4) - 7));
			addChild(new RSLedBWidget(left - 3, top + (ysp * 5) - 7));

		}

		if(!module) return;

		module->params[RSGroundControl::THEME_KNOB].setValue(RSGlobal.themeIdx);

		module->updateParams();
	}

    void customDraw(const DrawArgs& args) {}
	#include "RSModuleWidgetDraw.hpp"

	void step() override {
		if(!module) return;
		if(!module->running) return;

		static struct rsglobal lastRSGlobal;
		
		static int lastTheme = 0;
		int theme = (int)module->params[RSGroundControl::THEME_KNOB].getValue();		

		if(theme != lastTheme) {
			RSGlobal.themeIdx = lastTheme = theme;
			module->updateParams();
			updateRSTheme(theme);
			saveRSGlobal();
		}

		// Ideally only want to do the following if any params have changed
		// How can we achieve that?  Can we test if the mouse is over our module, that would help
		// No point checking each param for change before reflecting that in RSGlobal, may as well just assign blindly

		RSGlobal.themes[RSGlobal.themeIdx].bghsl.hue = module->params[RSGroundControl::BGHUE_KNOB].getValue();
		RSGlobal.themes[RSGlobal.themeIdx].bghsl.sat = module->params[RSGroundControl::BGSAT_KNOB].getValue();
		RSGlobal.themes[RSGlobal.themeIdx].bghsl.lum = module->params[RSGroundControl::BGLUM_KNOB].getValue();

		RSGlobal.themes[RSGlobal.themeIdx].lbhsl.hue = module->params[RSGroundControl::LBHUE_KNOB].getValue();
		RSGlobal.themes[RSGlobal.themeIdx].lbhsl.sat = module->params[RSGroundControl::LBSAT_KNOB].getValue();
		RSGlobal.themes[RSGlobal.themeIdx].lbhsl.lum = module->params[RSGroundControl::LBLUM_KNOB].getValue();

		RSGlobal.themes[RSGlobal.themeIdx].sshsl.hue = module->params[RSGroundControl::SSHUE_KNOB].getValue();
		RSGlobal.themes[RSGlobal.themeIdx].sshsl.sat = module->params[RSGroundControl::SSSAT_KNOB].getValue();
		RSGlobal.themes[RSGlobal.themeIdx].sshsl.lum = module->params[RSGroundControl::SSLUM_KNOB].getValue();

		RSGlobal.themes[RSGlobal.themeIdx].ledAh = module->params[RSGroundControl::LEDA_KNOB].getValue();
		RSGlobal.themes[RSGlobal.themeIdx].ledBh = module->params[RSGroundControl::LEDB_KNOB].getValue();

		updateRSTheme(RSGlobal.themeIdx);

		// memcmp should suffice, nothing unusual in rsglobal struct
		if(memcmp(&RSGlobal, &lastRSGlobal, sizeof(rsglobal)) != 0) { // Theme settings have changed
			lastRSGlobal = RSGlobal; // memcpy?
			saveRSGlobal();
		}

		ModuleWidget::step();
	}
};

Model *modelRSGroundControl = createModel<RSGroundControl, RSGroundControlWidget>("RSGroundControl");


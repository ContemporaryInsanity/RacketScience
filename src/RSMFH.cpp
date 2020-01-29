#include "plugin.hpp"

#include "RS.hpp"

struct RSMFH : RSModule {
	enum ParamIds {
		THEME_BUTTON,
		VOLTAGE_KNOB,
		NUM_PARAMS
	};
	enum InputIds {
		TRIG_IN,
		NUM_INPUTS
	};
	enum OutputIds {
		MINF_OUT,
		PINF_OUT,
		NAN_OUT,
		VOLTAGE_OUT,
		EVIL_OUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	dsp::BooleanTrigger themeTrigger;

	RSMFH() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(THEME_BUTTON, 0.f, 1.f, 0.f, "THEME");

		configParam(VOLTAGE_KNOB, -20.f, +20.f, 0.f, "VOTLAGE");
	}

	void process(const ProcessArgs &args) override {
		if(themeTrigger.process(params[THEME_BUTTON].getValue())) {
			RSTheme++;
			if(RSTheme > RSGlobal.themeCount) RSTheme = 1;
		}

		outputs[MINF_OUT].setChannels(16);
		outputs[PINF_OUT].setChannels(16);
		outputs[NAN_OUT].setChannels(16);
		outputs[VOLTAGE_OUT].setChannels(16);
		outputs[EVIL_OUT].setChannels(16);

		for(int c = 0; c < 16; c++) {
			outputs[MINF_OUT].setVoltage(-INFINITY, c);
			outputs[PINF_OUT].setVoltage(INFINITY, c);
			outputs[NAN_OUT].setVoltage(NAN, c);
			outputs[VOLTAGE_OUT].setVoltage(params[VOLTAGE_KNOB].getValue(), c);

			switch(rand() % 6) {
				case 0:	outputs[EVIL_OUT].setVoltage(-INFINITY, c); break;
				case 1:	outputs[EVIL_OUT].setVoltage(INFINITY, c);	break;
				case 2:	outputs[EVIL_OUT].setVoltage(-666.666f, c);	break;
				case 3:	outputs[EVIL_OUT].setVoltage(666.666f, c);	break;
				case 4:	outputs[EVIL_OUT].setVoltage(NAN, c); 		break;
				default: outputs[EVIL_OUT].setVoltage(rand(), c);
			}
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

struct RSMFHWidget : ModuleWidget {
	RSMFH* module;

	RSMFHWidget(RSMFH *module) {
		INFO("Racket Science: RSMFHWidget()");

		setModule(module);
		this->module = module;

        box.size = Vec(RACK_GRID_WIDTH * 3, RACK_GRID_HEIGHT);
		int middle = box.size.x / 2 + 1;

		addParam(createParamCentered<RSButtonMomentaryInvisible>(Vec(box.pos.x + 5, box.pos.y + 5), module, RSMFH::THEME_BUTTON));

		addChild(new RSLabelCentered(middle, box.pos.y + 13, "MODULE", 14, module));
		addChild(new RSLabelCentered(middle, box.pos.y + 25, "FROM", 14, module));
		addChild(new RSLabelCentered(middle, box.pos.y + 37, "HELL", 14, module));

		addChild(new RSLabelCentered(middle, box.size.y - 15, "Racket", 12, module));
		addChild(new RSLabelCentered(middle, box.size.y - 4, "Science", 12, module));

		addOutput(createOutputCentered<RSJackPolyOut>(Vec(23, 72), module, RSMFH::MINF_OUT));
		addChild(new RSLabelCentered(middle, 94, "-INF", 10, module));

		addOutput(createOutputCentered<RSJackPolyOut>(Vec(23, 112), module, RSMFH::PINF_OUT));
		addChild(new RSLabelCentered(middle, 134, "+INF", 10, module));

		addOutput(createOutputCentered<RSJackPolyOut>(Vec(23, 152), module, RSMFH::NAN_OUT));
		addChild(new RSLabelCentered(middle, 174, "NAN", 10, module));

		addOutput(createOutputCentered<RSJackPolyOut>(Vec(23, 218), module, RSMFH::VOLTAGE_OUT));
		addParam(createParamCentered<RSKnobDetentSml>(Vec(23, 248), module, RSMFH::VOLTAGE_KNOB));
		addChild(new RSLabelCentered(middle, 270, "-20       +20", 10, module));

		addChild(new RSLabel(middle - 15, 306, "!EVIL!", 16, COLOR_RED));
		addOutput(createOutputCentered<RSJackPolyOut>(Vec(23, 322), module, RSMFH::EVIL_OUT));
		addChild(new RSLabel(middle - 15, 348, "!EVIL!", 16, COLOR_RED));
	}

    void customDraw(const DrawArgs& args) {}
	#include "RSModuleWidgetDraw.hpp"

	void step() override {
		if(!module) return;

		ModuleWidget::step();
	}
};

Model *modelRSMFH = createModel<RSMFH, RSMFHWidget>("RSMFH");
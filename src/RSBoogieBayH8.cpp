#include "plugin.hpp"

#include "components/RSComponents.hpp"
#include "RSUtils.hpp"


struct RSBoogieBayH8 : Module {
	enum ParamIds {
		THEME_BUTTON,
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(INPUTS, 8),
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(LEFT_OUTPUTS, 8),
		ENUMS(RIGHT_OUTPUTS, 8),
		POLY_LEFT_OUTPUT,
		POLY_RIGHT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	dsp::BooleanTrigger themeTrigger;

	RSScribbleStrip *ss[8];


	RSBoogieBayH8() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(THEME_BUTTON, 0.f, 1.f, 0.f, "THEME");
	}

	void process(const ProcessArgs &args) override {
		outputs[POLY_LEFT_OUTPUT].setChannels(8);
		outputs[POLY_RIGHT_OUTPUT].setChannels(8);

		if(themeTrigger.process(params[THEME_BUTTON].getValue())) {
			RSTheme++;
			if(RSTheme > RSThemes) RSTheme = 0;
			saveDefaultTheme(RSTheme);
		}

		for(int i = 0; i < 8; i++) {
			float inv = inputs[INPUTS + i].getVoltage();
			outputs[LEFT_OUTPUTS + i].setVoltage(inv);
			outputs[RIGHT_OUTPUTS + i].setVoltage(inv);
			outputs[POLY_LEFT_OUTPUT].setVoltage(inv, i);
			outputs[POLY_RIGHT_OUTPUT].setVoltage(inv, i);
		}
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		char ssn[4];

		for(int i = 0; i < 8; i++) {
			json_t* ssj = json_string(ss[i]->text.c_str());
			sprintf(ssn, "SS%i", i);
			json_object_set_new(rootJ, ssn, ssj);
		}

		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		char ssn[4];

		for(int i = 0; i < 8; i++) {
			sprintf(ssn, "SS%i", i);
			json_t* ssj = json_object_get(rootJ, ssn);
			if(ssj) ss[i]->text = json_string_value(ssj);
		}
	}
};


struct RSBoogieBayH8Widget : ModuleWidget {
	RSBoogieBayH8* module;

	PortWidget *in[8];
	int middle, left, right;

	RSBoogieBayH8Widget(RSBoogieBayH8 *module) {
		setModule(module);
		this->module = module;

		box.size.x = mm2px(5.08 * 25);
		middle = box.size.x / 2 + 1;
		left = 40;
		right = box.size.y - 40;

		RSTheme = loadDefaultTheme();

		addChild(new RSLabelCentered(middle, box.pos.y + 13, "BOOGIE BAY H8", 14));
		addChild(new RSLabelCentered(middle, box.size.y - 4, "Racket Science", 12));

		addParam(createParamCentered<RSButtonMomentaryInvisible>(Vec(box.pos.x + 5, box.pos.y + 5), module, RSBoogieBayH8::THEME_BUTTON));

		for(int i = 0; i < 8; i++) {
			in[i] = createInputCentered<RSJackMonoIn>(Vec(middle, 40 + (i * 40)), module, RSBoogieBayH8::INPUTS + i); addInput(in[i]);
			addOutput(createOutputCentered<RSJackMonoOut>(Vec(left, 40 + (i * 40)), module, RSBoogieBayH8::LEFT_OUTPUTS + i));
			addOutput(createOutputCentered<RSJackMonoOut>(Vec(right, 40 + (i * 40)), module, RSBoogieBayH8::RIGHT_OUTPUTS + i));
			if(module) addChild(module->ss[i] = new RSScribbleStrip(left + 25, 25 + (i * 40)));
		}

		addOutput(createOutputCentered<RSJackPolyOut>(Vec(left, 360), module, RSBoogieBayH8::POLY_LEFT_OUTPUT));
		addOutput(createOutputCentered<RSJackPolyOut>(Vec(right, 360), module, RSBoogieBayH8::POLY_RIGHT_OUTPUT));
	}

    void customDraw(const DrawArgs& args) {
		// Socket slots
		nvgLineCap(args.vg, NVG_ROUND);
		nvgStrokeColor(args.vg, COLOR_BLACK);
		nvgStrokeWidth(args.vg, 5);

		for(int i = 0; i < 8; i++) {
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, left + 30, 40 + (i * 40));
			nvgLineTo(args.vg, right - 30, 40 + (i * 40));
			nvgStroke(args.vg);
		}

	}
	#include "RSModuleWidgetDraw.hpp"

	void step() override {
		if(!module) return;

		for(int i = 0; i < 8; i++) {
			in[i]->box.pos.x = (middle - 10 + (clamp10V(module->inputs[RSBoogieBayH8::INPUTS + i].getVoltage()) * 12));
		}

		ModuleWidget::step();
	}
};

Model *modelRSBoogieBayH8 = createModel<RSBoogieBayH8, RSBoogieBayH8Widget>("RSBoogieBayH8");
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
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	dsp::BooleanTrigger themeTrigger;

	RSBoogieBayH8() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(THEME_BUTTON, 0.f, 1.f, 0.f, "THEME");
	}

	void process(const ProcessArgs &args) override {

		if(themeTrigger.process(params[THEME_BUTTON].getValue())) {
			RSTheme++;
			if(RSTheme > RSThemes) RSTheme = 0;
			saveDefaultTheme(RSTheme);
		}

		for(int i = 0; i < 8; i++) {
			outputs[LEFT_OUTPUTS + i].setVoltage(inputs[INPUTS + i].getVoltage());
			outputs[RIGHT_OUTPUTS + i].setVoltage(inputs[INPUTS + i].getVoltage());
		}
	}
};


struct RSBoogieBayH8Widget : ModuleWidget {
	RSBoogieBayH8* module;

	PortWidget *in[8];
	int middle;

	RSBoogieBayH8Widget(RSBoogieBayH8 *module) {
		setModule(module);
		this->module = module;

		box.size.x = mm2px(5.09 * 25);
		middle = box.size.x / 2 + 1;

		RSTheme = loadDefaultTheme();

		addChild(new RSLabelCentered(middle, box.pos.y + 13, "BOOGIE BAY H8", 14));
		addChild(new RSLabelCentered(middle, box.size.y - 4, "Racket Science", 12));

		addParam(createParamCentered<RSButtonMomentaryInvisible>(Vec(box.pos.x + 5, box.pos.y + 5), module, RSBoogieBayH8::THEME_BUTTON));

		int left = 40;
		int right = box.size.y - 40;

		for(int i = 0; i < 8; i++) {
			in[i] = createInputCentered<RSJackMonoIn>(Vec(middle, 40 + (i * 40)), module, RSBoogieBayH8::INPUTS + i); addInput(in[i]);
			addOutput(createOutputCentered<RSJackMonoOut>(Vec(left, 40 + (i * 40)), module, RSBoogieBayH8::LEFT_OUTPUTS + i));
			addOutput(createOutputCentered<RSJackMonoOut>(Vec(right, 40 + (i * 40)), module, RSBoogieBayH8::RIGHT_OUTPUTS + i));
			// Add a scribble strip here
		}
	}

    void customDraw(const DrawArgs& args) {
		// Socket slots draw here
	}
	#include "RSModuleWidgetDraw.hpp"

	void step() override {
		if(!module) return;

		for(int i = 0; i < 8; i++)  // This isn't centering properly.
			in[i]->box.pos.x = (middle + (clamp10V(module->inputs[RSBoogieBayH8::INPUTS + i].getVoltage())) * 11);

		ModuleWidget::step();
	}
};

Model *modelRSBoogieBayH8 = createModel<RSBoogieBayH8, RSBoogieBayH8Widget>("RSBoogieBayH8");
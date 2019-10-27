#include "plugin.hpp"

#include "components/RSComponents.hpp"

struct RSScratch : Module {
	enum ParamIds {
		PARAM1_PARAM,
		PARAM2_PARAM,
		PARAM3_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN1_INPUT,
		IN2_INPUT,
		IN3_INPUT,
		IN4_INPUT,
		IN5_INPUT,
		IN6_INPUT,
		IN7_INPUT,
		IN8_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTL1_OUTPUT,
		OUTR1_OUTPUT,
		OUTL2_OUTPUT,
		OUTR2_OUTPUT,
		OUTL3_OUTPUT,
		OUTR3_OUTPUT,
		OUTL4_OUTPUT,
		OUTR4_OUTPUT,
		OUTL5_OUTPUT,
		OUTR5_OUTPUT,
		OUTL6_OUTPUT,
		OUTR6_OUTPUT,
		OUTL7_OUTPUT,
		OUTR7_OUTPUT,
		OUTL8_OUTPUT,
		OUTR8_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		LIGHT1_LIGHT,
		LIGHT2_LIGHT,
		LIGHT3_LIGHT,
		LIGHT4_LIGHT,
		NUM_LIGHTS
	};

	RSScratch() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PARAM1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(PARAM2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(PARAM3_PARAM, 0.f, 1.f, 0.f, "");
	}

	void process(const ProcessArgs &args) override {
	}
};


struct Display : TransparentWidget {
	RSScratch* module;

	std::shared_ptr<Font> font;

	Display() {
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Ubuntu Condensed 400.ttf"));
	}

	void draw(const DrawArgs& args) override {
		if(!module) return;

		nvgFontSize(args.vg, 10);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, 0);

		Rect b = Rect(Vec(0, 15), box.size.minus(Vec(0, 15 * 2)));
		//nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);

		nvgStrokeColor(args.vg, COLOR_RS_GREY);
		nvgText(args.vg, 0, 10, "OpenGL Text Test", NULL);

		nvgStrokeColor(args.vg, COLOR_GREEN);

		nvgBeginPath(args.vg);

		nvgMoveTo(args.vg, 0, 0);
		nvgLineTo(args.vg, 500, 500);

		nvgClosePath(args.vg);
		nvgStroke(args.vg);

	
		//nvgResetScissor(args.vg);
	}

};


struct RSScratchWidget : ModuleWidget {
	RSScratchWidget(RSScratch *module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/RSScratch.svg")));

		{
			Display* display = new Display();
			display->module = module;
			display->box.pos = Vec(0, 0);
			display->box.size = Vec(box.size.x, box.size.y);
			addChild(display);
		}

		addParam(createParamCentered<RSKnobMedBlk>(mm2px(Vec(38.981, 15.029)), module, RSScratch::PARAM1_PARAM));
		addParam(createParamCentered<RSKnobMedBlk>(mm2px(Vec(38.829, 35.159)), module, RSScratch::PARAM2_PARAM));
		addParam(createParamCentered<RSKnobMedBlk>(mm2px(Vec(38.678, 61.041)), module, RSScratch::PARAM3_PARAM));

		addInput(createInputCentered<RSJackMonoIn>(mm2px(Vec(63.5, 12.608)), module, RSScratch::IN1_INPUT));
		addInput(createInputCentered<RSJackMonoIn>(mm2px(Vec(63.5, 27.848)), module, RSScratch::IN2_INPUT));
		addInput(createInputCentered<RSJackMonoIn>(mm2px(Vec(63.5, 43.088)), module, RSScratch::IN3_INPUT));
		addInput(createInputCentered<RSJackMonoIn>(mm2px(Vec(63.5, 58.328)), module, RSScratch::IN4_INPUT));
		addInput(createInputCentered<RSJackMonoIn>(mm2px(Vec(63.5, 73.568)), module, RSScratch::IN5_INPUT));
		addInput(createInputCentered<RSJackMonoIn>(mm2px(Vec(63.5, 88.808)), module, RSScratch::IN6_INPUT));
		addInput(createInputCentered<RSJackMonoIn>(mm2px(Vec(63.5, 104.048)), module, RSScratch::IN7_INPUT));
		addInput(createInputCentered<RSJackMonoIn>(mm2px(Vec(63.5, 119.288)), module, RSScratch::IN8_INPUT));

		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(10.003, 12.608)), module, RSScratch::OUTL1_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(9.948, 27.848)), module, RSScratch::OUTL2_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(117.052, 27.848)), module, RSScratch::OUTR2_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(117.052, 12.608)), module, RSScratch::OUTR1_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(9.948, 43.088)), module, RSScratch::OUTL3_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(117.052, 43.088)), module, RSScratch::OUTR3_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(9.948, 58.328)), module, RSScratch::OUTL4_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(117.052, 58.328)), module, RSScratch::OUTR4_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(9.948, 73.568)), module, RSScratch::OUTL5_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(117.052, 73.568)), module, RSScratch::OUTR5_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(9.948, 88.808)), module, RSScratch::OUTL6_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(117.052, 88.808)), module, RSScratch::OUTR6_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(9.948, 104.048)), module, RSScratch::OUTL7_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(117.052, 104.048)), module, RSScratch::OUTR7_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(9.948, 119.288)), module, RSScratch::OUTL8_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(117.052, 119.288)), module, RSScratch::OUTR8_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(89.382, 13.97)), module, RSScratch::LIGHT1_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(89.382, 13.97)), module, RSScratch::LIGHT2_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(89.533, 33.646)), module, RSScratch::LIGHT3_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(89.836, 60.587)), module, RSScratch::LIGHT4_LIGHT));

		addChild(createWidgetCentered<Widget>(mm2px(Vec(38.375, 95.55))));
		addChild(createWidgetCentered<Widget>(mm2px(Vec(89.836, 95.55))));
	}
};


Model *modelRSScratch = createModel<RSScratch, RSScratchWidget>("RSScratch");
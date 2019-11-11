#include "plugin.hpp"

#include "components/RSComponents.hpp"
#include "RSUtils.hpp"


struct RSBoogieBayH8 : Module {
	enum ParamIds {
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
		NUM_LIGHTS
	};

	RSBoogieBayH8() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs &args) override {

		outputs[OUTL1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage()); outputs[OUTR1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage());
		outputs[OUTL2_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage()); outputs[OUTR2_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage());
		outputs[OUTL3_OUTPUT].setVoltage(inputs[IN3_INPUT].getVoltage()); outputs[OUTR3_OUTPUT].setVoltage(inputs[IN3_INPUT].getVoltage());
		outputs[OUTL4_OUTPUT].setVoltage(inputs[IN4_INPUT].getVoltage()); outputs[OUTR4_OUTPUT].setVoltage(inputs[IN4_INPUT].getVoltage());
		outputs[OUTL5_OUTPUT].setVoltage(inputs[IN5_INPUT].getVoltage()); outputs[OUTR5_OUTPUT].setVoltage(inputs[IN5_INPUT].getVoltage());
		outputs[OUTL6_OUTPUT].setVoltage(inputs[IN6_INPUT].getVoltage()); outputs[OUTR6_OUTPUT].setVoltage(inputs[IN6_INPUT].getVoltage());
		outputs[OUTL7_OUTPUT].setVoltage(inputs[IN7_INPUT].getVoltage()); outputs[OUTR7_OUTPUT].setVoltage(inputs[IN7_INPUT].getVoltage());
		outputs[OUTL8_OUTPUT].setVoltage(inputs[IN8_INPUT].getVoltage()); outputs[OUTR8_OUTPUT].setVoltage(inputs[IN8_INPUT].getVoltage());

	}
};


struct RSBoogieBayH8Widget : ModuleWidget {
	RSBoogieBayH8Widget(RSBoogieBayH8 *module) {
		setModule(module);
		_module = module;

		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/RSBoogieBayH8.svg")));

		in1 = createInputCentered<RSJackMonoIn>(mm2px(Vec(63.5, 12.608)), module, RSBoogieBayH8::IN1_INPUT);
		in2 = createInputCentered<RSJackMonoIn>(mm2px(Vec(63.5, 27.848)), module, RSBoogieBayH8::IN2_INPUT);
		in3 = createInputCentered<RSJackMonoIn>(mm2px(Vec(63.5, 43.088)), module, RSBoogieBayH8::IN3_INPUT);
		in4 = createInputCentered<RSJackMonoIn>(mm2px(Vec(63.5, 58.328)), module, RSBoogieBayH8::IN4_INPUT);
		in5 = createInputCentered<RSJackMonoIn>(mm2px(Vec(63.5, 73.568)), module, RSBoogieBayH8::IN5_INPUT);
		in6 = createInputCentered<RSJackMonoIn>(mm2px(Vec(63.5, 88.808)), module, RSBoogieBayH8::IN6_INPUT);
		in7 = createInputCentered<RSJackMonoIn>(mm2px(Vec(63.5, 104.048)), module, RSBoogieBayH8::IN7_INPUT);
		in8 = createInputCentered<RSJackMonoIn>(mm2px(Vec(63.5, 119.288)), module, RSBoogieBayH8::IN8_INPUT);
		
		addInput(in1);
		addInput(in2);
		addInput(in3);
		addInput(in4);
		addInput(in5);
		addInput(in6);
		addInput(in7);
		addInput(in8);

		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(10.003, 12.608)), module, RSBoogieBayH8::OUTL1_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(117.052, 12.608)), module, RSBoogieBayH8::OUTR1_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(9.948, 27.848)), module, RSBoogieBayH8::OUTL2_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(117.052, 27.848)), module, RSBoogieBayH8::OUTR2_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(9.948, 43.088)), module, RSBoogieBayH8::OUTL3_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(117.052, 43.088)), module, RSBoogieBayH8::OUTR3_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(9.948, 58.328)), module, RSBoogieBayH8::OUTL4_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(117.052, 58.328)), module, RSBoogieBayH8::OUTR4_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(9.948, 73.568)), module, RSBoogieBayH8::OUTL5_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(117.052, 73.568)), module, RSBoogieBayH8::OUTR5_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(9.948, 88.808)), module, RSBoogieBayH8::OUTL6_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(117.052, 88.808)), module, RSBoogieBayH8::OUTR6_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(9.948, 104.048)), module, RSBoogieBayH8::OUTL7_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(117.052, 104.048)), module, RSBoogieBayH8::OUTR7_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(9.948, 119.288)), module, RSBoogieBayH8::OUTL8_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(117.052, 119.288)), module, RSBoogieBayH8::OUTR8_OUTPUT));
	}

	RSBoogieBayH8* _module;

	bool init = true;

	PortWidget *in1, *in2, *in3, *in4, *in5, *in6, *in7, *in8;

	void step() override {
		if(!_module) return;

		if(init) {

			init = false;
		}

		float in1v = clamp10V(module->inputs[RSBoogieBayH8::IN1_INPUT].getVoltage());
		float in2v = clamp10V(module->inputs[RSBoogieBayH8::IN2_INPUT].getVoltage());
		float in3v = clamp10V(module->inputs[RSBoogieBayH8::IN3_INPUT].getVoltage());
		float in4v = clamp10V(module->inputs[RSBoogieBayH8::IN4_INPUT].getVoltage());
		float in5v = clamp10V(module->inputs[RSBoogieBayH8::IN5_INPUT].getVoltage());
		float in6v = clamp10V(module->inputs[RSBoogieBayH8::IN6_INPUT].getVoltage());
		float in7v = clamp10V(module->inputs[RSBoogieBayH8::IN7_INPUT].getVoltage());
		float in8v = clamp10V(module->inputs[RSBoogieBayH8::IN8_INPUT].getVoltage());

		int xpos1, xpos2, xpos3, xpos4, xpos5, xpos6, xpos7, xpos8;

		xpos1 = mm2px(60 + in1v * 4);
		xpos2 = mm2px(60 + in2v * 4);
		xpos3 = mm2px(60 + in3v * 4);
		xpos4 = mm2px(60 + in4v * 4);
		xpos5 = mm2px(60 + in5v * 4);
		xpos6 = mm2px(60 + in6v * 4);
		xpos7 = mm2px(60 + in7v * 4);
		xpos8 = mm2px(60 + in8v * 4);

		in1->box.pos.x = xpos1;
		in2->box.pos.x = xpos2;
		in3->box.pos.x = xpos3;
		in4->box.pos.x = xpos4;
		in5->box.pos.x = xpos5;
		in6->box.pos.x = xpos6;
		in7->box.pos.x = xpos7;
		in8->box.pos.x = xpos8;

		ModuleWidget::step();
	}
};


Model *modelRSBoogieBayH8 = createModel<RSBoogieBayH8, RSBoogieBayH8Widget>("RSBoogieBayH8");
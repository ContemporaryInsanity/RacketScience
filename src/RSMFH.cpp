#include "plugin.hpp"

#include "components/RSComponents.hpp"
#include "RSUtils.hpp"


struct RSMFH : Module {
	enum ParamIds {
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
		MINF_P_OUT,
		PINF_P_OUT,
		NAN_P_OUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	RSMFH() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	dsp::SchmittTrigger trig;

	void process(const ProcessArgs &args) override {
		static bool once = true;

		if(once) {
			outputs[MINF_OUT].setVoltage(-INFINITY);
			outputs[PINF_OUT].setVoltage(INFINITY);
			outputs[NAN_OUT].setVoltage(NAN);
			once = false;
		}

		if(trig.process(inputs[TRIG_IN].getVoltage() > 0.f)) {
			outputs[MINF_P_OUT].setVoltage(-INFINITY);
			outputs[PINF_P_OUT].setVoltage(INFINITY);
			outputs[NAN_P_OUT].setVoltage(NAN);
			INFO("Racket Science: trig");
		}
		else {
			outputs[MINF_P_OUT].setVoltage(0.f);
			outputs[PINF_P_OUT].setVoltage(0.f);
			outputs[NAN_P_OUT].setVoltage(0.f);
		}
	}
};


struct RSMFHWidget : ModuleWidget {
	RSMFH* module;

	RSMFHWidget(RSMFH *module) {
		setModule(module);
		this->module = module;

		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/RSMFH.svg")));

		addChild(new RSLabel(5, 54, "CONSTANT"));

		addOutput(createOutputCentered<RSJackMonoOut>(Vec(23, 72), module, RSMFH::MINF_OUT));
		addChild(new RSLabel(16, 94, "-INF"));

		addOutput(createOutputCentered<RSJackMonoOut>(Vec(23, 112), module, RSMFH::PINF_OUT));
		addChild(new RSLabel(16, 134, "+INF"));

		addOutput(createOutputCentered<RSJackMonoOut>(Vec(23, 152), module, RSMFH::NAN_OUT));
		addChild(new RSLabel(16, 174, "NAN"));

		addChild(new RSLabel(10, 194, "PULSED"));

		addInput(createInputCentered<RSJackMonoIn>(Vec(23, 212), module, RSMFH::TRIG_IN));;
		addChild(new RSLabel(15, 234, "TRIG"));

		addOutput(createOutputCentered<RSJackMonoOut>(Vec(23, 252), module, RSMFH::MINF_P_OUT));
		addChild(new RSLabel(16, 274, "-INF"));

		addOutput(createOutputCentered<RSJackMonoOut>(Vec(23, 292), module, RSMFH::PINF_P_OUT));
		addChild(new RSLabel(16, 314, "+INF"));

		addOutput(createOutputCentered<RSJackMonoOut>(Vec(23, 332), module, RSMFH::NAN_P_OUT));
		addChild(new RSLabel(16, 354, "NAN"));

	}

	void step() override {
		if(!module) return;

		ModuleWidget::step();
	}
};

Model *modelRSMFH = createModel<RSMFH, RSMFHWidget>("RSMFH");
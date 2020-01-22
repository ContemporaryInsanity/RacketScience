#include "plugin.hpp"

#include "RS.hpp"

struct RSLaunchControl : RSModule {
	enum ParamIds {
		THEME_BUTTON,
		ARM_PARAM,
		STEPS_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		PHASE_IN,
		ARM_IN,
		NUM_INPUTS
	};
	enum OutputIds {
		PHASE_OUT,
		RUNNING_OUT,
		STEP_OUT,
		EOC_OUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		STOPPED_LIGHT,	// Red
		ARMED_LIGHT,	// Yellow
		RUNNING_LIGHT,	// Green
		NUM_LIGHTS
	};

	dsp::BooleanTrigger themeTrigger;

	dsp::BooleanTrigger armInTrigger, armTrigger;

	dsp::SchmittTrigger eocTrigger;

	dsp::PulseGenerator stepPulse;
	dsp::PulseGenerator eocPulse;

	bool armed = false;
	bool running = false;
	bool step = false;
	bool eoc = false;

	float phaseIn, priorPhaseIn;

	RSLaunchControl() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(THEME_BUTTON, 0.f, 1.f, 0.f, "THEME");

		configParam(ARM_PARAM, 0.f, 1.f, 0.f, "ARM");
		configParam(STEPS_PARAM, 2.f, 64.f, 8.f, "STEPS");
	}

	void process(const ProcessArgs &args) override {
		if(themeTrigger.process(params[THEME_BUTTON].getValue())) {
			RSTheme++;
			if(RSTheme > RSGlobal.themeCount) RSTheme = 1;
		}

		if(inputs[ARM_IN].isConnected()) {
			if(armInTrigger.process(inputs[ARM_IN].getVoltage())) {
				//if(!running) {
					INFO("Racket Science: Launch Control armed");
					armed = true;
				//}
			}
		}

		if(armTrigger.process(params[ARM_PARAM].getValue())) {
			if(!running) {
				INFO("Racket Science: Launch Control armed via button");
				armed = true;
			}
		}

		phaseIn = inputs[PHASE_IN].getVoltage();

		if(armed) {
			if(phaseIn < priorPhaseIn) {
				INFO("Racket Science: Launch Control running");
				running = true;
				armed = false;
				priorPhaseIn = phaseIn; // So we don't stop immediately below
				params[ARM_PARAM].setValue(0.f);
			}
		}

		if(running) {
			outputs[PHASE_OUT].setVoltage(phaseIn);
			if(phaseIn < priorPhaseIn) {
				INFO("Racket Science: Launch Control stopped");
				running = false;
				eocPulse.trigger();
				stepPulse.trigger();
			}
		}

		float steps = params[STEPS_PARAM].getValue();
		float phaseStep = 10.f / steps;
		if(running) {
			for(float step = phaseStep; step < 10.f; step += phaseStep) {
				if(phaseIn > step && priorPhaseIn <= step) {
					INFO("Racket Science: step %f", phaseIn);
					stepPulse.trigger();
				}
			}
		}

		step = stepPulse.process(1.f / args.sampleRate);
		eoc = eocPulse.process(1.f / args.sampleRate);
		
		outputs[EOC_OUT].setVoltage(eoc ? 10.f : 0.f);
		outputs[RUNNING_OUT].setVoltage(running ? 10.f : 0.f);
		outputs[STEP_OUT].setVoltage(step ? 10.f : 0.f);

		//params[ARM_PARAM].setValue(armed ? 1.f : 0.f);
		lights[STOPPED_LIGHT].setSmoothBrightness(!running ? 1.f : 0.f, 1.f);
		lights[ARMED_LIGHT].setSmoothBrightness(armed ? 1.f : 0.f, 1.f);
		lights[RUNNING_LIGHT].setSmoothBrightness(running ? 1.f : 0.f, 1.f);

		priorPhaseIn = phaseIn;
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


struct RSLaunchControlWidget : ModuleWidget {
	RSLaunchControl* module;

	RSLaunchControlWidget(RSLaunchControl *module) {
		setModule(module);
		this->module = module;

		box.size = Vec(RACK_GRID_WIDTH * 30, RACK_GRID_HEIGHT);
		int middle = box.size.x / 2 + 1;

		addParam(createParamCentered<RSButtonMomentaryInvisible>(Vec(box.pos.x + 5, box.pos.y + 5), module, RSLaunchControl::THEME_BUTTON));

		addChild(new RSLabelCentered(middle, box.pos.y + 13, "LAUNCH CONTROL", 14, module));
		addChild(new RSLabelCentered(middle, box.size.y - 4, "Racket Science", 12, module));

		int x, y;

		LightWidget *lightWidget;

		// PHASE IN
		x = 25; y = 50;
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y), module, RSLaunchControl::PHASE_IN));
		addChild(new RSLabelCentered(x, y - 18, "PHASE", 10, module));

		// ARM IN
		x += 35;
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y), module, RSLaunchControl::ARM_IN));
		addChild(new RSLabelCentered(x, y - 18, "ARM", 10, module));

		// ARM BUTTON
		x += 35;
		addParam(createParamCentered<RSButtonToggle>(Vec(x, y), module, RSLaunchControl::ARM_PARAM));
		addChild(new RSLabelCentered(x, y + 3, "ARM", 10, module));

		// STOPPED LIGHT
		x += 35;
		addChild(createLightCentered<LargeLight<RedLight>>(Vec(x, y), module, RSLaunchControl::STOPPED_LIGHT));
		addChild(new RSLabelCentered(x, y - 18, "STOPPED", 10, module));

		// ARMED LIGHT
		x += 35;
		addChild(createLightCentered<LargeLight<YellowLight>>(Vec(x, y), module, RSLaunchControl::ARMED_LIGHT));
		addChild(new RSLabelCentered(x, y - 18, "ARMED", 10, module));

		// RUNNING LIGHT
		x += 35;
		addChild(createLightCentered<LargeLight<GreenLight>>(Vec(x, y), module, RSLaunchControl::RUNNING_LIGHT));
		addChild(new RSLabelCentered(x, y - 18, "RUNNING", 10, module));

		// RUNNING OUT
		x += 35;
		addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y), module, RSLaunchControl::RUNNING_OUT));
		addChild(new RSLabelCentered(x, y - 18, "RUNNING", 10, module));

		// PHASE OUT
		x += 35;
		addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y), module, RSLaunchControl::PHASE_OUT));
		addChild(new RSLabelCentered(x, y - 18, "PHASE", 10, module));

		// STEPS
		x += 35;
		addParam(createParamCentered<RSKnobDetentSml>(Vec(x, y), module, RSLaunchControl::STEPS_PARAM));
		addChild(new RSLabelCentered(x, y - 18, "STEPS", 10, module));

		// STEP OUT
		x += 35;
		addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y), module, RSLaunchControl::STEP_OUT));
		addChild(new RSLabelCentered(x, y - 18, "STEP", 10, module));

		// EOC OUT
		x += 35;
		addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y), module, RSLaunchControl::EOC_OUT));
		addChild(new RSLabelCentered(x, y - 18, "EOC", 10, module));
		
	}

	void step() override {
		if(!module) return;

		ModuleWidget::step();
	}

    void customDraw(const DrawArgs& args) {}
	#include "RSModuleWidgetDraw.hpp"
};

Model *modelRSLaunchControl = createModel<RSLaunchControl, RSLaunchControlWidget>("RSLaunchControl");
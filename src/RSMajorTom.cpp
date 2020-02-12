#include "plugin.hpp"

#include "RS.hpp"

struct RSMajorTom : RSModule {
	enum ParamIds {
		THEME_BUTTON,

		ATT_A_SCALE,
		ATT_A_OFFSET,
		ATT_B_SCALE,
		ATT_B_OFFSET,

		PULSE_A_BUTTON,
		GATE_A_BUTTON,
		DOOR_A_BUTTON,

		PULSE_B_BUTTON,
		GATE_B_BUTTON,
		DOOR_B_BUTTON,

		NUM_PARAMS
	};
	enum InputIds {
		ATT_A_IN,
		ATT_B_IN,

		NUM_INPUTS
	};
	enum OutputIds {
		ATT_A_OUT,
		ATT_B_OUT,

		PULSEGATE_A_OUT,
		PULSEGATE_B_OUT,

		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	dsp::BooleanTrigger themeTrigger;

	dsp::SchmittTrigger pulseTiggerA, pulseTriggerB;
	dsp::BooleanTrigger gateTriggerA, gateTriggerB;
	dsp::PulseGenerator pulseGeneratorA, pulseGeneratorB;
	bool pulseA, pulseB;

	RSMajorTom() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(THEME_BUTTON, 0.f, 1.f, 0.f, "THEME");

		configParam(ATT_A_SCALE, -1.f, 1.f, 0.f, "SCALE");
		configParam(ATT_A_OFFSET, -10.f, 10.f, 0.f, "OFFSET");
		configParam(ATT_B_SCALE, -1.f, 1.f, 0.f, "SCALE");
		configParam(ATT_B_OFFSET, -10.f, 10.f, 0.f, "OFFSET");

		configParam(PULSE_A_BUTTON, 0.f, 1.f, 0.f, "PULSE");
		configParam(GATE_A_BUTTON, 0.f, 1.f, 0.f, "GATE");
		configParam(DOOR_A_BUTTON, 0.f, 1.f, 0.f, "DOOR");

		configParam(PULSE_B_BUTTON, 0.f, 1.f, 0.f, "PULSE");
		configParam(GATE_B_BUTTON, 0.f, 1.f, 0.f, "GATE");
		configParam(DOOR_B_BUTTON, 0.f, 1.f, 0.f, "DOOR");

		pulseA = pulseB = false;
	}

	void process(const ProcessArgs &args) override {
		if(themeTrigger.process(params[THEME_BUTTON].getValue())) {
			RSTheme++;
			if(RSTheme > RSGlobal.themeCount) RSTheme = 1;
		}

		// Attenuverters
		float cvIn, cvOut;

		cvIn = RSclamp(inputs[ATT_A_IN].getVoltage(),-10.f, 10.f);
		cvOut = RSclamp(cvIn * params[ATT_A_SCALE].getValue() + params[ATT_A_OFFSET].getValue(), -10.f, 10.f);
		outputs[ATT_A_OUT].setVoltage(cvOut);

		cvIn = RSclamp(inputs[ATT_B_IN].getVoltage(),-10.f, 10.f);
		cvOut = RSclamp(cvIn * params[ATT_B_SCALE].getValue() + params[ATT_B_OFFSET].getValue(), -10.f, 10.f);
		outputs[ATT_B_OUT].setVoltage(cvOut);

		// Pulses / gates
		if(params[DOOR_A_BUTTON].getValue()) pulseA = true;
		else {
			pulseA = false;
			if(pulseTiggerA.process(params[PULSE_A_BUTTON].getValue() > 0.f)) pulseGeneratorA.trigger(1e-3f);
			pulseA = pulseGeneratorA.process(args.sampleTime);
			if(params[GATE_A_BUTTON].getValue()) pulseA = true;
		}

		if(params[DOOR_B_BUTTON].getValue()) pulseB = true;
		else {
			pulseB = false;
			if(pulseTriggerB.process(params[PULSE_B_BUTTON].getValue() > 0.f)) pulseGeneratorB.trigger(1e-3f);
			pulseB = pulseGeneratorB.process(args.sampleTime);
			if(params[GATE_B_BUTTON].getValue()) pulseB = true;
		}

		outputs[PULSEGATE_A_OUT].setVoltage(pulseA ? 10.f : 0.f);
		outputs[PULSEGATE_B_OUT].setVoltage(pulseB ? 10.f : 0.f);
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


struct RSMajorTomWidget : ModuleWidget {
	RSMajorTom* module;

	int x, y, smlGap, lrgGap, labOfs;

	RSMajorTomWidget(RSMajorTom *module) {
		INFO("Racket Science: RSMajorTomWidget()");

		setModule(module);
		this->module = module;

		box.size = Vec(RACK_GRID_WIDTH * 9, RACK_GRID_HEIGHT);
		int middle = box.size.x / 2 + 1;

		addParam(createParamCentered<RSButtonMomentaryInvisible>(Vec(box.pos.x + 5, box.pos.y + 5), module, RSMajorTom::THEME_BUTTON));

		addChild(new RSLabelCentered(middle, box.pos.y + 13, "MAJOR TOM", 14, module));
		addChild(new RSLabelCentered(middle, box.size.y - 4, "Racket Science", 12, module));

		x = 25; y = 50;
		smlGap = 30; lrgGap = 65;
		labOfs = 20;

		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y), module, RSMajorTom::ATT_A_IN));
		x += smlGap; y -= labOfs;
		addChild(new RSLabelCentered(x, y, "SCALE", 10, module));
		y += labOfs;
		addParam(createParamCentered<RSKnobSml>(Vec(x, y), module, RSMajorTom::ATT_A_SCALE));
		x += smlGap; y -= labOfs;
		addChild(new RSLabelCentered(x, y, "OFFSET", 10, module));
		y += labOfs;
		addParam(createParamCentered<RSKnobSml>(Vec(x, y), module, RSMajorTom::ATT_A_OFFSET));
		x += smlGap;
		addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y), module, RSMajorTom::ATT_A_OUT));

		x = 25; y += smlGap;
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y), module, RSMajorTom::ATT_B_IN));
		x += smlGap;
		addParam(createParamCentered<RSKnobSml>(Vec(x, y), module, RSMajorTom::ATT_B_SCALE));
		x += smlGap;
		addParam(createParamCentered<RSKnobSml>(Vec(x, y), module, RSMajorTom::ATT_B_OFFSET));
		x += smlGap;
		addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y), module, RSMajorTom::ATT_B_OUT));
		y += smlGap;

		// Pulses / gates
		addChild(new RSLabelCentered(middle, y, "PULSES / GATES / DOORS", 10, module));
		x = 25; y += labOfs;
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSMajorTom::PULSE_A_BUTTON));
		addChild(new RSLabelCentered(x, y + 3, "PULSE", 10, module));
		x += smlGap;
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSMajorTom::GATE_A_BUTTON));
		addChild(new RSLabelCentered(x, y + 3, "GATE", 10, module));
		x += smlGap;
		addParam(createParamCentered<RSButtonToggle>(Vec(x, y), module, RSMajorTom::DOOR_A_BUTTON));
		addChild(new RSLabelCentered(x, y + 3, "DOOR", 10, module));
		x += smlGap;
		addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y), module, RSMajorTom::PULSEGATE_A_OUT));

		x = 25; y += smlGap;
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSMajorTom::PULSE_B_BUTTON));
		addChild(new RSLabelCentered(x, y + 3, "PULSE", 10, module));
		x += smlGap;
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSMajorTom::GATE_B_BUTTON));
		addChild(new RSLabelCentered(x, y + 3, "GATE", 10, module));
		x += smlGap;
		addParam(createParamCentered<RSButtonToggle>(Vec(x, y), module, RSMajorTom::DOOR_B_BUTTON));
		addChild(new RSLabelCentered(x, y + 3, "DOOR", 10, module));
		x += smlGap;
		addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y), module, RSMajorTom::PULSEGATE_B_OUT));
		y += smlGap;

		// Whatever goes here
		addChild(new RSLabelCentered(middle, y, "something sometihng", 10, module));
		x = 25; y += labOfs;


	}

	void step() override {
		if(!module) return;

		ModuleWidget::step();
	}

    void customDraw(const DrawArgs& args) {}
	#include "RSModuleWidgetDraw.hpp"
};

Model *modelRSMajorTom = createModel<RSMajorTom, RSMajorTomWidget>("RSMajorTom");
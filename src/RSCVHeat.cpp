#include "plugin.hpp"

#include "components/RSComponents.hpp"
#include "RSUtils.hpp"

struct RSCVHeat : Module {
	enum ParamIds {
		GAIN_KNOB,
		LOSS_KNOB,
		RESET_BUTTON,
		NUM_PARAMS
	};
	enum InputIds {
		CV_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	RSCVHeat() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(GAIN_KNOB, 0.00001f, 0.001f, 0.0005f, "GAIN");
        configParam(LOSS_KNOB, 0.00001f, 0.001f, 0.0005f, "LOSS");
		configParam(RESET_BUTTON, 0.f, 1.f, 0.f, "RESET");
	}

	dsp::BooleanTrigger themeTrigger;
	dsp::BooleanTrigger resetTrigger;

	#define SAMPLES 96
	float heat[SAMPLES] = {};
	float heatGain;
	float heatLoss;

	void process(const ProcessArgs &args) override {
		float cvIn = RSclamp(inputs[CV_INPUT].getVoltage(), -10.f, 10.f);

		heatGain = params[GAIN_KNOB].getValue();
		heatLoss = params[LOSS_KNOB].getValue();

		int heatIdx = (int)RSscale(cvIn, -10.f, +10.f, 0, SAMPLES - 1);
		heat[heatIdx] += heatGain;
		if(heat[heatIdx] > 1.f) heat[heatIdx] = 1.f;

		for(int i = 0; i < SAMPLES; i++) {
			heat[i] -= heatLoss;
			if(heat[i] < 0.f) heat[i] = 0.f;
		}

		if(resetTrigger.process(params[RESET_BUTTON].getValue())) {
			onReset();
		}
	}

	void onReset() override {
		std::memset(heat, 0, sizeof(heat));
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();

		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {

	}
};


struct RSCVHeatDisplay : TransparentWidget {
	RSCVHeat* module;
	float *buffer;

	RSCVHeatDisplay(RSCVHeat* module, float buffer[], int x, int y, int xs, int ys) {
		this->module = module;
		this->buffer = buffer;

		box.pos = Vec(x, y);
		box.size = Vec(xs, ys);
	};

	void draw(const DrawArgs& args) override {

		// Bounding box
		nvgStrokeColor(args.vg, COLOR_RS_BRONZE);
		nvgFillColor(args.vg, COLOR_BLACK);
		nvgStrokeWidth(args.vg, 1.5f);

		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, box.pos.x, box.pos.y, box.size.x, box.size.y, 5);
		nvgStroke(args.vg);
		nvgFill(args.vg);

		if(!module) {
			return;
		}

		// Rounded edges needed here
		for(int i = 2; i < box.size.y - 1; i++) {
			nvgStrokeColor(args.vg, nvgHSL(0, 1, buffer[SAMPLES - (int)(SAMPLES / box.size.y * i)]));
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, box.pos.x + 2, box.pos.y + i);
			nvgLineTo(args.vg, box.pos.x + box.size.x - 2, box.pos.y + i);
			nvgStroke(args.vg);
		}
	};
};


struct RSCVHeatWidget : ModuleWidget {
	RSCVHeat* module;

	RSCVHeatWidget(RSCVHeat *module) {
		setModule(module);
		this->module = module;

		box.size.x = mm2px(5.08 * 3);
		int middle = box.size.x / 2 + 1;

		addChild(new RSLabelCentered(middle, box.pos.y + 13, "CVHEAT", 14));

		addChild(new RSLabelCentered(middle, box.size.y - 15, "Racket", 12));
		addChild(new RSLabelCentered(middle, box.size.y - 4, "Science", 12));

		addParam(createParamCentered<RSKnobSml>(Vec(middle, 36), module, RSCVHeat::GAIN_KNOB));
		addChild(new RSLabelCentered(middle, 60, "GAIN"));

		addParam(createParamCentered<RSKnobSml>(Vec(middle, 78), module, RSCVHeat::LOSS_KNOB));
		addChild(new RSLabelCentered(middle, 102, "LOSS"));

		addParam(createParamCentered<RSButtonMomentary>(Vec(middle, 118), module, RSCVHeat::RESET_BUTTON));
		addChild(new RSLabelCentered(middle, 121, "RESET"));

		// Top label & invisibutton for top scale

		// CV heat display
		addChild(new RSCVHeatDisplay(module, module->heat, 3, 72, box.size.x - 11, 168));

		// Bottom label & invisibutton for bottom scale

		addInput(createInputCentered<RSJackMonoIn>(Vec(middle, 328), module, RSCVHeat::CV_INPUT));
		addChild(new RSLabelCentered(middle, 350, "CV"));

	}

    void customDraw(const DrawArgs& args) {}
	#include "RSModuleWidgetDraw.hpp"

	void step() override {
		if(!module) return;

		ModuleWidget::step();
	}
};

Model *modelRSCVHeat = createModel<RSCVHeat, RSCVHeatWidget>("RSCVHeat");
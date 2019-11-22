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
		EVIL_OUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	RSMFH() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs &args) override {
		outputs[MINF_OUT].setVoltage(-INFINITY);
		outputs[PINF_OUT].setVoltage(INFINITY);
		outputs[NAN_OUT].setVoltage(NAN);

		outputs[EVIL_OUT].setVoltage(666.666f);
		switch(rand() % 5) {
			case 0:		outputs[EVIL_OUT].setVoltage(-INFINITY); 	break;
			case 1:		outputs[EVIL_OUT].setVoltage(INFINITY);		break;
			case 2:		outputs[EVIL_OUT].setVoltage(-666.666f);	break;
			case 3:		outputs[EVIL_OUT].setVoltage(666.666f);		break;
			default:	outputs[EVIL_OUT].setVoltage(NAN); 			break;
		}
	}
};


struct RSMFHWidget : ModuleWidget {
	RSMFH* module;

	int theme;

	RSMFHWidget(RSMFH *module) {
		setModule(module);
		this->module = module;

		box.size.x = mm2px(5.08 * 3);
		int middle = box.size.x / 2;

        theme = loadDefaultTheme();

        addChild(new RSLabelCentered(middle, box.pos.y + 14, "MODULE", 15));
        addChild(new RSLabelCentered(middle, box.pos.y + 26, "FROM", 15));
        addChild(new RSLabelCentered(middle, box.pos.y + 38, "HELL", 15));

        addChild(new RSLabelCentered(middle, box.size.y - 15, "Racket", 12));
        addChild(new RSLabelCentered(middle, box.size.y - 4, "Science", 12));

		addOutput(createOutputCentered<RSJackMonoOut>(Vec(23, 72), module, RSMFH::MINF_OUT));
		addChild(new RSLabelCentered(middle, 94, "-INF"));

		addOutput(createOutputCentered<RSJackMonoOut>(Vec(23, 112), module, RSMFH::PINF_OUT));
		addChild(new RSLabelCentered(middle, 134, "+INF"));

		addOutput(createOutputCentered<RSJackMonoOut>(Vec(23, 152), module, RSMFH::NAN_OUT));
		addChild(new RSLabelCentered(middle, 174, "NAN"));

		addOutput(createOutputCentered<RSJackMonoOut>(Vec(23, 252), module, RSMFH::EVIL_OUT));
		addChild(new RSLabelCentered(middle, 278, "!EVIL!", 16, COLOR_RED));

	}

    void draw(const DrawArgs& args) override {
		nvgStrokeColor(args.vg, COLOR_RS_BRONZE);
		switch(theme) {
            case 0: nvgFillColor(args.vg, COLOR_RS_BG); break;
            case 1: nvgFillColor(args.vg, COLOR_YELLOW); break;
			default: nvgFillColor(args.vg, COLOR_BLACK); break;
        }
		nvgStrokeWidth(args.vg, 3);

		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, 0, 0, box.size.x, box.size.y, 5);
		//nvgStroke(args.vg);
		nvgFill(args.vg);

        ModuleWidget::draw(args);
    }

	void step() override {
		if(!module) return;

		ModuleWidget::step();
	}
};

Model *modelRSMFH = createModel<RSMFH, RSMFHWidget>("RSMFH");
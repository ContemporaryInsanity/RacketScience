#include "plugin.hpp"

#include "components/RSComponents.hpp"
#include "RSUtils.hpp"


struct RSBoogieBayH8 : Module {
	enum ParamIds {
		ENUMS(LEFT_SCALE_BUTTONS, 8),
		ENUMS(RIGHT_SCALE_BUTTONS, 8),
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

	dsp::ClockDivider scaleDivider;
	dsp::BooleanTrigger themeTrigger;
	dsp::BooleanTrigger leftScaleTrigger[8];
	dsp::BooleanTrigger rightScaleTrigger[8];

	RSScribbleStrip *ss[8];

	bool scaleChanged = true;
	RSLabel *leftScaleLabels[8]; // Move to widget
	int leftScale[8] = {};
	RSLabel *rightScaleLabels[8]; // Move to widget
	int rightScale[8] = {};


	RSBoogieBayH8() {
		scaleDivider.setDivision(4096);

		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		for(int i = 0; i < 8; i++) {
			configParam(LEFT_SCALE_BUTTONS + i, 0.f, 1.f, 0.f, "SCALE");
			configParam(RIGHT_SCALE_BUTTONS + i, 0.f, 1.f, 0.f, "SCALE");
			leftScale[i] = 3; rightScale[i] = 2;
		}
	}

	void process(const ProcessArgs &args) override {
		outputs[POLY_LEFT_OUTPUT].setChannels(8);
		outputs[POLY_RIGHT_OUTPUT].setChannels(8);

		if(scaleDivider.process()) {
			for(int i = 0; i < 8; i++) {
				if(leftScaleTrigger[i].process(params[LEFT_SCALE_BUTTONS + i].getValue())) {
					++leftScale[i]; if(leftScale[i] > 4) leftScale[i] = 0;
					scaleChanged = true;
				}
				if(rightScaleTrigger[i].process(params[RIGHT_SCALE_BUTTONS + i].getValue())) {
					++rightScale[i]; if(rightScale[i] > 3) rightScale[i] = 0;
					scaleChanged = true;
				}
			}
		}

		for(int i = 0; i < 8; i++) {
			float inv = inputs[INPUTS + i].getVoltage();
			outputs[LEFT_OUTPUTS + i].setVoltage(inv);
			outputs[RIGHT_OUTPUTS + i].setVoltage(inv);
			outputs[POLY_LEFT_OUTPUT].setVoltage(inv, i);
			outputs[POLY_RIGHT_OUTPUT].setVoltage(inv, i);
		}
	}

	void onReset() override {
		for(int i = 0; i < 8; i++) {
			leftScale[i] = 3; rightScale[i] = 2;
			ss[i]->text = "_";
		}
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		char ssn[4], lsn[4], rsn[4];

		for(int i = 0; i < 8; i++) {
			// Scribble strips
			json_t* ssj = json_string(ss[i]->text.c_str());
			sprintf(ssn, "SS%i", i);
			json_object_set_new(rootJ, ssn, ssj);

			// Left scales
			json_t* lsj = json_string(leftScaleLabels[i]->text.c_str());
			sprintf(lsn, "LS%i", i);
			json_object_set_new(rootJ, lsn, lsj);

			// Right scales
			json_t* rsj = json_string(rightScaleLabels[i]->text.c_str());
			sprintf(rsn, "RS%i", i);
			json_object_set_new(rootJ, rsn, rsj);
		}

		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		char ssn[4], lsn[4], rsn[4];

		for(int i = 0; i < 8; i++) {
			// Scribble strips
			sprintf(ssn, "SS%i", i);
			json_t* ssj = json_object_get(rootJ, ssn);
			if(ssj) ss[i]->text = json_string_value(ssj);

			// Left scales
			sprintf(lsn, "LS%i", i);
			json_t* lsj = json_object_get(rootJ, lsn);
			if(lsj) {
				int ls = std::stoi(json_string_value(lsj));
				//INFO("Racket Science: leftScale  %i %i", i, ls);

				// SIMPLIFY!!! leftScale / rightScale will be -10 to + 10, scaleLabels can be created at run time

				switch(ls) {
					case 0: leftScale[i] = 0; leftScaleLabels[i]->text = "0"; break;
					case 1: leftScale[i] = 1; leftScaleLabels[i]->text = "1"; break;
					case 2: leftScale[i] = 2; leftScaleLabels[i]->text = "2"; break;
					case 5: leftScale[i] = 3; leftScaleLabels[i]->text = "5"; break;
					case 10: leftScale[i] = 4; leftScaleLabels[i]->text = "10"; break;
				}
			}

			// Right scales
			sprintf(rsn, "RS%i", i);
			json_t* rsj = json_object_get(rootJ, rsn);
			if(rsj) {
				int rs = std::stoi(json_string_value(rsj));
				//INFO("Racket Science: rightScale %i %i", i, rs);
				switch(rs) {
					case 1: rightScale[i] = 0; rightScaleLabels[i]->text = "1"; break;
					case 2: rightScale[i] = 1; rightScaleLabels[i]->text = "2"; break;
					case 5: rightScale[i] = 2; rightScaleLabels[i]->text = "5"; break;
					case 10: rightScale[i] = 3; rightScaleLabels[i]->text = "10"; break;
				}
			}
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
		middle = box.size.x / 2;
		left = 40;
		right = box.size.y - 40;

		addChild(new RSLabelCentered(middle, box.pos.y + 13, "BOOGIE BAY H8", 14));
		addChild(new RSLabelCentered(middle, box.size.y - 4, "Racket Science", 12));

		for(int i = 0; i < 8; i++) {
			in[i] = createInputCentered<RSJackMonoIn>(Vec(middle, 40 + (i * 40)), module, RSBoogieBayH8::INPUTS + i); addInput(in[i]);
			addOutput(createOutputCentered<RSJackMonoOut>(Vec(left, 40 + (i * 40)), module, RSBoogieBayH8::LEFT_OUTPUTS + i));
			addOutput(createOutputCentered<RSJackMonoOut>(Vec(right, 40 + (i * 40)), module, RSBoogieBayH8::RIGHT_OUTPUTS + i));
			addParam(createParamCentered<RSButtonMomentaryInvisible>(Vec(left + 20, 40 + (i * 40)), module, RSBoogieBayH8::LEFT_SCALE_BUTTONS + i));
			addParam(createParamCentered<RSButtonMomentaryInvisible>(Vec(right - 20, 40 + (i * 40)), module, RSBoogieBayH8::RIGHT_SCALE_BUTTONS + i));
			if(module) {
				addChild(module->ss[i] = new RSScribbleStrip(left + 25, 25 + (i * 40)));
				addChild(module->leftScaleLabels[i] = new RSLabel(left + 16, 43 + (i * 40), "10", 10, COLOR_RED));
				addChild(module->rightScaleLabels[i] = new RSLabel(right - 24, 43 + (i * 40), "10", 10, COLOR_GREEN));
			}
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

		static float ls = 0, rs = 0;

		for(int i = 0; i < 8; i++) {
			module->leftScaleLabels[i]->color = COLOR_RED;
			switch(module->leftScale[i]) {
				case 0: module->leftScaleLabels[i]->text = "0"; ls = 0; module->leftScaleLabels[i]->color = COLOR_RS_GREY; break;
				case 1: module->leftScaleLabels[i]->text = "1"; ls = -1; break;
				case 2: module->leftScaleLabels[i]->text = "2"; ls = -2; break;
				case 3: module->leftScaleLabels[i]->text = "5"; ls = -5; break;
				case 4: module->leftScaleLabels[i]->text = "10"; ls = -10; break;
			}
			switch(module->rightScale[i]) {
				case 0: module->rightScaleLabels[i]->text = "1"; rs = 1; break;
				case 1: module->rightScaleLabels[i]->text = "2"; rs = 2; break;
				case 2: module->rightScaleLabels[i]->text = "5"; rs = 5; break;
				case 3: module->rightScaleLabels[i]->text = "10"; rs = 10; break;
			}
			float inv = RSclamp(module->inputs[RSBoogieBayH8::INPUTS + i].getVoltage(), ls, rs);
			in[i]->box.pos.x = RSscale(inv, ls, rs, 65, 290);
		}

		ModuleWidget::step();
	}
};

Model *modelRSBoogieBayH8 = createModel<RSBoogieBayH8, RSBoogieBayH8Widget>("RSBoogieBayH8");
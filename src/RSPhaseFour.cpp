#include "plugin.hpp"

#include "RS.hpp"

struct RSPhaseFour : RSModule {
	static const int patterns = 16;
	static const int samples = 1000;
	static const int rows = 4;

	enum ParamIds {
		THEME_BUTTON,

		PATTERN_KNOB,
		COPY_PATTERN_BUTTON,
		PASTE_PATTERN_BUTTON,

		NEXT_PATTERN_BUTTON,
		RAND_PATTERN_BUTTON,
		PREV_PATTERN_BUTTON,

		ENUMS(SCRUB_KNOBS, rows),

		ENUMS(ENABLE_BUTTONS, rows),
		ENUMS(WRITE_BUTTONS, rows),

		ENUMS(CLEAR_BUTTONS, rows),
		ENUMS(RAND_BUTTONS, rows),

		ENUMS(DIVIDE_KNOBS, rows),
		ENUMS(SELECT_KNOBS, rows),
		ENUMS(ADJUST_KNOBS, rows),
		ENUMS(MOVE_KNOBS, rows),

		ENUMS(BAKE_BUTTONS, rows),

		NUM_PARAMS
	};
	enum InputIds {
		NEXT_PATTERN_IN,
		RAND_PATTERN_IN,
		PREV_PATTERN_IN,

		ENUMS(PHASE_INS, rows),

		ENUMS(WRITE_INS, rows),
		ENUMS(CV_INS, rows),

		ENUMS(CLEAR_INS, rows),
		ENUMS(RAND_INS, rows),

		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(REC_CV_OUTS, rows),
		ENUMS(OVL_CV_OUTS, rows),

		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};


	dsp::BooleanTrigger themeTrigger;

	dsp::BooleanTrigger clearTrigger[rows];
	dsp::BooleanTrigger randTrigger[rows];
	dsp::BooleanTrigger bakeTrigger[rows];

	RSScribbleStrip *ss = NULL;

	int pattern;

	float recBuffer[rows][samples] = {}; unsigned int recIdx[rows];
	float ovlBuffer[rows][samples] = {}; unsigned int ovlIdx[rows];
	bool enable[rows], write[rows];

	float phaseIn[rows], priorPhaseIn[rows];
	unsigned int divide[rows], priorDivide[rows];


	RSPhaseFour() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(THEME_BUTTON, 0.f, 1.f, 0.f, "THEME");

		configParam(PATTERN_KNOB, 0.f, (float) patterns - 1, 0.f, "PATTERN SELECT");
        configParam(COPY_PATTERN_BUTTON, 0.f, 1.f, 0.f, "COPY PATTERN");
        configParam(PASTE_PATTERN_BUTTON, 0.f, 1.f, 0.f, "PASTE PATTERN");

        configParam(PREV_PATTERN_BUTTON, 0.f, 1.f, 0.f, "PREV PATTERN");
        configParam(RAND_PATTERN_BUTTON, 0.f, 1.f, 0.f, "RAND PATTERN");
        configParam(NEXT_PATTERN_BUTTON, 0.f, 1.f, 0.f, "NEXT PATTERN");

		for(int row = 0; row < rows; row++) {
			configParam(SCRUB_KNOBS + row, -INFINITY, INFINITY, 0.f, "SCRUB");
			
			configParam(ENABLE_BUTTONS + row, 0.f, 1.f, 1.f, "WRITE ENABLE");
			configParam(WRITE_BUTTONS + row, 0.f, 1.f, 0.f, "WRITE");

			configParam(CLEAR_BUTTONS + row, 0.f, 1.f, 0.f, "CLEAR");
			configParam(RAND_BUTTONS + row, 0.f, 1.f, 0.f, "RANDOMIZE");

			configParam(DIVIDE_KNOBS + row, 1.f, 64.f, 1.f, "DIVIDE");
			configParam(SELECT_KNOBS + row, 1.f, 64.f, 1.f, "SELECT");
			configParam(ADJUST_KNOBS + row, -10.f, 10.f, 0.f, "ADJUST");
			configParam(MOVE_KNOBS + row, -INFINITY, INFINITY, 0.f, "MOVE");
			
			recIdx[row] = 0; ovlIdx[row] = 0;
			enable[row] = false; write[row] = false;
			phaseIn[row] = 0.f; priorPhaseIn[row] = 0.f;
			divide[row] = 0; priorDivide[row] = 0;
		}
	}

	void process(const ProcessArgs &args) override {
		if(themeTrigger.process(params[THEME_BUTTON].getValue())) {
			RSTheme++;
			if(RSTheme > RSGlobal.themeCount) RSTheme = 1;
		}


		for(int row = 0; row < rows; row++) {
			// Get idx from phase in or scrub knob
			if(inputs[PHASE_INS + row].isConnected()) {
				phaseIn[row] = RSclamp(inputs[PHASE_INS + row].getVoltage(), 0.f, 10.f);
				recIdx[row] = phaseIn[row] * samples / 10.f;
				params[SCRUB_KNOBS + row].setValue(phaseIn[row] / 4.15);
			}
			else {
				phaseIn[row] = params[SCRUB_KNOBS + row].getValue();
				recIdx[row] = std::fmod(abs(phaseIn[row] / 2.41), 1.f) * samples;
				if(phaseIn[row] < 0.f) recIdx[row] = samples - recIdx[row]; // So negative phase doesn't reverse head direction
			}
			if(recIdx[row] >= samples) {
				INFO("Racket Science: overrun idx=%i", recIdx[row]);
				recIdx[row] = samples - 1; // Just in case
			}

			// Get CV in
			float cvIn = RSclamp(inputs[CV_INS + row].getVoltage(), -10.f, 10.f);

			// Get write from write in or button if enabled
			write[row] = false;
			if(params[ENABLE_BUTTONS + row].getValue()) {
				if(inputs[WRITE_INS + row].isConnected()) write[row] = RSclamp(inputs[WRITE_INS + row].getVoltage(), 0.f, 1.f) ? true : false;
				else                                      write[row] = params[WRITE_BUTTONS + row].getValue() ? true : false;
				if(write[row]) {
					recBuffer[row][recIdx[row]] = cvIn;
					// Calc / recalc min / max here
				}
			}

			// Should do following in widget step()
			divide[row] = (int)params[DIVIDE_KNOBS + row].getValue();
			if(divide[row] != priorDivide[row]) updateOverlay(row);
			priorDivide[row] = divide[row]; // Else we eat CPU

			if(clearTrigger[row].process(params[CLEAR_BUTTONS + row].getValue() > 0.f)) onClear(row);
			if(randTrigger[row].process(params[RAND_BUTTONS + row].getValue() > 0.f)) onRand(row);

			// Bake
			if(bakeTrigger[row].process(params[BAKE_BUTTONS + row].getValue() > 0.f)) onBake(row);


			// rec CV out
			outputs[REC_CV_OUTS + row].setVoltage(recBuffer[row][recIdx[row]]);

			// ovl CV out
			outputs[OVL_CV_OUTS + row].setVoltage(ovlBuffer[row][recIdx[row]]); // [ovlIdx[row]]

		}
	}

	void updateOverlay(int row) {
		float step = (float)samples / (float)divide[row];
		for(int i = 0; i < samples - 1; i += step) {
			for(int j = i; j < i + step; j++) {
				if(j < samples) ovlBuffer[row][j] = recBuffer[row][i];
			}
		}
	}

	void onClear(int row) {
		for(int i = 0; i < samples; i ++) {
			recBuffer[row][i] = 0.f;
			ovlBuffer[row][i] = 0.f;
		}
	}

	void onRand(int row) {
		std::random_device rd;
		std::mt19937 e2(rd());
		std::uniform_real_distribution<> dist(-10.f, 10.f);
		for(int i = 0; i < samples; i++) recBuffer[row][i] = dist(e2);
		updateOverlay(row);
	}

	void onBake(int row) {
		for(int i = 0; i < samples; i++) recBuffer[row][i] = ovlBuffer[row][i];
	}

	void onReset() override {
		for(int row = 0; row < rows; row++) onClear(row);
	}

	void onRandomize() override {
		std::random_device rd;
		std::mt19937 e2(rd());
		std::uniform_real_distribution<> dist(-10.f, 10.f);
		for(int row = 0; row < rows; row++) onRand(row);
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
        json_object_set_new(rootJ, "theme", json_integer(RSTheme));

		if(ss) {
			json_t* SS = json_string(ss->text.c_str());
			json_object_set_new(rootJ, "ss", SS);
		}

		json_t* recSamples = json_array();
		json_t* ovlSamples = json_array();

		// for(int i = 0; i < samples; i++) {
		// 	json_array_append_new(recSamples, json_real(recBuffer[i]));
		// 	json_array_append_new(ovlSamples, json_real(ovlBuffer[i]));
		// }

		json_object_set_new(rootJ, "recSamples", recSamples);
		json_object_set_new(rootJ, "ovlSamples", ovlSamples);

		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
        json_t* themeJ = json_object_get(rootJ, "theme");
        if(themeJ) RSTheme = json_integer_value(themeJ);

		json_t* SS = json_object_get(rootJ, "ss");
		if(SS) ss->text = json_string_value(SS);

		json_t* recSamples = json_object_get(rootJ, "recSamples");
		json_t* ovlSamples = json_object_get(rootJ, "ovlSamples");

		if(recSamples && ovlSamples) {
			// for(int i = 0; i < samples; i++) {
			// 	recBuffer[i] = json_number_value(json_array_get(recSamples, i));
			// 	ovlBuffer[i] = json_number_value(json_array_get(ovlSamples, i));
			// }
		}
	}
};

struct RSPhaseDisplay : TransparentWidget {
	std::shared_ptr<Font> font;
	RSPhaseFour *module;
	unsigned int row;
	unsigned int *idx;
	bool *write;

	RSPhaseDisplay(RSPhaseFour *module, int row, int x, int y, int xs, int ys) {
		this->module = module;
		this->row = row;
		this->idx = &module->recIdx[row];
		this->write = &module->write[row];

		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Ubuntu Condensed 400.ttf"));

		box.pos = Vec(x / 2, y / 2);
		box.size = Vec(xs, ys);
	};

	void draw(const DrawArgs& args) override {
		auto startTime = std::chrono::system_clock::now();

		nvgFontSize(args.vg, 10);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, 0);

		// Bounding box
		nvgStrokeColor(args.vg, COLOR_RS_BRONZE);
		nvgFillColor(args.vg, COLOR_BLACK);
		nvgStrokeWidth(args.vg, 1.5f);

		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, box.pos.x, box.pos.y, box.size.x, box.size.y, 5);
		nvgStroke(args.vg);
		nvgFill(args.vg);

		if(!module) {  // Stand out in the module browser
			nvgFontSize(args.vg, 60);
			nvgFillColor(args.vg, COLOR_RS_BRONZE);
			nvgTextAlign(args.vg, NVG_ALIGN_CENTER);

			char msg[30];
			switch(this->row) {
				case 0: {
					std::time_t t = std::time(0);
					std::tm* now = std::localtime(&t);
					switch(now->tm_wday) {
						case 0: strcpy(msg, "Happy Sunday"); break; 
						case 1: strcpy(msg, "Happy Monday"); break; 
						case 2: strcpy(msg, "Happy Tuesday"); break; 
						case 3: strcpy(msg, "Happy Wednesday"); break; 
						case 4: strcpy(msg, "Happy Thursday"); break; 
						case 5: strcpy(msg, "Happy Friday"); break; 
						case 6: strcpy(msg, "Happy Saturday"); break; 
					}
					if(now->tm_mday == 25 && now->tm_mon + 1 == 12) strcpy(msg, "MERRY XMAS!");
					break;
				}
				case 1: strcpy(msg, "R a c k e t   S c i e n c e"); break;
				case 2: strcpy(msg, "This space available for rent"); break;
				case 3: strcpy(msg, "Please consider donating :)"); break;
			}
			nvgText(args.vg, box.pos.x + box.size.x / 2, box.pos.y + 50, msg, NULL);
			return;
		}

		// Center line
		int centerLine = box.pos.y + (box.size.y / 2);
		nvgStrokeColor(args.vg, COLOR_BLUE);
		nvgBeginPath(args.vg);
		nvgMoveTo(args.vg, box.pos.x, centerLine);
		nvgLineTo(args.vg, box.pos.x + box.size.x, centerLine);
		nvgStroke(args.vg);

		// recBuffer
		nvgStrokeColor(args.vg, COLOR_GREEN);
		nvgBeginPath(args.vg);
		nvgMoveTo(args.vg, box.pos.x, centerLine - (module->recBuffer[row][0] / 20 * box.size.y));
		for(int i = 0; i < box.size.x; i++) {
			unsigned int idx = module->samples / box.size.x * i;
			int val = module->recBuffer[row][idx] / 20.f * box.size.y;
			nvgLineTo(args.vg, box.pos.x + i, centerLine - val);
		}
		nvgStroke(args.vg);

		// ovlBuffer
		nvgStrokeColor(args.vg, COLOR_RED);
		nvgBeginPath(args.vg);
		nvgMoveTo(args.vg, box.pos.x, centerLine - (module->ovlBuffer[row][0] / 20 * box.size.y));
		for(int i = 0; i < box.size.x; i++) {
			unsigned int idx = module->samples / box.size.x * i;
			int val = module->ovlBuffer[row][idx] / 20 * box.size.y;
			nvgLineTo(args.vg, box.pos.x + i, centerLine - val);
		}
		nvgStroke(args.vg);

		// Divisions
		nvgStrokeColor(args.vg, COLOR_WHITE);
		for(int i = box.size.x / module->divide[row]; i < box.size.x; i += box.size.x / module->divide[row]) {
				nvgBeginPath(args.vg);
				nvgMoveTo(args.vg, box.pos.x + i, box.pos.y);
				nvgLineTo(args.vg, box.pos.x + i, box.pos.y + box.size.y);
				nvgStroke(args.vg);
		}

		// Index
		nvgStrokeColor(args.vg, *write == true ? COLOR_RED : COLOR_RS_GREY);
		nvgStrokeWidth(args.vg, 1.f);

		nvgBeginPath(args.vg);
		nvgMoveTo(args.vg, box.pos.x + (box.size.x / module->samples * *idx), box.pos.y);
		nvgLineTo(args.vg, box.pos.x + (box.size.x / module->samples * *idx), box.pos.y + box.size.y);
		nvgStroke(args.vg);
	};
};

struct RSPhaseFourWidget : ModuleWidget {
	RSPhaseFour* module;

	int x, y, smlGap, lrgGap, labOfs;

	RSLabelCentered *patternLabel;

	RSPhaseFourWidget(RSPhaseFour *module) {
		INFO("Racket Science: RSPhaseFourWidget()");

		setModule(module);
		this->module = module;

		box.size = Vec(RACK_GRID_WIDTH * 104, RACK_GRID_HEIGHT);
		int middle = box.size.x / 2 + 1;

		addParam(createParamCentered<RSButtonMomentaryInvisible>(Vec(box.pos.x + 5, box.pos.y + 5), module, RSPhaseFour::THEME_BUTTON));

		addChild(new RSLabelCentered(middle, box.pos.y + 13, "PHASE IV", 14, module));
		addChild(new RSLabelCentered(middle, box.size.y - 4, "Racket Science", 12, module));

		x = 60; y = 50;
		smlGap = 30; lrgGap = 65;
		labOfs = 22;

		// Pattern section
		addChild(new RSLabelCentered(x, y - (labOfs * 1.5), "PATTERN", 10, module));

		// Pattern copy & paste
        addParam(createParamCentered<RSButtonMomentary>(Vec(x - 30, y - smlGap + 3), module, RSPhaseFour::COPY_PATTERN_BUTTON));
        addChild(new RSLabelCentered(x - 30, y - smlGap + 6, "COPY", 10, module));

        addParam(createParamCentered<RSButtonMomentary>(Vec(x + 30, y - smlGap + 3), module, RSPhaseFour::PASTE_PATTERN_BUTTON));
        addChild(new RSLabelCentered(x + 30, y - smlGap + 6, "PASTE", 10, module));
        y += smlGap;

        // Pattern knob
        addParam(createParamCentered<RSKnobDetentLrg>(Vec(x, y), module, RSPhaseFour::PATTERN_KNOB));
        patternLabel = new RSLabelCentered(x, y + 5, "0", 22, module);
        addChild(patternLabel);
        y += smlGap + 12;

		// More pattern stuff here


		// Left side labels

		// Skip over display

		// Right side labels

		
		x = 145; y = 55;
		for(int row = 0, rowGap = 90; row < module->rows; row++, y += rowGap) addRow(row, x, y, 70);
	}

	void addRow(int row, int x, int y, int h) {
		// Add scrub knob & phase in
		addParam(createParamCentered<RSKnobMed>(Vec(x, y), module, RSPhaseFour::SCRUB_KNOBS + row));
		addInput(createInputCentered<RSStealthJackSmallMonoIn>(Vec(x, y), module, RSPhaseFour::PHASE_INS + row));
		x += lrgGap;

		// Add CV in
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y), module, RSPhaseFour::CV_INS + row));
		addChild(new RSLabelCentered(x, y + labOfs, "CV IN", 10, module));
		x += smlGap;

		// Add write enable, button & in
		y -= 18;
		addParam(createParamCentered<RSButtonToggle>(Vec(x, y), module, RSPhaseFour::ENABLE_BUTTONS + row));
		addChild(new RSLabelCentered(x, y + 3, "ENABLE", 9, module));
		y += 30;
		addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y), module, RSPhaseFour::WRITE_BUTTONS + row));
		addInput(createInputCentered<RSStealthJackMonoIn>(Vec(x, y), module, RSPhaseFour::WRITE_INS + row));
		addChild(new RSLabelCentered(x, y + labOfs, "WRITE"));
		x += smlGap;
		y -= 30;

		// Add clear & randomize
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSPhaseFour::CLEAR_BUTTONS + row));
		addChild(new RSLabelCentered(x, y - 14, "CLEAR", 10, module));
		y += 30;
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSPhaseFour::RAND_BUTTONS + row));
		addChild(new RSLabelCentered(x, y + 22, "RAND", 10, module));
		x += smlGap;
		y -= 30;

		// Add divide, select, adjust & move knobs
		addParam(createParamCentered<RSKnobDetentSml>(Vec(x, y), module, RSPhaseFour::DIVIDE_KNOBS + row));
		addChild(new RSLabelCentered(x, y - 14, "DIVIDE", 10, module));
		y += 30;
		addParam(createParamCentered<RSKnobDetentSml>(Vec(x, y), module, RSPhaseFour::ADJUST_KNOBS + row));
		addChild(new RSLabelCentered(x, y + 22, "ADJUST", 10, module));
		x += smlGap;
		y -= 30;
		addParam(createParamCentered<RSKnobDetentSml>(Vec(x, y), module, RSPhaseFour::SELECT_KNOBS + row));
		addChild(new RSLabelCentered(x, y - 14, "SELECT", 10, module));
		y += 30;
		addParam(createParamCentered<RSKnobDetentSml>(Vec(x, y), module, RSPhaseFour::MOVE_KNOBS + row));
		addChild(new RSLabelCentered(x, y + 22, "MOVE", 10, module));
		x += smlGap;

		// Add bake buttons
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSPhaseFour::BAKE_BUTTONS + row));
		addChild(new RSLabelCentered(x, y -14, "BAKE", 10, module));
		x += smlGap;

		//x = 370; // So we line up with Fido3 steps
		addChild(new RSPhaseDisplay(module, row, x, y - 45, 1040, h));
		x += 1040 + smlGap;
		y -= smlGap;

		// rec CV out
		addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y), module, RSPhaseFour::REC_CV_OUTS + row));
		y += smlGap;

		// ovl CV out
		addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y), module, RSPhaseFour::OVL_CV_OUTS + row));
	}

	void step() override {
		if(!module) return;

		ModuleWidget::step();
	}

    void customDraw(const DrawArgs& args) {}
	#include "RSModuleWidgetDraw.hpp"
};

Model *modelRSPhaseFour = createModel<RSPhaseFour, RSPhaseFourWidget>("RSPhaseFour");
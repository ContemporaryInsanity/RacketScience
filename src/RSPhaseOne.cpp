#include "plugin.hpp"

#include "RS.hpp"

struct RSPhaseOne : RSModule {
	enum ParamIds {
		THEME_BUTTON,
		SCRUB_PARAM,
		ENABLE_PARAM,
		WRITE_PARAM,
		IN_PARAM,
		CLEAR_PARAM,
		RAND_PARAM,
		DIVIDE_PARAM,
		SELECT_PARAM,
		ADJUST_PARAM,
		BAKE_PARAM,
		NEXT_PARAM,
		PREV_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		PHASE_INPUT,
		WRITE_INPUT,
		IN_INPUT,
		CLEAR_INPUT,
		RAND_INPUT,
		DIVIDE_INPUT,
		SELECT_INPUT,
		ADJUST_INPUT,
		BAKE_INPUT,
		NEXT_INPUT,
		PREV_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		REC_OUTPUT,
		OVL_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	dsp::BooleanTrigger themeTrigger;

	dsp::BooleanTrigger enableTrigger;
	dsp::BooleanTrigger clearTrigger;
	dsp::BooleanTrigger randTrigger;
	dsp::BooleanTrigger bakeTrigger;

	RSScribbleStrip *ss = NULL;

	static const unsigned int SAMPLES = 1000;
	float recBuffer[SAMPLES] = {}; unsigned int recIdx = 0;
	float ovlBuffer[SAMPLES] = {}; unsigned int ovlIdx = 0;
	bool enable = true, write = false;

	float phaseIn, priorPhaseIn;
	unsigned int divide, priorDivide;

	RSPhaseOne() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(THEME_BUTTON, 0.f, 1.f, 0.f, "THEME");

		configParam(SCRUB_PARAM, -INFINITY, INFINITY, 0.f, "SCRUB");
		configParam(ENABLE_PARAM, 0.f, 1.f, 1.f, "ENABLE");
		configParam(WRITE_PARAM, 0.f, 1.f, 0.f, "WRITE");
		configParam(IN_PARAM, -10.f, 10.f, 0.f, "IN");
		configParam(CLEAR_PARAM, 0.f, 1.f, 0.f, "CLEAR");
		configParam(RAND_PARAM, 0.f, 1.f, 0.f, "RAND");
		configParam(DIVIDE_PARAM, 1.f, 128.f, 1.f, "DIVIDE");
		configParam(SELECT_PARAM, 1.f, 128.f, 1.f, "SELECT");
		configParam(ADJUST_PARAM, -10.f, 10.f, 0.f, "ADJUST");
		configParam(BAKE_PARAM, 0.f, 1.f, 0.f, "BAKE");

		priorDivide = divide = 1;
	}

	void process(const ProcessArgs &args) override {
		if(themeTrigger.process(params[THEME_BUTTON].getValue())) {
			RSTheme++;
			if(RSTheme > RSGlobal.themeCount) RSTheme = 1;
		}

		enable = enableTrigger.process(params[ENABLE_PARAM].getValue()) > 0.f ? true : false;

		if(inputs[CLEAR_INPUT].isConnected()) {
			if(clearTrigger.process(inputs[CLEAR_INPUT].getVoltage() > 0.f)) onReset();
		}
		else {
			if(clearTrigger.process(params[CLEAR_PARAM].getValue() > 0.f)) onReset();
		}

		if(inputs[RAND_INPUT].isConnected()) {
			if(randTrigger.process(inputs[RAND_INPUT].getVoltage() > 0.f)) onRandomize();
		}
		else {
			if(randTrigger.process(params[RAND_PARAM].getValue() > 0.f)) onRandomize();
		}

		if(inputs[BAKE_INPUT].isConnected()) {
			if(bakeTrigger.process(inputs[BAKE_INPUT].getVoltage() > 0.f)) onBake();
		}
		else {
			if(bakeTrigger.process(params[BAKE_PARAM].getValue() > 0.f)) onBake();
		}

		float in;
		unsigned int select;
		float adjust;

		// Get buffer idx from phase input or scrub knob
		if(inputs[PHASE_INPUT].isConnected()) {
			phaseIn = RSclamp(inputs[PHASE_INPUT].getVoltage(), 0.f, 10.f);
			recIdx = phaseIn * SAMPLES / 10;
			params[SCRUB_PARAM].setValue(phaseIn / 4.15);
		}
		else {
			phaseIn = params[SCRUB_PARAM].getValue();
			recIdx = std::fmod(abs(phaseIn / 2.41), 1.f) * SAMPLES;
			if(phaseIn < 0.f) recIdx = SAMPLES - recIdx; // So negative phase doesn't reverse head direction
		}

		if(recIdx >= SAMPLES) recIdx = SAMPLES - 1;

		// Get input value from in or in knob
		if(inputs[IN_INPUT].isConnected()) {
			in = RSclamp(inputs[IN_INPUT].getVoltage(), -10.f, 10.f);
			params[IN_PARAM].setValue(in);
		}
		else in = params[IN_PARAM].getValue();

		// Get write from write in or write button if enabled
		if(params[ENABLE_PARAM].getValue() > 0.f) {
			if(inputs[WRITE_INPUT].isConnected()) write = RSclamp(inputs[WRITE_INPUT].getVoltage(), 0.f, 1.f) > 0.f ? true : false;
			else								  write = params[WRITE_PARAM].getValue() > 0.f ? true : false;
			if(write) recBuffer[recIdx] = in;
		}

		// Get divide
		if(inputs[DIVIDE_INPUT].isConnected()) {
			divide = RSclamp(inputs[DIVIDE_INPUT].getVoltage(), 0.f, 10.f);
			// Now scale divide to match divide knob
			// And set knob accordingly
		}
		else {
			divide = (unsigned int)params[DIVIDE_PARAM].getValue();
			if(divide != priorDivide) {
				updateOverlay();
				INFO("Racket Science: divide %i", divide);
			}
			priorDivide = divide;
		}

		// Get select


		// Get adjust



		//if(phaseIn < priorPhaseIn) INFO("Racket Science: EOC");

		// MAIN CV OUT
		outputs[REC_OUTPUT].setVoltage(recBuffer[recIdx]);

		// OVERLAY CV OUT
		outputs[OVL_OUTPUT].setVoltage(ovlBuffer[recIdx]);

		priorPhaseIn = phaseIn;
	}

	void updateOverlay() {
		float step = (float)SAMPLES / (float)divide;
		for(unsigned int i = 0; i < SAMPLES - 1; i += step) {
			//INFO("Racket Science: i %i", i);
			for(unsigned int j = i; j < i + step; j++) {
				if(j < SAMPLES) ovlBuffer[j] = recBuffer[i];
				//else INFO("Racket Science: j >= SAMPLES");
			}
		}
	}

	void onReset() override {
		std::memset(recBuffer, 0, sizeof(recBuffer));
	}

	void onRandomize() override {
		std::random_device rd;
		std::mt19937 e2(rd());
		std::uniform_real_distribution<> dist(-10.f, 10.f);
		for(int i = 0; i < SAMPLES; i++) recBuffer[i] = dist(e2);
		updateOverlay();
	}

	void onBake() {
		std::memcpy(recBuffer, ovlBuffer, sizeof(recBuffer));
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

		for(int i = 0; i < SAMPLES; i++) {
			json_array_append_new(recSamples, json_real(recBuffer[i]));
			json_array_append_new(ovlSamples, json_real(ovlBuffer[i]));
		}

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
			for(int i = 0; i < SAMPLES; i++) {
				recBuffer[i] = json_number_value(json_array_get(recSamples, i));
				ovlBuffer[i] = json_number_value(json_array_get(ovlSamples, i));
			}
		}
	}
};


struct RSPhaseOneDisplay : TransparentWidget {
	std::shared_ptr<Font> font;
	RSPhaseOne* module;
	//float *buffer;
	unsigned int *idx;
	bool *write;

	RSPhaseOneDisplay(RSPhaseOne* module, int x, int y, int xs, int ys) {
		this->module = module;
		//this->buffer = module->recBuffer;
		this->idx = &module->recIdx;
		this->write = &module->write;

		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Ubuntu Condensed 400.ttf"));

		box.pos = Vec(x, y);
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
		nvgStrokeWidth(args.vg, 1.5);

		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, box.pos.x, box.pos.y, box.size.x, box.size.y, 5);
		nvgStroke(args.vg);
		nvgFill(args.vg);

		if(!module) { // Stand out in the module browser
			nvgFontSize(args.vg, 60);
			nvgFillColor(args.vg, COLOR_RS_BRONZE);
			nvgTextAlign(args.vg, NVG_ALIGN_CENTER);

			char msg[20];
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
			nvgText(args.vg, box.pos.x + box.size.x / 2, box.pos.y + 60, msg, NULL);

			nvgText(args.vg, box.pos.x + box.size.x / 2, box.pos.y + 130, "R a c k e t   S c i e n c e", NULL);
		
			nvgFontSize(args.vg, 25);
			nvgText(args.vg, box.pos.x + box.size.x / 2, box.pos.y + 180, "Please consider donating : )", NULL);
			nvgStroke(args.vg);
			
			return;
		}

		// Centre line
		int centerLine = box.pos.y + (box.size.y / 2);
		nvgStrokeColor(args.vg, COLOR_BLUE);
		nvgStrokeWidth(args.vg, 2);
		nvgBeginPath(args.vg);
		nvgMoveTo(args.vg, box.pos.x, centerLine);
		nvgLineTo(args.vg, box.pos.x + box.size.x, centerLine);
		nvgStroke(args.vg);

		// Get max / min
		float min = 0.f;
		float max = 0.f;
		float range = 0.f;
		for(int i = 0; i < module->SAMPLES; i++) {
			if(module->recBuffer[i] > max) max = module->recBuffer[i];
			if(module->recBuffer[i] < min) min = module->recBuffer[i];
		}
		
		nvgStrokeWidth(args.vg, 1);

		// Draw recBuffer full scale, will have autoscale option soon
		nvgStrokeColor(args.vg, COLOR_GREEN);
		nvgBeginPath(args.vg);
		nvgMoveTo(args.vg, box.pos.x, centerLine - (module->recBuffer[0] / 20 * box.size.y));
		for(int i = 0; i < box.size.x; i++) {
			unsigned int idx = module->SAMPLES / box.size.x * i;
			int val = module->recBuffer[idx] / 20 * box.size.y;
			nvgLineTo(args.vg, box.pos.x + i, centerLine - val);
		}
		nvgStroke(args.vg);

		// Draw overlay buffer
		nvgStrokeColor(args.vg, COLOR_RED);
		nvgBeginPath(args.vg);
		nvgMoveTo(args.vg, box.pos.x, centerLine - (module->ovlBuffer[0] / 20 * box.size.y));
		for(int i = 0; i < box.size.x; i++) {
			unsigned int idx = module->SAMPLES / box.size.x * i;
			int val = module->ovlBuffer[idx] / 20 * box.size.y;
			nvgLineTo(args.vg, box.pos.x + i, centerLine - val);
		}
		nvgStroke(args.vg);

		// Scale indicators
		if(font->handle >= 0) {
			bndSetFont(font->handle);

			nvgFontSize(args.vg, 10);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, 0);
			nvgTextAlign(args.vg, NVG_ALIGN_LEFT);

			char str[6] = "SCALE";
			//sprintf(str, "%2.2f", range);

			nvgBeginPath(args.vg);
			nvgFillColor(args.vg, COLOR_GREEN);
			nvgText(args.vg, box.pos.x + 2, box.pos.y + 10, str, NULL);
			nvgStroke(args.vg);

			nvgBeginPath(args.vg);
			nvgFillColor(args.vg, COLOR_RED);
			nvgText(args.vg, box.pos.x + 2, box.pos.y + box.size.y - 4, str, NULL);
			nvgStroke(args.vg);

			bndSetFont(APP->window->uiFont->handle);
		}

		// Divisions
		nvgStrokeColor(args.vg, COLOR_WHITE);
		for(int i = 0; i < box.size.x; i += box.size.x / module->divide) {
				nvgBeginPath(args.vg);
				nvgMoveTo(args.vg, box.pos.x + i, box.pos.y);
				nvgLineTo(args.vg, box.pos.x + i, box.pos.y + box.size.y);
				nvgStroke(args.vg);
		}

		// Index
		nvgStrokeColor(args.vg, *write == true ? COLOR_RED : COLOR_RS_GREY);
		nvgStrokeWidth(args.vg, 2);
		nvgBeginPath(args.vg);

		nvgMoveTo(args.vg, box.pos.x + (box.size.x / module->SAMPLES * *idx), box.pos.y);
		nvgLineTo(args.vg, box.pos.x + (box.size.x / module->SAMPLES * *idx), box.pos.y + box.size.y);

		nvgStroke(args.vg);

		auto endTime = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsedTime = endTime - startTime;
		//INFO("Racket Science: RSScratch buffer draw elapsed time: %f", elapsedTime.count());
	};
};


struct RSPhaseOneWidget : ModuleWidget {
	RSPhaseOne* module;

	RSScribbleStrip *ss;

	RSPhaseOneWidget(RSPhaseOne *module) {
		INFO("Racket Science: RSPhaseOneWidget()");

		setModule(module);
		this->module = module;

		box.size = Vec(RACK_GRID_WIDTH * 75, RACK_GRID_HEIGHT);
		int middle = box.size.x / 2 + 1;

		addParam(createParamCentered<RSButtonMomentaryInvisible>(Vec(box.pos.x + 5, box.pos.y + 5), module, RSPhaseOne::THEME_BUTTON));

		addChild(new RSLabelCentered(middle, box.pos.y + 13, "PHASE ONE", 14, module));

		addChild(new RSLabelCentered(middle, box.size.y - 4, "Racket Science", 12, module));

		addChild(ss = new RSScribbleStrip(5, 5));
		if(module) module->ss = ss;

		addChild(new RSPhaseOneDisplay(module, 20, 20, 1040, 200));

		int x, y;

		// SCRUB / PHASE
		x = 73; y = 310;
		addParam(createParamCentered<RSKnobLrg>(Vec(x, y), module, RSPhaseOne::SCRUB_PARAM));
		addChild(new RSLabelCentered(x, y + 54, "SCRUB", 10, module));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y), module, RSPhaseOne::PHASE_INPUT));
		addChild(new RSLabelCentered(x, y + 22, "PHASE", 10, module));

		// ENABLE / WRITE
		x += 73; y -= 30;
		addParam(createParamCentered<RSButtonToggle>(Vec(x, y), module, RSPhaseOne::ENABLE_PARAM));
		addChild(new RSLabelCentered(x, y + 3, "ENABLE", 10, module));
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y + 30), module, RSPhaseOne::WRITE_PARAM));
		addChild(new RSLabelCentered(x, y + 33, "WRITE", 10, module));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y + 60), module, RSPhaseOne::WRITE_INPUT));

		// CV IN
		x += 73; y += 30;
		addParam(createParamCentered<RSKnobLrg>(Vec(x, y), module, RSPhaseOne::IN_PARAM));
		addChild(new RSLabelCentered(x, y + 54, "CV", 10, module));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y), module, RSPhaseOne::IN_INPUT));
		addChild(new RSLabelCentered(x, y + 22, "IN", 10, module));

		// CLEAR
		x += 73; y -= 15;
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSPhaseOne::CLEAR_PARAM));
		addChild(new RSLabelCentered(x, y + 3, "CLEAR", 10, module));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y + 30), module, RSPhaseOne::CLEAR_INPUT));

		// RAND
		x += 37;
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSPhaseOne::RAND_PARAM));
		addChild(new RSLabelCentered(x, y + 3, "RAND", 10, module));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y + 30), module, RSPhaseOne::RAND_INPUT));

		// DIVIDE
		x += 73; y += 15;
		addParam(createParamCentered<RSKnobDetentLrg>(Vec(x, y), module, RSPhaseOne::DIVIDE_PARAM));
		addChild(new RSLabelCentered(x, y + 54, "DIVIDE", 10, module));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y), module, RSPhaseOne::DIVIDE_INPUT));
		
		// SELECT
		x += 100;
		addParam(createParamCentered<RSKnobDetentLrg>(Vec(x, y), module, RSPhaseOne::SELECT_PARAM));
		addChild(new RSLabelCentered(x, y + 54, "SELECT", 10, module));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y), module, RSPhaseOne::SELECT_INPUT));

		// ADJUST
		x += 100;
		addParam(createParamCentered<RSKnobLrg>(Vec(x, y), module, RSPhaseOne::ADJUST_PARAM));
		addChild(new RSLabelCentered(x, y + 54, "ADJUST", 10, module));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y), module, RSPhaseOne::ADJUST_INPUT));

		// BAKE
		x += 73; y -= 15;
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSPhaseOne::BAKE_PARAM));
		addChild(new RSLabelCentered(x, y + 3, "BAKE", 10, module));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y + 30), module, RSPhaseOne::BAKE_INPUT));

		// MAIN OUT
		x = box.size.x - 73; y -= 15;
		addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y), module, RSPhaseOne::REC_OUTPUT));
		addChild(new RSLabelCentered(x, y + 22, "MAIN OUT", 10, module));

		// OVERLAY OUT
		y += 45;
		addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y), module, RSPhaseOne::OVL_OUTPUT));
		addChild(new RSLabelCentered(x, y + 22, "OVERLAY OUT", 10, module));


	}

	void step() override {
		if(!module) return;

		ModuleWidget::step();
	}

    void customDraw(const DrawArgs& args) {}
	#include "RSModuleWidgetDraw.hpp"
};

Model *modelRSPhaseOne = createModel<RSPhaseOne, RSPhaseOneWidget>("RSPhaseOne");


#include <random>
#include <string>

#include "plugin.hpp"

#include "RS.hpp"

struct RSScratch : RSModule {
	enum ParamIds {
		THEME_BUTTON,
		IN_A_PARAM, WRITE_A_PARAM, SCRUB_A_PARAM, CLEAR_A_PARAM, RAND_A_PARAM,
		IN_B_PARAM, WRITE_B_PARAM, SCRUB_B_PARAM, CLEAR_B_PARAM, RAND_B_PARAM,
		IN_C_PARAM, WRITE_C_PARAM, SCRUB_C_PARAM, CLEAR_C_PARAM, RAND_C_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN_A_INPUT, WRITE_A_INPUT, PHASE_A_INPUT, CLEAR_A_INPUT, RAND_A_INPUT,
		IN_B_INPUT, WRITE_B_INPUT, PHASE_B_INPUT, CLEAR_B_INPUT, RAND_B_INPUT,
		IN_C_INPUT, WRITE_C_INPUT, PHASE_C_INPUT, CLEAR_C_INPUT, RAND_C_INPUT,
		POLY_A_INPUT, POLY_B_INPUT, POLY_C_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_A_OUTPUT, OUT_B_OUTPUT,OUT_C_OUTPUT,
		POLY_A_OUTPUT, POLY_B_OUTPUT, POLY_C_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	dsp::BooleanTrigger themeTrigger;

	RSScribbleStrip *ss[3];

	dsp::BooleanTrigger clearTriggerA;
	dsp::BooleanTrigger clearTriggerB;
	dsp::BooleanTrigger clearTriggerC;
	
	dsp::BooleanTrigger randTriggerA;
	dsp::BooleanTrigger randTriggerB;
	dsp::BooleanTrigger randTriggerC;

	#define SAMPLES 1000
	float bufferA[SAMPLES] = {}; unsigned int idxA = 0; bool writeA = false;
	float bufferB[SAMPLES] = {}; unsigned int idxB = 0; bool writeB = false;
	float bufferC[SAMPLES] = {}; unsigned int idxC = 0; bool writeC = false;

	RSScratch() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(THEME_BUTTON, 0.f, 1.f, 0.f, "THEME");

		configParam(IN_A_PARAM, -10.f, 10.f, 0.f, "IN");
		configParam(WRITE_A_PARAM, 0.f, 1.f, 0.f, "WRITE");
		configParam(SCRUB_A_PARAM, -INFINITY, INFINITY, 0.f, "SCRUB");
		configParam(CLEAR_A_PARAM, 0.f, 1.f, 0.f, "CLEAR");
		configParam(RAND_A_PARAM, 0.f, 1.f, 0.f, "RAND");

		configParam(IN_B_PARAM, -10.f, 10.f, 0.f, "IN");
		configParam(WRITE_B_PARAM, 0.f, 1.f, 0.f, "WRITE");
		configParam(SCRUB_B_PARAM, -INFINITY, INFINITY, 0.f, "SCRUB");
		configParam(CLEAR_B_PARAM, 0.f, 1.f, 0.f, "CLEAR");
		configParam(RAND_B_PARAM, 0.f, 1.f, 0.f, "RAND");

		configParam(IN_C_PARAM, -10.f, 10.f, 0.f, "IN");
		configParam(WRITE_C_PARAM, 0.f, 1.f, 0.f, "WRITE");
		configParam(SCRUB_C_PARAM, -INFINITY, INFINITY, 0.f, "SCRUB");
		configParam(CLEAR_C_PARAM, 0.f, 1.f, 0.f, "CLEAR");
		configParam(RAND_C_PARAM, 0.f, 1.f, 0.f, "RAND");
	}

	void process(const ProcessArgs &args) override {
		if(themeTrigger.process(params[THEME_BUTTON].getValue())) {
			RSTheme++;
			if(RSTheme > RSGlobal.themeCount) RSTheme = 1;
		}

		float inA, inB, inC;
		float phaseA, phaseB, phaseC;

		if(clearTriggerA.process(params[CLEAR_A_PARAM].getValue() > 0.f)) resetA();
		if(clearTriggerB.process(params[CLEAR_B_PARAM].getValue() > 0.f)) resetB();
		if(clearTriggerC.process(params[CLEAR_C_PARAM].getValue() > 0.f)) resetC();

		if(randTriggerA.process(params[RAND_A_PARAM].getValue() > 0.f)) randomizeA();
		if(randTriggerB.process(params[RAND_B_PARAM].getValue() > 0.f)) randomizeB();
		if(randTriggerC.process(params[RAND_C_PARAM].getValue() > 0.f)) randomizeC();

		// Would like tooltips to display 0 - 1000 per revolution
		/*
		struct MyParamQuantity : ParamQuantity {
			float getDisplayValue() override {
				return std::fmod(ParamQuantity::getDisplayValue(), 10.f);
			}
		};

		configParam<MyParamQuantity>(...);
		*/

		// Fudge factor 4.15 version 2.41 (tm) (all rights reserved) (use at own risk)

        if(inputs[PHASE_A_INPUT].isConnected()) {
			phaseA = RSclamp(inputs[PHASE_A_INPUT].getVoltage(), 0.f, 10.f);
			idxA = phaseA * SAMPLES / 10;
			params[SCRUB_A_PARAM].setValue(phaseA / 4.15);
		}
		else {
			phaseA = params[SCRUB_A_PARAM].getValue();
			idxA = std::fmod(abs(phaseA / 2.41), 1.f) * SAMPLES;
			if(phaseA < 0.f) idxA = SAMPLES - idxA; // So negative phase doesn't reverse head direction
		}

        if(inputs[PHASE_B_INPUT].isConnected()) {
			phaseB = RSclamp(inputs[PHASE_B_INPUT].getVoltage(), 0.f, 10.f);
			idxB = phaseB * SAMPLES / 10;
			params[SCRUB_B_PARAM].setValue(phaseB / 4.15);
		}
		else {
			phaseB = params[SCRUB_B_PARAM].getValue();
			idxB = std::fmod(abs(phaseB / 2.41), 1.f) * SAMPLES;
			if(phaseB < 0.f) idxB = SAMPLES - idxB;
		}

        if(inputs[PHASE_C_INPUT].isConnected()) {
			phaseC = RSclamp(inputs[PHASE_C_INPUT].getVoltage(), 0.f, 10.f);
			idxC = phaseC * SAMPLES / 10;
			params[SCRUB_C_PARAM].setValue(phaseC / 4.15);
		}
		else {
			phaseC = params[SCRUB_C_PARAM].getValue();
			idxC = std::fmod(abs(phaseC / 2.41), 1.f) * SAMPLES;
			if(phaseC < 0.f) idxC = SAMPLES - idxC;
		}

		if(idxA >= SAMPLES) idxA = SAMPLES - 1;
		if(idxB >= SAMPLES) idxB = SAMPLES - 1;
		if(idxC >= SAMPLES) idxC = SAMPLES - 1;

        if(inputs[IN_A_INPUT].isConnected()) {
			inA = RSclamp(inputs[IN_A_INPUT].getVoltage(), -10.f, 10.f);
			params[IN_A_PARAM].setValue(inA);
		}
		else inA = params[IN_A_PARAM].getValue();

        if(inputs[IN_B_INPUT].isConnected()) {
			inB = RSclamp(inputs[IN_B_INPUT].getVoltage(), -10.f, 10.f);
			params[IN_B_PARAM].setValue(inB);
		}
		else inB = params[IN_B_PARAM].getValue();

        if(inputs[IN_C_INPUT].isConnected()) {
			inC = RSclamp(inputs[IN_C_INPUT].getVoltage(), -10.f, 10.f);
			params[IN_C_PARAM].setValue(inC);
		}
		else inC = params[IN_C_PARAM].getValue();

		if(inputs[WRITE_A_INPUT].isConnected()) {
			if(RSclamp(inputs[WRITE_A_INPUT].getVoltage() > 0.f, -10.f, 10.f)) { // 0.f will be threshold from slider
				bufferA[idxA] = inA;
				writeA = true;
			}
			else writeA = false;
			params[WRITE_A_PARAM].setValue(writeA);
		}
        else 
			if(params[WRITE_A_PARAM].getValue() > 0.f) { // Zero threshold as coming from on off button
				bufferA[idxA] = inA;
				writeA = true;
			}
			else writeA = false;

		if(inputs[WRITE_B_INPUT].isConnected()) {
			if(RSclamp(inputs[WRITE_B_INPUT].getVoltage() > 0.f, -10.f, 10.f)) {
				bufferB[idxB] = inB;
				writeB = true;
			}
			else writeB = false;
			params[WRITE_B_PARAM].setValue(writeB);
		}
        else 
			if(params[WRITE_B_PARAM].getValue() > 0.f) {
				bufferB[idxB] = inB;
				writeB = true;
			}
			else writeB = false;

		if(inputs[WRITE_C_INPUT].isConnected()) {
			if(RSclamp(inputs[WRITE_C_INPUT].getVoltage() > 0.f, -10.f, 10.f)) {
				bufferC[idxC] = inC;
				writeC = true;
			}
			else writeC = false;
			params[WRITE_C_PARAM].setValue(writeC);
		}
        else 
			if(params[WRITE_C_PARAM].getValue() > 0.f) {
				bufferC[idxC] = inC;
				writeC = true;
			}
			else writeC = false;


        outputs[OUT_A_OUTPUT].setVoltage(bufferA[idxA]);
        outputs[OUT_B_OUTPUT].setVoltage(bufferB[idxB]);
        outputs[OUT_C_OUTPUT].setVoltage(bufferC[idxC]);

		// Have a phase out driven from scrub knob
		// Have a eight playhead version with individal scrub / phase & outs, mono & poly out
		// Step input & step length control & direction for stepping through array
		// Scrub knob sensitivity control fine<-->coarse
		// Separate record & play head, then we can loop a seq with the play head and modify with rec head
		// Have a write theshold, then we can up the voltage threshold for writes at will to modify seqs via lfo
		//   this should be a static slider, no need to animate & an excuse to create another component.
		//   Keep it up, keep practicing, you might get back to where you were eventually.

		// record head white when not recording, red when recording
	}

	void onReset() override {
		resetA();
		resetB();
		resetC();
	}

	void resetA() {
		std::memset(bufferA, 0, sizeof(bufferA));
	}

	void resetB() {
		std::memset(bufferB, 0, sizeof(bufferB));
	}

	void resetC() {
		std::memset(bufferC, 0, sizeof(bufferC));
	}

	void onRandomize() override {
		randomizeA();
		randomizeB();
		randomizeC();
	}

	void randomizeA() {
		std::random_device rd;
		std::mt19937 e2(rd());
		std::uniform_real_distribution<> dist(-10.f, 10.f);
		for(int i = 0; i < SAMPLES; i++) {
			bufferA[i] = dist(e2);			
		}
	}

	void randomizeB() {
		std::random_device rd;
		std::mt19937 e2(rd());
		std::uniform_real_distribution<> dist(-10.f, 10.f);
		for(int i = 0; i < SAMPLES; i++) {
			bufferB[i] = dist(e2);			
		}
	}

	void randomizeC() {
		std::random_device rd;
		std::mt19937 e2(rd());
		std::uniform_real_distribution<> dist(-10.f, 10.f);
		for(int i = 0; i < SAMPLES; i++) {
			bufferC[i] = dist(e2);			
		}
	}

	void onPreset() {
		static int p = 0;

		// To create some musical presets, record midi into vv, then save & extract data from save file,
		//  incorporate into a header containing alternative buffers to copy from, flight of the bumblebee
		//  should be good for a chuckle, what else?

		switch(p) {
			case 0:
				for(int i = 0; i < SAMPLES; i++) {
					bufferA[i] = (float)(i * 20.f / SAMPLES) - 10.f;
				}
				break;
			case 1:
				for(int i = 0; i < SAMPLES; i++) {
					bufferA[i] = (float)(i * 10.f / SAMPLES) - 10.f;
				}
				break;
			case 2:
				for(int i = 0; i < SAMPLES; i++) {
					bufferA[i] = (float)(i * 10.f / SAMPLES);
				}
				break;
			case 3:
				for(int i = 0; i < SAMPLES; i++) {
					bufferA[SAMPLES - i - 1] = (float)(i * 20.f / SAMPLES) - 10.f;
				}
				break;
		}

		p++;
		if(p > 3) p = 0;
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();

        json_object_set_new(rootJ, "theme", json_integer(RSTheme));

		json_t* ss0 = json_string(ss[0]->text.c_str());
		json_t* ss1 = json_string(ss[1]->text.c_str());
		json_t* ss2 = json_string(ss[2]->text.c_str());

		json_t* samplesA = json_array();
		json_t* samplesB = json_array();
		json_t* samplesC = json_array();

		for(int i = 0; i < SAMPLES; i++) {
			json_array_append_new(samplesA, json_real(bufferA[i]));
			json_array_append_new(samplesB, json_real(bufferB[i]));
			json_array_append_new(samplesC, json_real(bufferC[i]));
		}

		json_object_set_new(rootJ, "ss0", ss0);
		json_object_set_new(rootJ, "ss1", ss1);
		json_object_set_new(rootJ, "ss2", ss2);

		json_object_set_new(rootJ, "samplesA", samplesA);
		json_object_set_new(rootJ, "samplesB", samplesB);
		json_object_set_new(rootJ, "samplesC", samplesC);

		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
        json_t* themeJ = json_object_get(rootJ, "theme");

        if(themeJ) RSTheme = json_integer_value(themeJ);

		json_t* ss0 = json_object_get(rootJ, "ss0");
		json_t* ss1 = json_object_get(rootJ, "ss1");
		json_t* ss2 = json_object_get(rootJ, "ss2");

		json_t* samplesA = json_object_get(rootJ, "samplesA");
		json_t* samplesB = json_object_get(rootJ, "samplesB");
		json_t* samplesC = json_object_get(rootJ, "samplesC");

		if(ss0) ss[0]->text = json_string_value(ss0);
		if(ss1) ss[1]->text = json_string_value(ss1);
		if(ss2) ss[2]->text = json_string_value(ss2);

		if(samplesA && samplesB && samplesC)
			for(int i = 0; i < SAMPLES; i++) {
				bufferA[i] = json_number_value(json_array_get(samplesA, i));
				bufferB[i] = json_number_value(json_array_get(samplesB, i));
				bufferC[i] = json_number_value(json_array_get(samplesC, i));
			}
	}
};

struct RSBufferDisplay : TransparentWidget {
	std::shared_ptr<Font> font;
	RSScratch* module;
	float *buffer;
	unsigned int *idx;
	bool *write;

	RSBufferDisplay(RSScratch* module, float buffer[], unsigned int &idx, bool &write, int x, int y, int xs, int ys) {
		this->module = module;
		this->buffer = buffer;
		this->idx = &idx;
		this->write = &write;

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
			//if(box.pos.y == 10) nvgText(args.vg, box.pos.x + 350, box.pos.y + 70, "Vector Victor", NULL);

			if(box.pos.y == 10) { //} && now->tm_mday == 26 && now->tm_mon + 1 == 11) {
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
				nvgText(args.vg, box.pos.x + box.size.x / 2, box.pos.y + 70, msg, NULL);
			}

			if(box.pos.y == 70) nvgText(args.vg, box.pos.x + box.size.x / 2, box.pos.y + 70, "R a c k e t   S c i e n c e", NULL);
		
			nvgFontSize(args.vg, 30);
			if(box.pos.y == 130) nvgText(args.vg, box.pos.x + box.size.x / 2, box.pos.y + 70, "Please consider donating : )", NULL);
			nvgStroke(args.vg);
			
			return; // Down here so the module browser thumbnailer gets to see above
		}

		// Centre line
		nvgStrokeColor(args.vg, COLOR_BLUE);
		nvgStrokeWidth(args.vg, 2);
		nvgBeginPath(args.vg);
		nvgMoveTo(args.vg, box.pos.x, box.pos.y + (box.size.y / 2)); // Will need amending for min / max
		nvgLineTo(args.vg, box.pos.x + box.size.x, box.pos.y + (box.size.y / 2));
		nvgStroke(args.vg);



		/*
			availableSpace = 100;
			dataRange = 10; // Will vary according to max
			scaleFactor = 100 / 10; // == 10

		*/

		// Buffer

		int centerLine = box.pos.y + box.size.y / 2;

		// Scale using RSscale.


			// Auto scaled
			float min = 0.f;
			float max = 0.f;
			float range = 0.f;
			for(int i = 0; i < SAMPLES; i++) {
				if(buffer[i] > max) max = buffer[i];
				if(buffer[i] < min) min = buffer[i];
			}

			range = abs(max) > abs(min) ? abs(max) : abs(min);
			range = range < 1.f ? 1.f : range;

			//float scaleFactor = box.size.y / range;
			float scaleFactor = ceil(range * 2);
			if(scaleFactor < 1.f) scaleFactor = 1.f;

			//INFO("Racket Science:  scaleFactor=%f min=%+2.4f max=%+2.4f range=%2.4f", scaleFactor, min, max, range);

			nvgStrokeColor(args.vg, COLOR_RED);
			nvgStrokeWidth(args.vg, 1);
			nvgBeginPath(args.vg);

			nvgMoveTo(args.vg, box.pos.x, centerLine - (buffer[0] / scaleFactor * box.size.y));
			for(int i = 0; i < box.size.x; i++) {
				unsigned int idx = SAMPLES / box.size.x * i;
				int val = buffer[idx] / scaleFactor * box.size.y;
				nvgLineTo(args.vg, box.pos.x + i, centerLine - val);
			}
			nvgStroke(args.vg);
		
		{	// Full scale
			nvgStrokeColor(args.vg, COLOR_GREEN);
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, box.pos.x, centerLine - (buffer[0] / 20 * box.size.y));
			for(int i = 0; i < box.size.x; i++) {
				unsigned int idx = SAMPLES / box.size.x * i;
				int val = buffer[idx] / 20 * box.size.y;
				nvgLineTo(args.vg, box.pos.x + i, centerLine - val);
			}
			nvgStroke(args.vg);
		}


		if(font->handle >= 0) {
			bndSetFont(font->handle);

			nvgFontSize(args.vg, 10);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, 0);
			nvgTextAlign(args.vg, NVG_ALIGN_LEFT);

			char str[6];
			sprintf(str, "%2.2f", range);

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

		// Index
		nvgStrokeColor(args.vg, *write == true ? COLOR_RED : COLOR_RS_GREY);
		nvgStrokeWidth(args.vg, 2);
		nvgBeginPath(args.vg);

		nvgMoveTo(args.vg, box.pos.x + (box.size.x / SAMPLES * *idx), box.pos.y);
		nvgLineTo(args.vg, box.pos.x + (box.size.x / SAMPLES * *idx), box.pos.y + box.size.y);

		nvgStroke(args.vg);

		auto endTime = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsedTime = endTime - startTime;
		//INFO("Racket Science: RSScratch buffer draw elapsed time: %f", elapsedTime.count());
	};
};


struct RSScratchWidget : ModuleWidget {
	RSScratch* module;

	RSScribbleStrip* ss[3];
	RSBufferDisplay* disp;

	RSScratchWidget(RSScratch *module) {
		setModule(module);
		this->module = module;

		box.size.x = mm2px(5.08 * 100);
		int middle = box.size.x / 2 + 1;

		addParam(createParamCentered<RSButtonMomentaryInvisible>(Vec(box.pos.x + 5, box.pos.y + 5), module, RSScratch::THEME_BUTTON));

        addChild(new RSLabelCentered(middle, box.pos.y + 13, "VECTOR VICTOR WITH KNOBS ON", 14, module));
        addChild(new RSLabelCentered(middle, box.size.y - 6, "Racket Science", 12, module));


		int x, y;

		// Channel A

		// SCRUB / PHASE
		x = 65; y = 70;
		addParam(createParamCentered<RSKnobLrg>(Vec(x, y), module, RSScratch::SCRUB_A_PARAM));
		addChild(new RSLabelCentered(x, y + 54, "SCRUB", 10, module));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y), module, RSScratch::PHASE_A_INPUT));
		addChild(new RSLabelCentered(x, y + 22, "PHASE", 10, module));

		// WRITE
		x += 73; y -= 15;
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSScratch::WRITE_A_PARAM));
		addChild(new RSLabelCentered(x, y + 3, "WRITE", 10, module));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y + 30), module, RSScratch::WRITE_A_INPUT));

		// CV IN
		x += 73; y += 15;
		addParam(createParamCentered<RSKnobLrg>(Vec(x, y), module, RSScratch::IN_A_PARAM));
		addChild(new RSLabelCentered(x, y + 54, "CV", 10, module));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y), module, RSScratch::IN_A_INPUT));
		addChild(new RSLabelCentered(x, y + 22, "IN", 10, module));

		// CLEAR
		x += 73; y -= 15;
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSScratch::CLEAR_A_PARAM));
		addChild(new RSLabelCentered(x, y + 3, "CLEAR", 10, module));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y + 30), module, RSScratch::CLEAR_A_INPUT));

		// RAND
		x += 37; y = y;
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSScratch::RAND_A_PARAM));
		addChild(new RSLabelCentered(x, y + 3, "RAND", 10, module));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y + 30), module, RSScratch::RAND_A_INPUT));

		// OUT
		x += 37; y += 15;
		addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y), module, RSScratch::OUT_A_OUTPUT));
		addChild(new RSLabelCentered(x, y + 22, "OUT", 10, module));

		// CHART
		x += 43; y -= 70;
		addChild(ss[0] = new RSScribbleStrip(x, y + 5));
		addChild(new RSBufferDisplay(module, module->bufferA, module->idxA, module->writeA, x / 2, (y / 2) + 10, 1002, 100));


		// Channel B

		// SCRUB / PHASE
		x = 65; y = 190;
		addParam(createParamCentered<RSKnobLrg>(Vec(x, y), module, RSScratch::SCRUB_B_PARAM));
		addChild(new RSLabelCentered(x, y + 54, "SCRUB", 10, module));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y), module, RSScratch::PHASE_B_INPUT));
		addChild(new RSLabelCentered(x, y + 22, "PHASE", 10, module));

		// WRITE
		x += 73; y -= 15;
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSScratch::WRITE_B_PARAM));
		addChild(new RSLabelCentered(x, y + 3, "WRITE", 10, module));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y + 30), module, RSScratch::WRITE_B_INPUT));

		// CV IN
		x += 73; y += 15;
		addParam(createParamCentered<RSKnobLrg>(Vec(x, y), module, RSScratch::IN_B_PARAM));
		addChild(new RSLabelCentered(x, y + 54, "CV", 10, module));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y), module, RSScratch::IN_B_INPUT));
		addChild(new RSLabelCentered(x, y + 22, "IN", 10, module));

		// CLEAR
		x += 73; y -= 15;
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSScratch::CLEAR_B_PARAM));
		addChild(new RSLabelCentered(x, y + 3, "CLEAR", 10, module));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y + 30), module, RSScratch::CLEAR_B_INPUT));

		// RAND
		x += 37; y = y;
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSScratch::RAND_B_PARAM));
		addChild(new RSLabelCentered(x, y + 3, "RAND", 10, module));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y + 30), module, RSScratch::RAND_B_INPUT));

		// OUT
		x += 37; y += 15;
		addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y), module, RSScratch::OUT_B_OUTPUT));
		addChild(new RSLabelCentered(x, y + 22, "OUT", 10, module));

		// CHART
		x += 43; y -= 70;
		addChild(ss[1] = new RSScribbleStrip(x, y + 5));
		addChild(new RSBufferDisplay(module, module->bufferB, module->idxB, module->writeB, x / 2,  (y / 2) + 10, 1002, 100));


		// Channel C
		
		// SCRUB / PHASE
		x = 65; y = 310;
		addParam(createParamCentered<RSKnobLrg>(Vec(x, y), module, RSScratch::SCRUB_C_PARAM));
		addChild(new RSLabelCentered(x, y + 54, "SCRUB", 10, module));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y), module, RSScratch::PHASE_C_INPUT));
		addChild(new RSLabelCentered(x, y + 22, "PHASE", 10, module));

		// WRITE
		x += 73; y -= 15;
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSScratch::WRITE_C_PARAM));
		addChild(new RSLabelCentered(x, y + 3, "WRITE", 10, module));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y + 30), module, RSScratch::WRITE_C_INPUT));

		// CV IN
		x += 73; y += 15;
		addParam(createParamCentered<RSKnobLrg>(Vec(x, y), module, RSScratch::IN_C_PARAM));
		addChild(new RSLabelCentered(x, y + 54, "CV", 10, module));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y), module, RSScratch::IN_C_INPUT));
		addChild(new RSLabelCentered(x, y + 22, "IN", 10, module));

		// CLEAR
		x += 73; y -= 15;
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSScratch::CLEAR_C_PARAM));
		addChild(new RSLabelCentered(x, y + 3, "CLEAR", 10, module));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y + 30), module, RSScratch::CLEAR_C_INPUT));

		// RAND
		x += 37; y = y;
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSScratch::RAND_C_PARAM));
		addChild(new RSLabelCentered(x, y + 3, "RAND", 10, module));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y + 30), module, RSScratch::RAND_C_INPUT));

		// OUT
		x += 37; y += 15;
		addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y), module, RSScratch::OUT_C_OUTPUT));
		addChild(new RSLabelCentered(x, y + 22, "OUT", 10, module));

		// CHART
		x += 43; y -= 70;
		addChild(ss[2] = new RSScribbleStrip(x, y + 5));
		addChild(new RSBufferDisplay(module, module->bufferC, module->idxC, module->writeC, x / 2, (y / 2) + 10, 1002, 100));


		if(module) {
			module->ss[0] = ss[0];
			module->ss[1] = ss[1];
			module->ss[2] = ss[2];
		}

		/*
		addChild(new RSLabelCentered(700, 100, "Test", 14));
		addChild(new RSLabelCentered(700, 150, "This is a Test", 16));
		addChild(new RSLabelCentered(700, 200, "Text should be centered", 18));
		addChild(new RSLabelCentered(700, 250, "Racket Science Custom Label", 24));
		*/
	}

    void customDraw(const DrawArgs& args) {}
	#include "RSModuleWidgetDraw.hpp"

	void step() override {
		if(!module) return;
		
		ModuleWidget::step();
	}
};

Model *modelRSScratch = createModel<RSScratch, RSScratchWidget>("RSScratch");
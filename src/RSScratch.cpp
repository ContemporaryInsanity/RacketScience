#include <random>

#include "plugin.hpp"

#include "RSUtils.hpp"
#include "components/RSComponents.hpp"


struct RSScratch : Module {
	enum ParamIds {
		IN_A_PARAM,
		WRITE_A_PARAM,
		SCRUB_A_PARAM,
		CLEAR_A_PARAM,
		RAND_A_PARAM,
		IN_B_PARAM,
		WRITE_B_PARAM,
		SCRUB_B_PARAM,
		CLEAR_B_PARAM,
		RAND_B_PARAM,
		IN_C_PARAM,
		WRITE_C_PARAM,
		SCRUB_C_PARAM,
		CLEAR_C_PARAM,
		RAND_C_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN_A_INPUT,
		WRITE_A_INPUT,
		PHASE_A_INPUT,
		CLEAR_A_INPUT,
		RAND_A_INPUT,
		IN_B_INPUT,
		WRITE_B_INPUT,
		PHASE_B_INPUT,
		CLEAR_B_INPUT,
		RAND_B_INPUT,
		IN_C_INPUT,
		WRITE_C_INPUT,
		PHASE_C_INPUT,
		CLEAR_C_INPUT,
		RAND_C_INPUT,
		POLY_A_INPUT,
		POLY_B_INPUT,
		POLY_C_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT_A_OUTPUT,
		OUT_B_OUTPUT,
		OUT_C_OUTPUT,
		POLY_A_OUTPUT,
		POLY_B_OUTPUT,
		POLY_C_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	dsp::ClockDivider logDivider;

	RSScratch() {
		logDivider.setDivision(16384);

		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
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

	RSScribbleStrip* ss[3];

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

	void process(const ProcessArgs &args) override {
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

        if(inputs[PHASE_A_INPUT].isConnected()) {
			phaseA = clamp010V(inputs[PHASE_A_INPUT].getVoltage());
			idxA = phaseA * SAMPLES / 10;
			params[SCRUB_A_PARAM].setValue(phaseA / 4.15);
		}
		else {
			phaseA = params[SCRUB_A_PARAM].getValue();
			idxA = std::fmod(abs(phaseA / 2.41), 1.f) * SAMPLES;
			if(phaseA < 0.f) idxA = SAMPLES - idxA; // So negative phase doesn't reverse head direction
		}

        if(inputs[PHASE_B_INPUT].isConnected()) {
			phaseB = clamp010V(inputs[PHASE_B_INPUT].getVoltage());
			idxB = phaseB * SAMPLES / 10;
			params[SCRUB_B_PARAM].setValue(phaseB / 4.15);
		}
		else {
			phaseB = params[SCRUB_B_PARAM].getValue();
			idxB = std::fmod(abs(phaseB / 2.41), 1.f) * SAMPLES;
			if(phaseB < 0.f) idxB = SAMPLES - idxB;
		}

        if(inputs[PHASE_C_INPUT].isConnected()) {
			phaseC = clamp010V(inputs[PHASE_C_INPUT].getVoltage());
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
			inA = clamp10V(inputs[IN_A_INPUT].getVoltage());
			params[IN_A_PARAM].setValue(inA);
		}
		else inA = params[IN_A_PARAM].getValue();

        if(inputs[IN_B_INPUT].isConnected()) {
			inB = clamp10V(inputs[IN_B_INPUT].getVoltage());
			params[IN_B_PARAM].setValue(inB);
		}
		else inB = params[IN_B_PARAM].getValue();

        if(inputs[IN_C_INPUT].isConnected()) {
			inC = clamp10V(inputs[IN_C_INPUT].getVoltage());
			params[IN_C_PARAM].setValue(inC);
		}
		else inC = params[IN_C_PARAM].getValue();

		if(inputs[WRITE_A_INPUT].isConnected()) {
			if(clamp10V(inputs[WRITE_A_INPUT].getVoltage()) > 0.f) { // 0.f will be threshold from slider
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
			if(clamp10V(inputs[WRITE_B_INPUT].getVoltage() > 0.f)) {
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
			if(inputs[WRITE_C_INPUT].getVoltage() > 0.f) {
				bufferC[idxC] = inC;
				params[WRITE_C_PARAM].setValue(1);
			}
			else params[WRITE_C_PARAM].setValue(0);
		}
        else if(params[WRITE_C_PARAM].getValue() > 0.f) bufferC[idxC] = inC;


        outputs[OUT_A_OUTPUT].setVoltage(bufferA[idxA]);
        outputs[OUT_B_OUTPUT].setVoltage(bufferB[idxB]);
        outputs[OUT_C_OUTPUT].setVoltage(bufferC[idxC]);

		if(logDivider.process()) {
			INFO("Racket Science: %i", writeA);
		}
		// Have a phase out driven from scrub knob
		// Have a eight playhead version with individal scrub / phase & outs, mono & poly out
		// Step input & step length control & direction for stepping through array
		// Scrub knob sensitivity control fine<-->coarse
		// Separate record & play head, then we can loop a seq with the play head and modify with rec head
		// Have a write theshold, then we can up the voltage threshold for writes at will to modify seqs via lfo
		//   this should be a static slider, no need to animate & an excuse to create another component.
		//   Keep it up, keep practicing, you might get back to where you were eventually.

		// Fix scrub knobs so that neg values don't reverse scrub direction
		// record head white when not recording, red when recording
	}

	void step() override {

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

	RSBufferDisplay(RSScratch* module, float buffer[], unsigned int &idx, int x, int y, int xs, int ys) {
		this->module = module;
		this->buffer = buffer;
		this->idx = &idx;

		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Ubuntu Condensed 400.ttf"));

		box.pos = Vec(x, y);
		box.size = Vec(xs, ys);
	};

	void draw(const DrawArgs& args) override {
		float min = 0.f;
		float max = 0.f;

		nvgFontSize(args.vg, 10);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, 0);

		// Bounding box
		nvgStrokeColor(args.vg, COLOR_RS_BRONZE);
		nvgFillColor(args.vg, COLOR_BLACK);
		nvgStrokeWidth(args.vg, 3);

		nvgBeginPath(args.vg);
		nvgRoundedRect(args.vg, box.pos.x, box.pos.y, box.size.x, box.size.y, 5);
		nvgStroke(args.vg);
		nvgFill(args.vg);

		if(!module) { // Stand out in the module browser
			nvgFontSize(args.vg, 60);
			nvgFillColor(args.vg, COLOR_RS_BRONZE);
			//if(box.pos.y == 10) nvgText(args.vg, box.pos.x + 350, box.pos.y + 70, "Vector Victor", NULL);
			if(box.pos.y == 70) nvgText(args.vg, box.pos.x + 350, box.pos.y + 70, "Racket Science", NULL);
			//if(box.pos.y == 130) nvgText(args.vg, box.pos.x + 350, box.pos.y + 70, "With Knobs On!!!", NULL);
			nvgStroke(args.vg);
			INFO("Racket Science: !module");
			return; // Down here so the module browser thumbnailer gets to see above
		}

		// Centre line
		nvgStrokeColor(args.vg, COLOR_BLUE);
		nvgStrokeWidth(args.vg, 2);
		nvgBeginPath(args.vg);
		nvgMoveTo(args.vg, box.pos.x, box.pos.y + (box.size.y / 2)); // Will need amending for min / max
		nvgLineTo(args.vg, box.pos.x + box.size.x, box.pos.y + (box.size.y / 2));
		nvgStroke(args.vg);

		// Buffer
		nvgStrokeColor(args.vg, COLOR_GREEN);
		nvgStrokeWidth(args.vg, 2);
		nvgBeginPath(args.vg);

		for(int i = 0; i < SAMPLES; i++) {
			if(buffer[i] > max) max = buffer[i];
			if(buffer[i] < min) min = buffer[i];
		}

		int scale = 10 - (int)max > (int)min ? int(max) : int(min);

		//INFO("Racket Science: scale=%i min=%i max=%i", scale, int(min), int(max));

		int val = buffer[0] / 20 * box.size.y;
		nvgMoveTo(args.vg, box.pos.x, box.pos.y + box.size.y / 2);

		for(int i = 0; i < box.size.x; i++) {
			unsigned int idx = SAMPLES / box.size.x * i;
			//val = buffer[idx] / 20 * box.size.y;
			val = buffer[idx] / 20 * box.size.y;
			nvgLineTo(args.vg, box.pos.x + i, box.pos.y + box.size.y / 2 - val);
		}

		nvgStroke(args.vg);

		// Index
		nvgStrokeColor(args.vg, COLOR_RED);
		nvgStrokeWidth(args.vg, 2);
		nvgBeginPath(args.vg);

		nvgMoveTo(args.vg, box.pos.x + (box.size.x / SAMPLES * *idx), box.pos.y);
		nvgLineTo(args.vg, box.pos.x + (box.size.x / SAMPLES * *idx), box.pos.y + box.size.y);

		nvgStroke(args.vg);
	};
};


struct RSScratchWidget : ModuleWidget {
	RSScratch* module;

	RSScribbleStrip* ss[3];
	RSBufferDisplay* disp;

	RSScratchWidget(RSScratch *module) {
		setModule(module);
		this->module = module;

		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/RSScratch.svg")));

		// Channel A
		{
		int x = 65, y = 65;
		addParam(createParamCentered<RSKnobMedBlk>(Vec(x, y), module, RSScratch::SCRUB_A_PARAM));
		addChild(new RSLabel(x - 10, y + 54, "SCRUB"));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y), module, RSScratch::PHASE_A_INPUT));
		addChild(new RSLabel(x - 10, y + 22, "PHASE"));
		}

		{
		int x = 138, y = 50;
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSScratch::WRITE_A_PARAM));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y + 30), module, RSScratch::WRITE_A_INPUT));
		addChild(new RSLabel(x - 10, y + 52, "WRITE"));
		}

		{
		int x = 210, y = 65;
		addParam(createParamCentered<RSKnobMedBlk>(Vec(x, y), module, RSScratch::IN_A_PARAM));
		addChild(new RSLabel(x - 4, y + 54, "CV"));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y), module, RSScratch::IN_A_INPUT));
		addChild(new RSLabel(x - 5, y + 22, "IN"));
		}

		{
		int x = 283, y = 50;
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSScratch::CLEAR_A_PARAM));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y + 30), module, RSScratch::CLEAR_A_INPUT));
		addChild(new RSLabel(x - 10, y + 52, "CLEAR"));
		}

		{
		int x = 320, y = 50;
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSScratch::RAND_A_PARAM));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y + 30), module, RSScratch::RAND_A_INPUT));
		addChild(new RSLabel(x - 10, y + 52, "RAND"));
		}

		{
		int x = 357, y = 65;
		addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y), module, RSScratch::OUT_A_OUTPUT));
		addChild(new RSLabel(x - 7, y + 22, "OUT"));
		}

		{
		int x = 1450, y = 50;
		addInput(createInputCentered<RSJackPolyIn>(Vec(x, y), module, RSScratch::POLY_A_INPUT));
		addOutput(createOutputCentered<RSJackPolyOut>(Vec(x, y + 30), module, RSScratch::POLY_A_OUTPUT));
		}

		// Channel B
		{
		int x = 65, y = 185;
		addParam(createParamCentered<RSKnobMedBlk>(Vec(x, y), module, RSScratch::SCRUB_B_PARAM));
		addChild(new RSLabel(x - 10, y + 54, "SCRUB"));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y), module, RSScratch::PHASE_B_INPUT));
		addChild(new RSLabel(x - 10, y + 22, "PHASE"));
		}

		{
		int x = 138, y = 170;
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSScratch::WRITE_B_PARAM));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y + 30), module, RSScratch::WRITE_B_INPUT));
		addChild(new RSLabel(x - 10, y + 52, "WRITE"));
		}

		{
		int x = 210, y = 185;
		addParam(createParamCentered<RSKnobMedBlk>(Vec(x, y), module, RSScratch::IN_B_PARAM));
		addChild(new RSLabel(x - 4, y + 54, "CV"));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y), module, RSScratch::IN_B_INPUT));
		addChild(new RSLabel(x - 5, y + 22, "IN"));
		}

		{
		int x = 283, y = 170;
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSScratch::CLEAR_B_PARAM));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y + 30), module, RSScratch::CLEAR_B_INPUT));
		addChild(new RSLabel(x - 10, y + 52, "CLEAR"));
		}

		{
		int x = 320, y = 170;
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSScratch::RAND_B_PARAM));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y + 30), module, RSScratch::RAND_B_INPUT));
		addChild(new RSLabel(x - 10, y + 52, "RAND"));
		}

		{
		int x = 357, y = 185;
		addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y), module, RSScratch::OUT_B_OUTPUT));
		addChild(new RSLabel(x - 7, y + 22, "OUT"));
		}

		{
		int x = 1450, y = 170;
		addInput(createInputCentered<RSJackPolyIn>(Vec(x, y), module, RSScratch::POLY_B_INPUT));
		addOutput(createOutputCentered<RSJackPolyOut>(Vec(x, y + 30), module, RSScratch::POLY_B_OUTPUT));
		}

		// Channel C
		{
		int x = 65, y = 305;
		addParam(createParamCentered<RSKnobMedBlk>(Vec(x, y), module, RSScratch::SCRUB_C_PARAM));
		addChild(new RSLabel(x - 10, y + 54, "SCRUB"));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y), module, RSScratch::PHASE_C_INPUT));
		addChild(new RSLabel(x - 10, y + 22, "PHASE"));
		}

		{
		int x = 138, y = 290;
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSScratch::WRITE_C_PARAM));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y + 30), module, RSScratch::WRITE_C_INPUT));
		addChild(new RSLabel(x - 10, y + 52, "WRITE"));
		}

		{
		int x = 210, y = 305;
		addParam(createParamCentered<RSKnobMedBlk>(Vec(x, y), module, RSScratch::IN_C_PARAM));
		addChild(new RSLabel(x - 4, y + 54, "CV"));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y), module, RSScratch::IN_C_INPUT));
		addChild(new RSLabel(x - 5, y + 22, "IN"));
		}

		{
		int x = 283, y = 290;
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSScratch::CLEAR_C_PARAM));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y + 30), module, RSScratch::CLEAR_C_INPUT));
		addChild(new RSLabel(x - 10, y + 52, "CLEAR"));
		}

		{
		int x = 320, y = 290;
		addParam(createParamCentered<RSButtonMomentary>(Vec(x, y), module, RSScratch::RAND_C_PARAM));
		addInput(createInputCentered<RSJackMonoIn>(Vec(x, y + 30), module, RSScratch::RAND_C_INPUT));
		addChild(new RSLabel(x - 10, y + 52, "RAND"));
		}

		{
		int x = 357, y = 305;
		addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y), module, RSScratch::OUT_C_OUTPUT));
		addChild(new RSLabel(x - 7, y + 22, "OUT"));
		}

		{
		int x = 1450, y = 290;
		addInput(createInputCentered<RSJackPolyIn>(Vec(x, y), module, RSScratch::POLY_C_INPUT));
		addOutput(createOutputCentered<RSJackPolyOut>(Vec(x, y + 30), module, RSScratch::POLY_C_OUTPUT));
		}


		addChild(ss[0] = new RSScribbleStrip(400, 5));
		addChild(ss[1] = new RSScribbleStrip(400, 125));
		addChild(ss[2] = new RSScribbleStrip(400, 245));

		if(module) {
			module->ss[0] = ss[0];
			module->ss[1] = ss[1];
			module->ss[2] = ss[2];
		}

		addChild(new RSBufferDisplay(module, module->bufferA, module->idxA, 200,  10, 1002, 100));
		addChild(new RSBufferDisplay(module, module->bufferB, module->idxB, 200,  70, 1002, 100));
		addChild(new RSBufferDisplay(module, module->bufferC, module->idxC, 200, 130, 1002, 100));
	}

	void step() override {
		if(!module) return;
		
		ModuleWidget::step();
	}
};


Model *modelRSScratch = createModel<RSScratch, RSScratchWidget>("RSScratch");
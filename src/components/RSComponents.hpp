/*
	Racket Science custom components
	(C) 2019 Ewen Bates
*/

using namespace rack;

// Colours

#define COLOR_BLACK nvgRGB(0x00, 0x00, 0x00)
#define COLOR_WHITE nvgRGB(0xff, 0xff, 0xff)
#define COLOR_RED   nvgRGB(0xff, 0x00, 0x00)
#define COLOR_GREEN nvgRGB(0x00, 0xff, 0x00)
#define COLOR_BLUE  nvgRGB(0x00, 0x00, 0xff)

#define COLOR_RS_GREY   nvgRGB(0xB4, 0xB4, 0xB4)
#define COLOR_RS_BRONZE nvgRGB(133, 135, 57);


// Labels

inline Label* addLabel(int x, int y, const char* str = "", const NVGcolor& color = COLOR_RS_GREY) {
	// Adds a label to the panel, with row spacing of 12 this gives us 30 lines from top to bottom
	Label* label = new Label();

	label->box.pos.x = x;
	label->box.pos.y = y;
	label->color = color;
	label->text = str;
	label->fontSize = 10;

	return label;
}


// Ports

struct RSJackMonoOut : SVGPort { RSJackMonoOut() { setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackMonoOut.svg"))); } };
struct RSJackPolyOut : SVGPort { RSJackPolyOut() { setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackPolyOut.svg"))); } };
struct RSJackMonoIn  : SVGPort { RSJackMonoIn()  { setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackMonoIn.svg"))); } };
struct RSJackPolyIn  : SVGPort { RSJackPolyIn()  { setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackPolyIn.svg"))); } };


// Knobs

struct RSKnob : SVGKnob {
	RSKnob() {
		minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;

		shadow->opacity = 0.0f; // Hide shadows
	}
};

struct RSKnobSmlBlk : RSKnob { RSKnobSmlBlk() {	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSKnobSmlBlk.svg")));	} };
struct RSKnobMedBlk : RSKnob { RSKnobMedBlk() {	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSKnobMedBlk.svg")));	} };
struct RSKnobLrgBlk : RSKnob { RSKnobLrgBlk() {	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSKnobLrgBlk.svg")));	} };


// Buttons

struct RSButton : SVGSwitch { // The Count uses SvgSwitch?
	RSButton() {
		shadow->opacity = 0.0f; // Hide shadows
	}

	void randomize() override {
		SVGSwitch::randomize();

		if(paramQuantity->getValue() > 0.5f) paramQuantity->setValue(1.0f);
		else								 paramQuantity->setValue(0.0f);
	}
};

struct RSButtonToggle : RSButton {
	RSButtonToggle() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSButton_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSButton_1.svg")));
	}
};

struct RSButtonMomentary: RSButtonToggle { // The Count inherits from button and explicitly adds the frames again, surely this will work ok?
	RSButtonMomentary() {
		momentary = true;
	}
};


// Switches

struct RSSwitch2P : SVGSwitch {
	RSSwitch2P() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSSwitch_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSSwitch_2.svg")));

		shadow->opacity = 0.0f;
	}

	void onChange(const event::Change &e) override {
		SVGSwitch::onChange(e);

		if(paramQuantity->getValue() > 0.5f) paramQuantity->setValue(1.0f);
		else 								 paramQuantity->setValue(0.0f);
	}

	void randomize() override {
		SVGSwitch::randomize();

		if(paramQuantity->getValue() > 0.5f) paramQuantity->setValue(1.0f);
		else 								 paramQuantity->setValue(0.0f);
	}
};

struct RSSwitch3P : SVGSwitch {
	RSSwitch3P() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSSwitch_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSSwitch_1.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSSwitch_2.svg")));

		shadow->opacity = 0.0f;
	}

	void onChange(const event::Change &e) override {
		SVGSwitch::onChange(e);

		if(paramQuantity->getValue() > 1.33f) 	   paramQuantity->setValue(2.0f);
		else if(paramQuantity->getValue() > 0.67f) paramQuantity->setValue(1.0f);
		else 									   paramQuantity->setValue(0.0f);
	}

	void randomize() override {
		SVGSwitch::randomize();
		if(paramQuantity->getValue() > 1.33f) 	   paramQuantity->setValue(2.0f);
		else if(paramQuantity->getValue() > 0.67f) paramQuantity->setValue(1.0f);
		else 									   paramQuantity->setValue(0.0f);
	}
};

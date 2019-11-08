/*
	Racket Science custom components
	(C) 2019 Ewen Bates
*/

//using namespace rack;

// Colours

#define COLOR_BLACK  nvgRGB(0x00, 0x00, 0x00)
#define COLOR_WHITE  nvgRGB(0xff, 0xff, 0xff)
#define COLOR_RED    nvgRGB(0xff, 0x00, 0x00)
#define COLOR_GREEN  nvgRGB(0x00, 0xff, 0x00)
#define COLOR_BLUE   nvgRGB(0x00, 0x00, 0xff)
#define COLOR_YELLOW nvgRGB(0xff, 0xff, 0x00)

#define COLOR_RS_GREY   nvgRGB(0xB4, 0xB4, 0xB4)
#define COLOR_RS_BRONZE nvgRGB(133, 135, 57)
#define COLOR_RS_BG		nvgRGB(25, 25, 25)
#define COLOR_RS_COMP	nvgRGB(42, 42, 42)


// Labels
/*
struct RSLabel : Label {
	RSLabel(int x, int y, const char* str = "", const NVGcolor& colour = COLOR_RS_GREY) {
		box.pos = Vec(x, y);
		text = str;
		color = colour;
		fontSize = 10;
	};
};
*/
struct RSLabel : LedDisplay {
	std::shared_ptr<Font> font;
	std::string text;
	NVGcolor color;

	RSLabel(int x, int y, const char* str = "", const NVGcolor& colour = COLOR_RS_GREY) {
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Ubuntu Condensed 400.ttf"));
		box.pos = Vec(x, y);
		text = str;
		color = colour;
	};

	void draw(const DrawArgs &args) override {
		if(font->handle >= 0) {
			bndSetFont(font->handle);

			nvgFontSize(args.vg, 10);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, 0);

			nvgFillColor(args.vg, color);
			nvgText(args.vg, 0, 0, text.c_str(), NULL);

			bndSetFont(APP->window->uiFont->handle);
		}
	};
};


// Scribble Strips

struct RSScribbleStrip : LedDisplayTextField {
	int textSize = 12;
	int numChars = 40;

	RSScribbleStrip(int x, int y, int size = 150, const char* str = "_", const NVGcolor& colour = COLOR_RS_BRONZE) {
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Ubuntu Condensed 400.ttf"));
		box.pos = Vec(x, y);
		box.size = Vec(size, 14); // Derive size from pos & panel width?  have numChars as parameter instead
		textOffset = Vec(0, -3);
		multiline = false; // Doesn't appear to have the desired effect
		text = str;
		color = colour;
	};

	// We want scribbles without background
	void draw(const DrawArgs &args) override {
		if(cursor > numChars) {
			text.resize(numChars);
			cursor = numChars;
			selection = numChars;
		}

		nvgScissor(args.vg, RECT_ARGS(args.clipBox));
		if (font->handle >= 0) {
			bndSetFont(font->handle);

			//NVGcolor highlightColor = color;
			//highlightColor.a = 0.5;
			int begin = std::min(cursor, selection);
			int end = (this == APP->event->selectedWidget) ? std::max(cursor, selection) : -1;
			bndIconLabelCaret(args.vg, textOffset.x, textOffset.y,
				box.size.x - 2*textOffset.x, box.size.y - 2*textOffset.y,
				-1, color, textSize, text.c_str(), color /* highlightColor */, begin, end);

			bndSetFont(APP->window->uiFont->handle);
		}
		nvgResetScissor(args.vg);
	};
};


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

struct RSKnobDetent : RSKnob {
	RSKnobDetent() {
		snap = true;
	}
};

struct RSKnobSmlBlk : RSKnob { RSKnobSmlBlk() {	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSKnobSmlBlk.svg")));	} };
struct RSKnobMedBlk : RSKnob { RSKnobMedBlk() {	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSKnobMedBlk.svg")));	} };
struct RSKnobLrgBlk : RSKnob { RSKnobLrgBlk() {	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSKnobLrgBlk.svg")));	} };

struct RSKnobDetentSmlBlk : RSKnobDetent { RSKnobDetentSmlBlk() { setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSKnobSmlBlk.svg"))); } };
struct RSKnobDetentMedBlk : RSKnobDetent { RSKnobDetentMedBlk() { setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSKnobMedBlk.svg"))); } };
struct RSKnobDetentLrgBlk : RSKnobDetent { RSKnobDetentLrgBlk() { setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSKnobLrgBlk.svg"))); } };


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

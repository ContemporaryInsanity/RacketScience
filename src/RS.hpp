#include <ctime>
#include <chrono>
#include <random>

#include "RSModule.hpp"


#define quantize(v)  (round(v * 12.f) / 12.f)
#define octave(v)    int(floor(quantize(v)))
#define note(v)      (int(round((v + 10) * 12)) % 12)

static float noteVoltage[] = {
    0.00000f, // C
    0.08333f, // C#
    0.16667f, // D
    0.25000f, // D#
    0.33333f, // E
    0.41667f, // F
    0.50000f, // F#
    0.58333f, // G
    0.66667f, // G#
    0.75000f, // A
    0.83333f, // A#
    0.91667f, // B
};

static inline float RSclamp(float in, float min, float max) {
	return in < min ? min : (in > max ? max : in);
}

static inline float RSscale(float in, float inMin, float inMax, float outMin, float outMax) {
	return((outMax - outMin) * (in - inMin) / (inMax - inMin)) + outMin;
}


/*
	Racket Science custom components
	(C) 2020 Ewen Bates
*/

// Colours

#define COLOR_RS_GREY   nvgRGB(0xb4, 0xb4, 0xb4)
#define COLOR_RS_BRONZE nvgRGB(0x85, 0x87, 0x39)
#define COLOR_BLACK		nvgRGB(0x00, 0x00, 0x00)
#define COLOR_RED		nvgRGB(0xff, 0x00, 0x00)
#define COLOR_GREEN		nvgRGB(0x00, 0xff, 0x00)
#define COLOR_BLUE		nvgRGB(0x00, 0x00, 0xff)
#define COLOR_YELLOW	nvgRGB(0xff, 0xff, 0x00)
#define COLOR_WHITE		nvgRGB(0xff, 0xff, 0xff)


// LEDs

struct RSLightWidget : LightWidget {



	

};


// Labels

// Uses own colour, intended for use with scale labels, green for pos, red for neg
// Now we have themes this causes problems with contrast, perhaps have a black background?
struct RSLabel : LedDisplay {
	int fontSize;
	std::shared_ptr<Font> font;
	std::string text;
	NVGcolor color;

	RSLabel(int x, int y, const char* str = "", int fontSize = 10, const NVGcolor& colour = COLOR_RS_GREY) {
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Ubuntu Condensed 400.ttf"));
		box.pos = Vec(x, y);
		box.size = Vec(120, 12);
		text = str;
		color = colour;
		this->fontSize = fontSize;
	}

	void draw(const DrawArgs &args) override {
		if(font->handle >= 0) {
			bndSetFont(font->handle);

			nvgFontSize(args.vg, fontSize);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, 0);

			nvgBeginPath(args.vg);
			nvgFillColor(args.vg, color);
			nvgText(args.vg, 0, 0, text.c_str(), NULL);
			nvgStroke(args.vg);

			bndSetFont(APP->window->uiFont->handle);
		}
	}
};


// Uses RSGlobal color, intended for general labels
struct RSLabelCentered : LedDisplay {
	int fontSize;
	std::shared_ptr<Font> font;
	std::string text;
	int *themeIdx = NULL;

	RSLabelCentered(int x, int y, const char* str = "", int fontSize = 10, RSModule *module = NULL) {
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Ubuntu Condensed 400.ttf"));
		this->fontSize = fontSize;
		box.pos = Vec(x, y);
		text = str;

        if(module) themeIdx = &(module->RSTheme);
	}

	void draw(const DrawArgs &args) override {
		if(font->handle >= 0) {
			bndSetFont(font->handle);

			nvgFontSize(args.vg, fontSize);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, 0);
			nvgTextAlign(args.vg, NVG_ALIGN_CENTER);

			nvgBeginPath(args.vg);
			if(themeIdx == NULL) nvgFillColor(args.vg, RSGlobal.themes[RSGlobal.themeIdx].lbColor);
			else                 nvgFillColor(args.vg, RSGlobal.themes[*themeIdx - 1].lbColor);
			
			nvgText(args.vg, 0, 0, text.c_str(), NULL);
			nvgStroke(args.vg);

			bndSetFont(APP->window->uiFont->handle);
		}
	}
};


// Scribble Strips

struct RSScribbleStrip : LedDisplayTextField {
	int textSize = 12;
	int numChars = 25;

	RSScribbleStrip(int x, int y, int size = 150, const char* str = "_") {
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/Ubuntu Condensed 400.ttf"));
		box.pos = Vec(x, y);
		box.size = Vec(size, 14); // Derive size from pos & panel width?  have numChars as parameter instead
		textOffset = Vec(0, -3);
		multiline = false; // Doesn't appear to have the desired effect
		text = str;
	}

	// We want scribbles without background
	void draw(const DrawArgs &args) override {
		if(cursor > numChars) {
			text.resize(numChars);
			cursor = numChars;
			selection = numChars;
		}

		//nvgScissor(args.vg, RECT_ARGS(args.clipBox));
		if (font->handle >= 0) {
			bndSetFont(font->handle);

			float bounds[4];
			nvgTextBounds(args.vg, 0.0f, 0.0f, text.c_str(), NULL, bounds);
			float textWidth = bounds[2];

			// If we subtract textWidth / 2 from parameter 2 textOffset.x we get dynamically centered strips
			//   however the mouse doesn't position the cursor accordingly
			NVGcolor color = RSGlobal.themes[RSGlobal.themeIdx].ssColor;
			NVGcolor highlightColor = color;
			highlightColor.a = 0.5;
			int begin = std::min(cursor, selection);
			int end = (this == APP->event->selectedWidget) ? std::max(cursor, selection) : -1;
			//INFO("Racket Science: textWidth: %f  box.size.x: %f  box.size.y: %f", textWidth, box.size.x, box.size.y);
			//INFO("Racket Science: cursor: %i  selection: %i  begin: %i  end: %i", cursor, selection, begin, end);
			bndIconLabelCaret(args.vg, textOffset.x, textOffset.y,
				box.size.x - 2*textOffset.x, box.size.y - 2*textOffset.y,
				-1, color, textSize, text.c_str(), highlightColor, begin, end);

			bndSetFont(APP->window->uiFont->handle);
		}
		//nvgResetScissor(args.vg);
	}
};


// Ports

struct RSJackMonoOut      : SVGPort { RSJackMonoOut()      { setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackMonoOut.svg"))); } };
struct RSJackSmallMonoOut : SVGPort { RSJackSmallMonoOut() { setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackSmallMonoOut.svg"))); } };
struct RSJackPolyOut      : SVGPort { RSJackPolyOut()      { setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackPolyOut.svg"))); } };
struct RSJackMonoIn       : SVGPort { RSJackMonoIn()       { setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackMonoIn.svg"))); } };
struct RSJackSmallMonoIn  : SVGPort { RSJackSmallMonoIn()  { setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackSmallMonoIn.svg"))); } };
struct RSJackPolyIn       : SVGPort { RSJackPolyIn()       { setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackPolyIn.svg"))); } };

struct RSStealthJackIn : app::SvgPort { // With thanks to https://github.com/DominoMarama/ReTunesFree
//	RSStealthJackIn() { 
//		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackMonoIn.svg")));
//	}

	void step() override {
		if(!module) return;

		if(module->inputs[portId].isConnected()) {
			Widget::show();
		}
		else {
			CableWidget* cw = APP->scene->rack->incompleteCable;
			if(cw) {
				if(cw->outputPort) Widget::show();
				else Widget::hide();
			}
			else Widget::hide();
		}
		Widget::step();
	}
};

struct RSStealthJackOut : app::SvgPort {
//	RSStealthJackOut() { 
//		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackMonoOut.svg")));
//	}

	void step() override {
		if(!module) return;

		if(module->outputs[portId].isConnected()) {
			Widget::show();
		}
		else {
			CableWidget* cw = APP->scene->rack->incompleteCable;
			if(cw) {
				if(cw->inputPort) Widget::show();
				else Widget::hide();
			}
			else Widget::hide();
		}
		Widget::step();
	}
};

struct RSStealthJackMonoIn : RSStealthJackIn {
	RSStealthJackMonoIn() { 
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackMonoIn.svg")));
	}
};

struct RSStealthJackSmallMonoIn : RSStealthJackIn {
	RSStealthJackSmallMonoIn() { 
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackSmallMonoIn.svg")));
	}
};

struct RSStealthJackMonoOut : RSStealthJackOut {
	RSStealthJackMonoOut() { 
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackMonoOut.svg")));
	}
};

struct RSStealthJackSmallMonoOut : RSStealthJackOut {
	RSStealthJackSmallMonoOut() { 
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackSmallMonoOut.svg")));
	}
};

struct RSStealthJackPolyIn : RSStealthJackIn {
	RSStealthJackPolyIn() { 
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackPolyIn.svg")));
	}
};

struct RSStealthJackPolyOut : RSStealthJackOut {
	RSStealthJackPolyOut() { 
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSJackPolyOut.svg")));
	}
};


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

struct RSKnobSml : RSKnob { RSKnobSml() {setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSKnobSml.svg"))); } };
struct RSKnobMed : RSKnob { RSKnobMed() {setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSKnobMed.svg"))); } };
struct RSKnobLrg : RSKnob { RSKnobLrg() {setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSKnobLrg.svg"))); } };

struct RSKnobInvisible : RSKnob { RSKnobInvisible() {setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSKnobInvisible.svg"))); } };

struct RSKnobDetentSml : RSKnobDetent { RSKnobDetentSml() { setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSKnobSml.svg"))); } };
struct RSKnobDetentMed : RSKnobDetent { RSKnobDetentMed() { setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSKnobMed.svg"))); } };
struct RSKnobDetentLrg : RSKnobDetent { RSKnobDetentLrg() { setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSKnobLrg.svg"))); } };

struct RSKnobDetentInvisible : RSKnobDetent { RSKnobDetentInvisible() {setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSKnobInvisible.svg"))); } };


// Buttons

struct RSButton : SVGSwitch {
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
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSButton.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSButtonPress.svg")));
	}
};

struct RSRoundButtonToggle : RSButton {
	RSRoundButtonToggle() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSRoundButton.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSRoundButtonPress.svg")));
	}
};

struct RSButtonToggleInvisible : RSButton {
	RSButtonToggleInvisible() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSButtonInvisibleIsh.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RSButtonInvisible.svg")));
	}
};

struct RSButtonMomentary : RSButtonToggle {
	RSButtonMomentary() {
		momentary = true;
	}
};

struct RSRoundButtonMomentary : RSRoundButtonToggle {
	RSRoundButtonMomentary() {
		momentary = true;
	}
};

struct RSButtonMomentaryInvisible : RSButtonToggleInvisible {
	RSButtonMomentaryInvisible() {
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

struct RSSwitch3PV : SVGSwitch {
	RSSwitch3PV() {
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

struct ModuleResizeHandle : OpaqueWidget {
	bool right = false;
	Vec dragPos;
	Rect originalBox;

	ModuleResizeHandle() {
		box.size = Vec(1 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	}

	void onDragStart(const event::DragStart &e) override {
		if (e.button != GLFW_MOUSE_BUTTON_LEFT)
			return;

		dragPos = APP->scene->rack->mousePos;
		ModuleWidget *mw = getAncestorOfType<ModuleWidget>();
		assert(mw);
		originalBox = mw->box;
	}

	void onDragMove(const event::DragMove &e) override {
		ModuleWidget *mw = getAncestorOfType<ModuleWidget>();
		assert(mw);

		Vec newDragPos = APP->scene->rack->mousePos;
		float deltaX = newDragPos.x - dragPos.x;

		Rect newBox = originalBox;
		Rect oldBox = mw->box;
		const float minWidth = 3 * RACK_GRID_WIDTH;
		if (right) {
			newBox.size.x += deltaX;
			newBox.size.x = std::fmax(newBox.size.x, minWidth);
			newBox.size.x = std::round(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
		}
		else {
			newBox.size.x -= deltaX;
			newBox.size.x = std::fmax(newBox.size.x, minWidth);
			newBox.size.x = std::round(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
			newBox.pos.x = originalBox.pos.x + originalBox.size.x - newBox.size.x;
		}

		mw->box = newBox;
		if (!APP->scene->rack->requestModulePos(mw, newBox.pos)) {
			mw->box = oldBox;
		}
	}
};

#include "plugin.hpp"

#include "RS.hpp"

struct RSXYGLR : RSModule {
    static const int samples = 1000;

    enum ParamIds {
        THEME_BUTTON,
        CLEAR_BUTTON,
        NUM_PARAMS
    };
    enum InputIds {
        PHASE_IN,
        X_IN,
        Y_IN,
        G_IN,
        NUM_INPUTS
    };
    enum OutputIds {
        X_OUT,
        Y_OUT,
        G_OUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    dsp::BooleanTrigger themeTrigger;

    float x[samples] = {};
    float y[samples] = {};
    bool g[samples] = {};

    float phaseIn, xIn, yIn;
    bool gIn;
    unsigned int idx;

    RSXYGLR() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(THEME_BUTTON, 0.f, 1.f, 0.f, "THEME");

        configParam(CLEAR_BUTTON, 0.f, 1.f, 0.f, "CLEAR");
    }

    void process(const ProcessArgs &args) override {
		if(themeTrigger.process(params[THEME_BUTTON].getValue())) {
			RSTheme++;
			if(RSTheme > RSGlobal.themeCount) RSTheme = 1;
		}

        if(inputs[PHASE_IN].isConnected()) {
            phaseIn = RSclamp(inputs[PHASE_IN].getVoltage(), 0.f, 10.f);
            idx = phaseIn * samples / 10.f;
            if(idx >= samples) idx = samples - 1;

            xIn = RSclamp(inputs[X_IN].getVoltage(), 0.f, 10.f);
            yIn = RSclamp(inputs[Y_IN].getVoltage(), 0.f, 10.f);
            gIn = inputs[G_IN].getVoltage() > 0.f ? true : false;

            if(gIn) {
                x[idx] = xIn;
                y[idx] = yIn;
                g[idx] = gIn;
            }

            outputs[X_OUT].setVoltage(x[idx]);
            outputs[Y_OUT].setVoltage(y[idx]);
            outputs[G_OUT].setVoltage(g[idx]);
        }

        if(params[CLEAR_BUTTON].getValue()) std::memset(g, 0, sizeof(g));
    }

    void onReset() override {
        std::memset(x, 0, sizeof(x));
        std::memset(y, 0, sizeof(y));
        std::memset(g, 0, sizeof(g));
    }

    json_t* dataToJson() override {
        json_t* rootJ = json_object();

        json_object_set_new(rootJ, "theme", json_integer(RSTheme));

        return rootJ;
    }

    void dataFromJson(json_t* rootJ) override {
        json_t* themeJ = json_object_get(rootJ, "theme");

        if(themeJ) RSTheme = json_integer_value(themeJ);
    }
};

struct RSTouchPadDisplay : TransparentWidget {
    RSXYGLR *module;

    RSTouchPadDisplay(RSXYGLR *module, int x, int y, int xs, int ys) {
        this->module = module;

        box.pos = Vec(x / 2, y / 2);
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

        if(!module) return;

        // Indicate last reported position
        nvgFillColor(args.vg, module->gIn ? COLOR_RED : COLOR_GREEN);
        nvgBeginPath(args.vg);
        nvgCircle(args.vg, module->xIn * (box.size.x / 12.f) + 30, box.size.y - (module->yIn * (box.size.y / 12.f)) -5, 10.f);
        nvgStroke(args.vg);
        nvgFill(args.vg);

        // Indicate phase indexed position
        if(module->g[module->idx]) {
            nvgFillColor(args.vg, COLOR_RED);
            nvgBeginPath(args.vg);
            nvgCircle(args.vg, module->x[module->idx] * (box.size.x / 12.f) + 30, box.size.y - (module->y[module->idx] * (box.size.y / 12.f)) - 5, 10.f);
            nvgStroke(args.vg);
            nvgFill(args.vg);
        }
    };
};

struct RSXYGBufferDisplay : TransparentWidget {
    RSXYGLR *module;

    RSXYGBufferDisplay(RSXYGLR *module, int x, int y, int xs, int ys) {
        this->module = module;

        box.pos = Vec(x / 2, y / 2);
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

        if(!module) return;

        // Display g buffer contents
        nvgStrokeColor(args.vg, COLOR_RED);
        nvgBeginPath(args.vg);
        for(int i = 0; i < box.size.x; i++) {
            unsigned int idx = module->samples / box.size.x * i;
            if(module->g[idx]) {
                nvgMoveTo(args.vg, box.pos.x + i, box.pos.y);
                nvgLineTo(args.vg, box.pos.x + i, box.pos.y + box.size.y);
            }
        }
        nvgStroke(args.vg);

        // Display X buffer contents
        nvgStrokeColor(args.vg, COLOR_GREEN);
        nvgBeginPath(args.vg);
        nvgMoveTo(args.vg, box.pos.x, box.pos.y + box.size.y);
        for(int i = 0; i < box.size.x; i++) {
            unsigned int idx = module->samples / box.size.x * i;
            nvgLineTo(args.vg, box.pos.x + i, box.pos.y + box.size.y - (module->x[idx] / 10 * box.size.y));
        }
        nvgStroke(args.vg);

        // Display Y buffer contents
        nvgStrokeColor(args.vg, COLOR_BLUE);
        nvgBeginPath(args.vg);
        nvgMoveTo(args.vg, box.pos.x, box.pos.y + box.size.y);
        for(int i = 0; i < box.size.x; i++) {
            unsigned int idx = module->samples / box.size.x * i;
            nvgLineTo(args.vg, box.pos.x + i, box.pos.y + box.size.y - (module->y[idx] / 10 * box.size.y));
        }
        nvgStroke(args.vg);

        // Indicate buffer index
        nvgStrokeColor(args.vg, module->gIn ? COLOR_RED : COLOR_RS_GREY);
        nvgStrokeWidth(args.vg, 1.f);
        nvgBeginPath(args.vg);
        nvgMoveTo(args.vg, box.pos.x + (box.size.x / module->samples * module->idx), box.pos.y);
        nvgLineTo(args.vg, box.pos.x + (box.size.x / module->samples * module->idx), box.pos.y + box.size.y);
        nvgStroke(args.vg);

    };
};

struct RSXYGLRWidget : ModuleWidget {
    RSXYGLR* module;
    Widget* panelBorder;

    RSXYGLRWidget(RSXYGLR *module) {
		INFO("Racket Science: RSXYGLRWidget()");

        setModule(module);
        this->module = module;

        panelBorder = new PanelBorder;
        addChild(panelBorder);

        box.size = Vec(RACK_GRID_WIDTH * 20, RACK_GRID_HEIGHT);
        int middle = box.size.x / 2 + 1;
        int third = box.size.x / 3;

		addParam(createParamCentered<RSButtonMomentaryInvisible>(Vec(box.pos.x + 5, box.pos.y + 5), module, RSXYGLR::THEME_BUTTON));

        addChild(new RSLabelCentered(middle, box.pos.y + 14, "XYGLR - Touchpad Loop Recorder", 15, module));
        //addChild(new RSLabelCentered(middle, box.pos.y + 30, "Touchpad Loop Recorder", 14));
        addChild(new RSLabelCentered(middle, box.size.y - 4, "Racket Science", 12, module)); // >= 4HP
        //addChild(new RSLabelCentered(middle, box.size.y - 15, "Racket", 12));
        //addChild(new RSLabelCentered(middle, box.size.y - 4, "Science", 12));

        int x = 30, y = box.size.y - 50;

        addChild(new RSLabelCentered(x, y, "PHASE", 10));
        addInput(createInputCentered<RSJackMonoIn>(Vec(x, y + 20), module, RSXYGLR::PHASE_IN));

        x += 40;
        addChild(new RSLabelCentered(x, y, "CLEAR", 10));
        addParam(createParamCentered<RSRoundButtonMomentary>(Vec(x, y + 20), module, RSXYGLR::CLEAR_BUTTON));

        x += 40;
        addChild(new RSLabelCentered(x, y, "X", 10));
        addInput(createInputCentered<RSJackMonoIn>(Vec(x, y + 20), module, RSXYGLR::X_IN));

        x += 30;
        addChild(new RSLabelCentered(x, y, "Y", 10));
        addInput(createInputCentered<RSJackMonoIn>(Vec(x, y + 20), module, RSXYGLR::Y_IN));

        x += 30;
        addChild(new RSLabelCentered(x, y, "G", 10));
        addInput(createInputCentered<RSJackMonoIn>(Vec(x, y + 20), module, RSXYGLR::G_IN));

        x += 40;
        addChild(new RSLabelCentered(x, y, "X", 10));
        addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y + 20), module, RSXYGLR::X_OUT));

        x += 30;
        addChild(new RSLabelCentered(x, y, "Y", 10));
        addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y + 20), module, RSXYGLR::Y_OUT));

        x += 30;
        addChild(new RSLabelCentered(x, y, "G", 10));
        addOutput(createOutputCentered<RSJackMonoOut>(Vec(x, y + 20), module, RSXYGLR::G_OUT));

        addChild(new RSTouchPadDisplay(module, 20, 20, 260, 200));
        addChild(new RSXYGBufferDisplay(module, 20, 240, 260, 70));
    }

    void step() override {
        if(!module) return;

        ModuleWidget::step();
    }

    void customDraw(const DrawArgs& args) {}
    #include "RSModuleWidgetDraw.hpp"
};

Model *modelRSXYGLR = createModel<RSXYGLR, RSXYGLRWidget>("RSXYGLR");
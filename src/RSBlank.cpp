#include "plugin.hpp"

#include "RS.hpp"

// struct BlankPanel : Widget {
//     Widget* panelBorder;

//     BlankPanel() {
//         panelBorder = new PanelBorder;
//         addChild(panelBorder);
//     }

//     void step() override {
//         panelBorder->box.size = box.size;
//         Widget::step();
//     }

//     void draw(const DrawArgs& args) {
// 		nvgBeginPath(args.vg);
// 		nvgRoundedRect(args.vg, 0.0, 0.0, box.size.x, box.size.y, 5);
// 		nvgFillColor(args.vg, nvgRGB(0xe6, 0xe6, 0xe6));
// 		nvgFill(args.vg);
// 		Widget::draw(args);
//     }
// };

struct RSBlank : RSModule {
	enum ParamIds {
		THEME_BUTTON,
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	dsp::BooleanTrigger themeTrigger;

	RSBlank() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(THEME_BUTTON, 0.f, 1.f, 0.f, "THEME");
	}

	void process(const ProcessArgs &args) override {
		if(themeTrigger.process(params[THEME_BUTTON].getValue())) {
			RSTheme++;
			if(RSTheme > RSGlobal.themeCount) RSTheme = 1;
		}

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

struct RSBlankWidget : ModuleWidget {
    RSBlank *module;
    Widget *rightHandle;

    RSBlankWidget(RSBlank *module) {
        setModule(module);
        this->module = module;

        box.size = Vec(RACK_GRID_WIDTH * 10, RACK_GRID_HEIGHT);

		addParam(createParamCentered<RSButtonMomentaryInvisible>(Vec(box.pos.x + 5, box.pos.y + 5), module, RSBlank::THEME_BUTTON));

        addChild(new RSLabelCentered(18, box.size.y - 15, "Racket", 12, module));
        addChild(new RSLabelCentered(18, box.size.y - 4, "Science", 12, module));

        ModuleResizeHandle *rightHandle = new ModuleResizeHandle;
        rightHandle->right = true;
        this->rightHandle = rightHandle;
        addChild(rightHandle);
    }

    void step() override {
        rightHandle->box.pos.x = box.size.x - rightHandle->box.size.x;
        ModuleWidget::step();
    }

    void customDraw(const DrawArgs& args) {}
	#include "RSModuleWidgetDraw.hpp"

	json_t *toJson() override {
		json_t* rootJ = ModuleWidget::toJson();

        json_object_set_new(rootJ, "width", json_real(box.size.x));

		return rootJ;
	}

	void fromJson(json_t* rootJ) override {
        ModuleWidget::fromJson(rootJ);

        json_t* widthJ = json_object_get(rootJ, "width");
        if(widthJ) box.size.x = json_number_value(widthJ);
	}
};

Model* modelRSBlank = createModel<RSBlank, RSBlankWidget>("RSBlank");
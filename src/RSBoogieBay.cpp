#include "plugin.hpp"

#include "RS.hpp"

struct RSBoogieBay : RSModule {
	enum ParamIds {
        THEME_BUTTON,
		NUM_PARAMS
	};
	enum InputIds {
		INA_INPUT,
		INB_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTA_OUTPUT,
		OUTB_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

    dsp::BooleanTrigger themeTrigger;

    int vrangea = 4;
    int vrangeb = 2;

    bool menuaChanged = true;
    bool menubChanged = true;

	RSBoogieBay() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(THEME_BUTTON, 0.f, 1.f, 0.f, "THEME");
	}

	void process(const ProcessArgs &args) override {
		if(themeTrigger.process(params[THEME_BUTTON].getValue())) {
			RSTheme++;
			if(RSTheme > RSGlobal.themeCount) RSTheme = 1;
		}

        outputs[OUTA_OUTPUT].setVoltage(inputs[INA_INPUT].getVoltage());
        outputs[OUTB_OUTPUT].setVoltage(inputs[INB_INPUT].getVoltage());
	}

    void onReset() override {
    }

    json_t* dataToJson() override {
        json_t* rootJ = json_object();

        json_object_set_new(rootJ, "theme", json_integer(RSTheme));

        json_object_set_new(rootJ, "vrangea", json_integer(vrangea));
        json_object_set_new(rootJ, "vrangeb", json_integer(vrangeb));

        return rootJ;
    }

    void dataFromJson(json_t* rootJ) override {
        json_t* themeJ = json_object_get(rootJ, "theme");

        if(themeJ) RSTheme = json_integer_value(themeJ);

        json_t* vrangeaJ = json_object_get(rootJ, "vrangea");
        json_t* vrangebJ = json_object_get(rootJ, "vrangeb");

        if(vrangeaJ) vrangea = json_integer_value(vrangeaJ);
        if(vrangebJ) vrangeb = json_integer_value(vrangebJ);
    }
};


struct RSBoogieBayWidget : ModuleWidget {
    RSBoogieBay* module;

    PortWidget *ina, *inb;

    RSLabel* topScaleaLabel;
    RSLabel* midScaleaLabel;
    RSLabel* botScaleaLabel;

    RSLabel* topScalebLabel;
    RSLabel* midScalebLabel;
    RSLabel* botScalebLabel;
   
	RSBoogieBayWidget(RSBoogieBay *module) {
        INFO("Racket Science: RSBoogieBayWidget()");

		setModule(module);
        this->module = module;

        box.size.x = mm2px(5.08 * 5);
        int middle = box.size.x / 2 + 1;

		addParam(createParamCentered<RSButtonMomentaryInvisible>(Vec(box.pos.x + 5, box.pos.y + 5), module, RSBoogieBay::THEME_BUTTON));

        addChild(new RSLabelCentered(middle, box.pos.y + 13, "BOOGIE", 14, module));
        addChild(new RSLabelCentered(middle, box.pos.y + 25, "BAY", 14, module));

        addChild(new RSLabelCentered(box.size.x / 2, box.size.y - 6, "Racket Science", 12, module));

        ina = createInputCentered<RSJackMonoIn>(Vec(22, 175), module, RSBoogieBay::INA_INPUT);
		addInput(ina);
        inb = createInputCentered<RSJackMonoIn>(Vec(52, 175), module, RSBoogieBay::INB_INPUT);
        addInput(inb);

        addOutput(createOutputCentered<RSJackMonoOut>(Vec(22, 345), module, RSBoogieBay::OUTA_OUTPUT));
        addOutput(createOutputCentered<RSJackMonoOut>(Vec(52, 345), module, RSBoogieBay::OUTB_OUTPUT));

        addChild(topScaleaLabel = new RSLabel(3,  58, "0", 10, COLOR_GREEN));
        addChild(midScaleaLabel = new RSLabel(3, 177, "0", 10, COLOR_RS_GREY));
        addChild(botScaleaLabel = new RSLabel(3, 296, "0", 10, COLOR_RED));

        addChild(topScalebLabel = new RSLabel(67,  58, "0", 10, COLOR_GREEN));
        addChild(midScalebLabel = new RSLabel(67, 177, "0", 10, COLOR_RS_GREY));
        addChild(botScalebLabel = new RSLabel(67, 296, "0", 10, COLOR_RED));
    }

    void customDraw(const DrawArgs& args) {
        int top = 55, bottom = 295;
        nvgLineCap(args.vg, NVG_ROUND);

        // Socket scale markers
        nvgStrokeColor(args.vg, COLOR_RS_GREY);
        nvgStrokeWidth(args.vg, 1);
        for(int y = top, cnt = 0; y <= bottom; y += (bottom - top) / 10, cnt++) {
            nvgBeginPath(args.vg);
            switch(cnt) {
                case 0: case 5: case 10:
                    nvgMoveTo(args.vg, 16, y);
                    nvgLineTo(args.vg, 58, y);
                    break;
                default:
                    nvgMoveTo(args.vg, 30, y);
                    nvgLineTo(args.vg, 44, y);
                    break;
            }
            nvgStroke(args.vg);
        }

        nvgStrokeColor(args.vg, COLOR_BLACK);
        nvgStrokeWidth(args.vg, 5);

        // Left socket slot
        nvgBeginPath(args.vg);
        nvgMoveTo(args.vg, 22, top);
        nvgLineTo(args.vg, 22, bottom);
        nvgStroke(args.vg);

        // Right socket slot
        nvgBeginPath(args.vg);
        nvgMoveTo(args.vg, 52, top);
        nvgLineTo(args.vg, 52, bottom);
        nvgStroke(args.vg);
    }
   	#include "RSModuleWidgetDraw.hpp"

    void step() override {
        if(!module) return;

        if(module->menuaChanged) {
            switch(module->vrangea) {
                case 0: // 0 - 1
                    topScaleaLabel->text = "1";   topScaleaLabel->box.pos.x = 3; topScaleaLabel->color = COLOR_GREEN;
                    midScaleaLabel->text = ".5";  midScaleaLabel->box.pos.x = 3; midScaleaLabel->color = COLOR_GREEN;
                    botScaleaLabel->text = "0";   botScaleaLabel->box.pos.x = 3; botScaleaLabel->color = COLOR_RS_GREY;
                    break;
                case 1: // 0 - 5
                    topScaleaLabel->text = "5";   topScaleaLabel->box.pos.x = 3; topScaleaLabel->color = COLOR_GREEN;
                    midScaleaLabel->text = "2.5"; midScaleaLabel->box.pos.x = 3; midScaleaLabel->color = COLOR_GREEN;
                    botScaleaLabel->text = "0";   botScaleaLabel->box.pos.x = 3; botScaleaLabel->color = COLOR_RS_GREY;
                    break;
                case 2: // 0 - 10
                    topScaleaLabel->text = "10";  topScaleaLabel->box.pos.x = 3; topScaleaLabel->color = COLOR_GREEN;
                    midScaleaLabel->text = "5";   midScaleaLabel->box.pos.x = 3; midScaleaLabel->color = COLOR_GREEN;
                    botScaleaLabel->text = "0";   botScaleaLabel->box.pos.x = 3; botScaleaLabel->color = COLOR_RS_GREY;
                    break;
                case 3: // -2 - 2
                    topScaleaLabel->text = "2";   topScaleaLabel->box.pos.x = 3; topScaleaLabel->color = COLOR_GREEN;
                    midScaleaLabel->text = "0";   midScaleaLabel->box.pos.x = 3; midScaleaLabel->color = COLOR_RS_GREY;
                    botScaleaLabel->text = "2";   botScaleaLabel->box.pos.x = 3; botScaleaLabel->color = COLOR_RED;
                    break;
                case 4: // -5 - 5
                    topScaleaLabel->text = "5";   topScaleaLabel->box.pos.x = 3; topScaleaLabel->color = COLOR_GREEN;
                    midScaleaLabel->text = "0";   midScaleaLabel->box.pos.x = 3; midScaleaLabel->color = COLOR_RS_GREY;
                    botScaleaLabel->text = "5";   botScaleaLabel->box.pos.x = 3; botScaleaLabel->color = COLOR_RED;
                    break;
                case 5: // -10 - 10
                    topScaleaLabel->text = "10";  topScaleaLabel->box.pos.x = 3; topScaleaLabel->color = COLOR_GREEN;
                    midScaleaLabel->text = "0";   midScaleaLabel->box.pos.x = 3; midScaleaLabel->color = COLOR_RS_GREY;
                    botScaleaLabel->text = "10";  botScaleaLabel->box.pos.x = 3; botScaleaLabel->color = COLOR_RED;
                    break;
            }
            module->menuaChanged = false;
        }

        if(module->menubChanged) {
            switch(module->vrangeb) {
                case 0: // 0 - 1
                    topScalebLabel->text = "1";   topScalebLabel->box.pos.x = 67; topScalebLabel->color = COLOR_GREEN;
                    midScalebLabel->text = ".5";  midScalebLabel->box.pos.x = 65; midScalebLabel->color = COLOR_GREEN;
                    botScalebLabel->text = "0";   botScalebLabel->box.pos.x = 67; botScalebLabel->color = COLOR_RS_GREY;
                    break;
                case 1: // 0 - 5
                    topScalebLabel->text = "5";   topScalebLabel->box.pos.x = 67; topScalebLabel->color = COLOR_GREEN;
                    midScalebLabel->text = "2.5"; midScalebLabel->box.pos.x = 63; midScalebLabel->color = COLOR_GREEN;
                    botScalebLabel->text = "0";   botScalebLabel->box.pos.x = 67; botScalebLabel->color = COLOR_RS_GREY;
                    break;
                case 2: // 0 - 10
                    topScalebLabel->text = "10";  topScalebLabel->box.pos.x = 64; topScalebLabel->color = COLOR_GREEN;
                    midScalebLabel->text = "5";   midScalebLabel->box.pos.x = 67; midScalebLabel->color = COLOR_GREEN;
                    botScalebLabel->text = "0";   botScalebLabel->box.pos.x = 67; botScalebLabel->color = COLOR_RS_GREY;
                    break;
                case 3: // -2 - 2
                    topScalebLabel->text = "2";   topScalebLabel->box.pos.x = 67; topScalebLabel->color = COLOR_GREEN;
                    midScalebLabel->text = "0";   midScalebLabel->box.pos.x = 67; midScalebLabel->color = COLOR_RS_GREY;
                    botScalebLabel->text = "2";   botScalebLabel->box.pos.x = 67; botScalebLabel->color = COLOR_RED;
                    break;
                case 4: // -5 - 5
                    topScalebLabel->text = "5";   topScalebLabel->box.pos.x = 67; topScalebLabel->color = COLOR_GREEN;
                    midScalebLabel->text = "0";   midScalebLabel->box.pos.x = 67; midScalebLabel->color = COLOR_RS_GREY;
                    botScalebLabel->text = "5";   botScalebLabel->box.pos.x = 67; botScalebLabel->color = COLOR_RED;
                    break;
                case 5: // -10 - 10
                    topScalebLabel->text = "10";  topScalebLabel->box.pos.x = 64; topScalebLabel->color = COLOR_GREEN;
                    midScalebLabel->text = "0";   midScalebLabel->box.pos.x = 67; midScalebLabel->color = COLOR_RS_GREY;
                    botScalebLabel->text = "10";  botScalebLabel->box.pos.x = 64; botScalebLabel->color = COLOR_RED;
                    break;
            }
            module->menubChanged = false;
        }

        float inav = RSclamp(module->inputs[RSBoogieBay::INA_INPUT].getVoltage(), -10.f, 10.f);
        float inbv = RSclamp(module->inputs[RSBoogieBay::INB_INPUT].getVoltage(), -10.f, 10.f);
        int yposa = 0, yposb = 0;

        switch(module->vrangea) {
            case 0: // 0 - 1
                inav = clamp(inav, 0.f, 1.f);
                yposa = mm2px(95 - inav * 80);
                break;
            case 1: // 0 - 5
                inav = clamp(inav, 0.f, 5.f);
                yposa = mm2px(95 - inav * 16);
                break;
            case 2: // 0 - 10
                inav = clamp(inav, 0.f, 10.f);
                yposa = mm2px(95 - inav * 8);
                break;
            case 3: // -2 - 2
                inav = clamp(inav, -2.f, 2.f);
                inav += 2;
                yposa = mm2px(95 - inav * 20);
                break;
            case 4: // -5 - 5
                inav = clamp(inav, -5.f, 5.f);
                inav += 5;
                yposa = mm2px(95 - inav * 8);
                break;
            case 5: // -10 - 10
                inav = clamp(inav, -10.f, 10.f);
                inav += 10;
                yposa = mm2px(95 - inav * 4);
                break;
            default:
                break;
        };

        switch(module->vrangeb) {
            case 0: // 0 - 1
                inbv = clamp(inbv, 0.f, 1.f);
                yposb = mm2px(95 - inbv * 80);
                break;
            case 1: // 0 - 5
                inbv = clamp(inbv, 0.f, 5.f);
                yposb = mm2px(95 - inbv * 16);
                break;
            case 2: // 0 - 10
                inbv = clamp(inbv, 0.f, 10.f);
                yposb = mm2px(95 - inbv * 8);
                break;
            case 3: // -2 - 2
                inbv = clamp(inbv, -2.f, 2.f);
                inbv += 2;
                yposb = mm2px(95 - inbv * 20);
                break;
            case 4: // -5 - 5
                inbv = clamp(inbv, -5.f, 5.f);
                inbv += 5;
                yposb = mm2px(95 - inbv * 8);
                break;
            case 5: // -10 - 10
                inbv = clamp(inbv, -10.f, 10.f);
                inbv += 10;
                yposb = mm2px(95 - inbv * 4);
                break;
            default:
                break;
        };

        ina->box.pos.y = yposa;
        inb->box.pos.y = yposb;

        ModuleWidget::step();
    }


    void appendContextMenu(Menu* menu) override {
        RSBoogieBay* module = dynamic_cast<RSBoogieBay*>(this->module);

        std::string rangeNames[6] = {"0V - 1V", "0V - 5V", "0V - 10V", "-2V - 2V", "-5V - 5V", "-10V - 10V"};

        menu->addChild(new MenuEntry);

        menu->addChild(createMenuLabel("Voltage Range A"));

        struct RangeaItem : MenuItem {
            RSBoogieBay* module;
            int vrangea;
            void onAction(const event::Action& e) override {
                module->vrangea = vrangea;
                module->menuaChanged = true;
            }
        };

        for(int i = 0; i < 6; i++) {
            RangeaItem* rangeaItem = createMenuItem<RangeaItem>(rangeNames[i]);
            rangeaItem->rightText = CHECKMARK(module->vrangea == i);
            rangeaItem->module = module;
            rangeaItem->vrangea = i;
            menu->addChild(rangeaItem);
        }

        menu->addChild(createMenuLabel("Voltage Range B"));

        struct RangebItem : MenuItem {
            RSBoogieBay* module;
            int vrangeb;
            void onAction(const event::Action& e) override {
                module->vrangeb = vrangeb;
                module->menubChanged = true;
            }
        };

        for(int i = 0; i < 6; i++) {
            RangebItem* rangebItem = createMenuItem<RangebItem>(rangeNames[i]);
            rangebItem->rightText = CHECKMARK(module->vrangeb == i);
            rangebItem->module = module;
            rangebItem->vrangeb = i;
            menu->addChild(rangebItem);
        }
    }
};


Model *modelRSBoogieBay = createModel<RSBoogieBay, RSBoogieBayWidget>("RSBoogieBay");

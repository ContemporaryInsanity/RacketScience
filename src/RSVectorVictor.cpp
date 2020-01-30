#include "plugin.hpp"

#include "RS.hpp"


struct RSVectorVictor : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		PHASEA_INPUT,
		WRITEA_INPUT,
		INA_INPUT,
		PHASEB_INPUT,
		WRITEB_INPUT,
		INB_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTA_OUTPUT,
		OUTB_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		WRITEA_LIGHT,
		WRITEB_LIGHT,
		NUM_LIGHTS
	};

	RSVectorVictor() {
        INFO("Racket Science: %i params  %i inputs  %i outputs  %i lights", NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

    #define SAMPLES 1000
    float bufferA[SAMPLES] = {};
    float bufferB[SAMPLES] = {};

    void onReset() override {
        std::memset(bufferA, 0, sizeof(bufferA));
        std::memset(bufferB, 0, sizeof(bufferB));
    }

	void process(const ProcessArgs &args) override {
        unsigned int idxA, idxB;
        float inA, inB;

        idxA = (unsigned int)abs(inputs[PHASEA_INPUT].getVoltage() * SAMPLES / 10); if(idxA > SAMPLES - 1) idxA = SAMPLES - 1;
        idxB = (unsigned int)abs(inputs[PHASEB_INPUT].getVoltage() * SAMPLES / 10); if(idxB > SAMPLES - 1) idxB = SAMPLES - 1;

        inA = inputs[INA_INPUT].getVoltage();
        inB = inputs[INB_INPUT].getVoltage();

        if(inputs[WRITEA_INPUT].getVoltage()) bufferA[idxA] = inA;
        if(inputs[WRITEB_INPUT].getVoltage()) bufferB[idxB] = inB;

        outputs[OUTA_OUTPUT].setVoltage(bufferA[idxA]);
        outputs[OUTB_OUTPUT].setVoltage(bufferB[idxB]);

        lights[WRITEA_LIGHT].setSmoothBrightness(inputs[WRITEA_INPUT].getVoltage() ? 1 : 0, 10);
        lights[WRITEB_LIGHT].setSmoothBrightness(inputs[WRITEB_INPUT].getVoltage() ? 1 : 0, 10);
	}

    json_t* dataToJson() override {
        INFO("Racket Science: d2j()");
        json_t* rootJ = json_object();

        json_t* samplesAJ = json_array();
        json_t* samplesBJ = json_array();

        for(int i = 0; i < SAMPLES; i++) {
            json_array_append_new(samplesAJ, json_real(bufferA[i]));
            json_array_append_new(samplesBJ, json_real(bufferB[i]));
        }

        json_object_set_new(rootJ, "samplesA", samplesAJ);
        json_object_set_new(rootJ, "samplesB", samplesBJ);

        return rootJ;
    }

    void dataFromJson(json_t* rootJ) override {
        INFO("Racket Science: dfj()");
        json_t* samplesAJ = json_object_get(rootJ, "samplesA");
        json_t* samplesBJ = json_object_get(rootJ, "samplesB");

        if(samplesAJ) {
            for(int i = 0; i < SAMPLES; i++) {
                bufferA[i] = json_number_value(json_array_get(samplesAJ, i));
                bufferB[i] = json_number_value(json_array_get(samplesBJ, i));
            }
        }
    }
};


struct RSVectorVictorWidget : ModuleWidget {
    RSVectorVictor *module;

	RSVectorVictorWidget(RSVectorVictor *module) {
        INFO("Racket Science: RSVectorVictorWidget()");

		setModule(module);
        this->module = module;

		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/RSVectorVictor.svg")));

		addInput(createInputCentered<RSJackMonoIn>(mm2px(Vec(7.398, 33.299)), module, RSVectorVictor::PHASEA_INPUT));
		addInput(createInputCentered<RSJackMonoIn>(mm2px(Vec(18.102, 33.299)), module, RSVectorVictor::WRITEA_INPUT));
		addInput(createInputCentered<RSJackMonoIn>(mm2px(Vec(7.398, 52.578)), module, RSVectorVictor::INA_INPUT));
		addInput(createInputCentered<RSJackMonoIn>(mm2px(Vec(7.393, 80.144)), module, RSVectorVictor::PHASEB_INPUT));
		addInput(createInputCentered<RSJackMonoIn>(mm2px(Vec(18.097, 80.144)), module, RSVectorVictor::WRITEB_INPUT));
		addInput(createInputCentered<RSJackMonoIn>(mm2px(Vec(7.393, 99.422)), module, RSVectorVictor::INB_INPUT));

		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(18.102, 52.578)), module, RSVectorVictor::OUTA_OUTPUT));
		addOutput(createOutputCentered<RSJackMonoOut>(mm2px(Vec(18.097, 99.422)), module, RSVectorVictor::OUTB_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(18.102, 23.655)), module, RSVectorVictor::WRITEA_LIGHT));
		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(18.097, 70.5)), module, RSVectorVictor::WRITEB_LIGHT));

        INFO("Racket Science: exiting RSVectorVictorWidget()");
	}
};


Model *modelRSVectorVictor = createModel<RSVectorVictor, RSVectorVictorWidget>("RSVectorVictor");

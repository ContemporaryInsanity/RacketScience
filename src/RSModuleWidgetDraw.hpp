
// Draws panel background instead of using SVG file
// Calls customDraw() should we need to draw anything else
void draw(const DrawArgs& args) override {
    nvgStrokeColor(args.vg, COLOR_RS_BRONZE);
    switch(theme) {
        case 0: nvgFillColor(args.vg, COLOR_RS_BG); break;
        case 1: nvgFillColor(args.vg, nvgRGB(0x60, 0x60, 0x00)); break;
        default: nvgFillColor(args.vg, COLOR_BLACK);
    }

    nvgStrokeWidth(args.vg, 2);
    nvgBeginPath(args.vg);
    nvgRoundedRect(args.vg, 1, 1, box.size.x - 1, box.size.y - 1, 5);
    nvgStroke(args.vg);
    nvgFill(args.vg);

    customDraw(args);

    ModuleWidget::draw(args);
}

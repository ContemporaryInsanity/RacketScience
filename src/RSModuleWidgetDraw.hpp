// Draws panel background instead of using SVG file
// Calls customDraw() should we need to draw anything else
void draw(const DrawArgs& args) override {
    nvgStrokeColor(args.vg, COLOR_RS_BRONZE);
    switch(RSGlobal.theme) {
        case 0: nvgFillColor(args.vg, COLOR_RS_BG); break;
        case 1: nvgFillColor(args.vg, nvgRGB(0x17, 0x69, 0x49)); break; // Green
        case 2: nvgFillColor(args.vg, nvgRGB(0x78, 0x1f, 0x2f)); break; // Red
        case 3: nvgFillColor(args.vg, nvgRGB(0x0d, 0x2d, 0x59)); break; // Blue
        case 4: nvgFillColor(args.vg, nvgRGB(0x78, 0x7d, 0x14)); break; // Yellow
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

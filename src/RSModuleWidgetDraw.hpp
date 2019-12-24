// Draws panel background instead of using SVG file
// Calls customDraw() should we need to draw anything else
void draw(const DrawArgs& args) override {
    nvgStrokeColor(args.vg, COLOR_RS_BRONZE);

    if(module && module->RSTheme > 0) nvgFillColor(args.vg, RSGlobal.themes[module->RSTheme - 1].bgColor);
    else                              nvgFillColor(args.vg, RSGlobal.themes[RSGlobal.themeIdx].bgColor);

    nvgStrokeWidth(args.vg, 2);
    nvgBeginPath(args.vg);
    nvgRoundedRect(args.vg, 1, 1, box.size.x - 1, box.size.y - 1, 5);
    nvgStroke(args.vg);
    nvgFill(args.vg);

    // Maybe look into gradient background option controlled by Ground Control

    customDraw(args);

    ModuleWidget::draw(args);
}

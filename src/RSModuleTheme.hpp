// Processes theme switching, should be included in process()
if(themeTrigger.process(params[THEME_BUTTON].getValue())) {
    RSGlobal.theme++;
    if(RSGlobal.theme > RSGlobal.themeCount) RSGlobal.theme = 0;
    saveDefaultTheme(RSGlobal.theme);
}

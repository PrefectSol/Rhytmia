#ifndef RHYTMIA_H
#define RHYTMIA_H

#include <map>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <filesystem>

#include <Windows.h>

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include "OsuClient.h"
#include "WebWrapper.h"

#undef main

namespace fs = std::filesystem;

class Rhytmia
{
public:
	explicit Rhytmia();

	~Rhytmia();

	void run();

	int exitCode();

private:
    enum AppCode
    {
        webWrapperError = -5,
        imageError = -4,
        osuClientError = -3,
        ttfError = -2,
        sdlError = -1,
        success = 0,

        openConfigFileError,
        openPluginsFileError,
    };

    typedef struct Color
    {
        SDL_Color text;
        SDL_Color sText;
        SDL_Color background;
    } Color;

    AppCode m_appCode;

    OsuClient *m_osuClient;

    WebWrapper *m_webWrapper;

    SDL_Window *m_window;

    SDL_Renderer *m_renderer;

    TTF_Font *m_font;

    TTF_Font *m_smallFont;

    SDL_Surface *m_iconSurface;

    Color m_colors;

    std::vector<std::string> m_menuButtons;

    cfg m_config;

    int m_marginH, m_marginW;

    int m_aboutPageW, m_aboutPageH;

    void getWindowSizes(int *width, int *height);

    void getWindowPos(int *x, int *y);

    void renderWindow(int buttonCount, int selectedButtonIndex, int windowWidth, int windowHeight);

    AppCode initialize(int *windowWidth, int *windowHeight, int *x, int *y);

    AppCode aboutButtonAction(SDL_Event &event, int *selectedIndex, int buttonIndexes, int x, int y);

    AppCode parseConfig();

    AppCode initButtons();

    AppCode getPlugins(std::vector<std::string> *toInstall, std::vector<std::string> *installed);

    AppCode getAllPlugins(std::vector<std::string> *all);

    SDL_Color getColor(const char *type);

    std::string Rhytmia_GetError();
};

#endif // !RHYTMIA_H

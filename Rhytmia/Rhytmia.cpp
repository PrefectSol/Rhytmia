#include "Rhytmia.h"

Rhytmia::Rhytmia()
    : m_appCode(AppCode()),
    m_osuClient(NULL),
    m_webWrapper(NULL),
    m_window(NULL),
    m_renderer(NULL),
    m_font(NULL),
    m_iconSurface(NULL),
    m_marginH(5), m_marginW(5),
    m_aboutPageH(200), m_aboutPageW(150),
    m_installPluginsPageH(35), m_installPluginsPageW(100)
{
    m_appCode = parseConfig();
    if (m_appCode != AppCode::success)
    {
        return;
    }

    m_osuClient = OsuClientOpen(m_config);
    if (m_osuClient == NULL)
    {
        m_appCode = AppCode::osuClientError;
        return;
    }

    m_webWrapper = WebWrapperOpen();
    if (m_webWrapper == NULL)
    {
        m_appCode = AppCode::webWrapperError;
        return;
    }

    m_appCode = initButtons();
    if (m_appCode != AppCode::success)
    {
        return;
    }

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        m_appCode = AppCode::sdlError;
        return;
    }

    m_iconSurface = IMG_Load(m_config["APP"]["icon_path"].c_str());
    if (m_iconSurface == NULL)
    {
        m_appCode = AppCode::imageError;
    }

    if (TTF_Init() < 0)
    {
        m_appCode = AppCode::ttfError;
    }

    m_font = TTF_OpenFont(m_config["APP"]["font_path"].c_str(), std::stoi(m_config["APP"]["font_size"]));
    if (m_font == NULL)
    {
        m_appCode = AppCode::ttfError;
        return;
    }

    m_smallFont = TTF_OpenFont(m_config["APP"]["font_path"].c_str(), std::stoi(m_config["APP"]["font_smallsize"]));
    if (m_smallFont == NULL)
    {
        m_appCode = AppCode::ttfError;
        return;
    }
}

Rhytmia::~Rhytmia()
{
    WebWrapperClose(m_webWrapper);
    OsuClientClose(m_osuClient);

    TTF_CloseFont(m_font);
    TTF_CloseFont(m_smallFont);
    TTF_Quit();

    SDL_FreeSurface(m_iconSurface);

    SDL_DestroyRenderer(m_renderer);

    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

void Rhytmia::run()
{
    if (m_appCode != AppCode::success)
    {
        return;
    }

    int windowWidth, windowHeight, x, y;
    m_appCode = initialize(&windowWidth, &windowHeight, &x, &y);

    if (m_appCode != AppCode::success)
    {
        return;
    }

    const int buttonCount = m_menuButtons.size();
    const int buttonIndexes = buttonCount - 1;
    const int exitButton = buttonIndexes;
    const int aboutButton = buttonIndexes - 1;
    const int installPluginsButton = int(m_toInstallPlugins.size() != 0) - 1;

    int selectedButtonIndex = 0;

    bool quit = false;
    bool isShow = true;
    bool prevKeyPressed = false;

    SDL_Event event;
    while (!quit)
    {
        if (GetAsyncKeyState(VK_MENU) & 0x8000 && GetAsyncKeyState(VK_F11) & 0x8000) 
        {
            if (!prevKeyPressed) 
            {
                if (isShow) 
                {
                    SDL_HideWindow(m_window);
                }
                else 
                {
                    SDL_ShowWindow(m_window);
                }

                isShow = !isShow;
            }

            prevKeyPressed = true;
        }
        else 
        {
            prevKeyPressed = false;
        }

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_UP:
                    if (selectedButtonIndex > 0)
                    {
                        selectedButtonIndex--;
                    }
                    break;
                case SDLK_DOWN:
                    if (selectedButtonIndex < buttonIndexes)
                    {
                        selectedButtonIndex++;
                    }
                    break;
                case SDLK_RETURN:
                case SDLK_RETURN2:
                    if (selectedButtonIndex == exitButton)
                    {
                        quit = true;
                    }
                    else if (selectedButtonIndex == aboutButton)
                    {
                        m_appCode = aboutButtonAction(event, &selectedButtonIndex, buttonIndexes, x + windowWidth + m_marginW, y);
                        if (m_appCode != AppCode::success)
                        {
                            return;
                        }
                    }
                    else if (selectedButtonIndex == installPluginsButton)
                    {
                        m_appCode = installPluginsButtonAction(event, &selectedButtonIndex, buttonIndexes, x + windowWidth + m_marginW, y);
                        if (m_appCode != AppCode::success)
                        {
                            return;
                        }
                    }
                    break;
                default:
                    break;
                }
            }
        }

        renderWindow(buttonCount, selectedButtonIndex, windowWidth, windowHeight);
    }
}

int Rhytmia::exitCode()
{
    if (m_appCode != AppCode::success)
    {
        switch (m_appCode)
        {
        case AppCode::webWrapperError:
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "WebWrapper_ERROR", WebWrapper_GetError().c_str(), NULL);
            break;

        case AppCode::osuClientError:
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "OsuClient_ERROR", OsuClient_GetError().c_str(), NULL);
            break;

        case AppCode::ttfError:
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "TTF_ERROR", TTF_GetError(), NULL);
            break;

        case AppCode::sdlError:
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL_ERROR", SDL_GetError(), NULL);
            break;

        case AppCode::imageError:
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "IMAGE_ERROR", IMG_GetError(), NULL);
            break;

        default:
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "RHYTMIA_ERROR", Rhytmia_GetError().c_str(), NULL);
            break;
        }
    }

	return m_appCode;
}

void Rhytmia::getWindowSizes(int *width, int *height, const std::vector<std::string> &buttons)
{
    int totalHeight = 0;
    int maxWidth = 0;

    for (const std::string &name : buttons)
    {
        int textWidth, textHeight;
        TTF_SizeText(m_font, name.c_str(), &textWidth, &textHeight);

        totalHeight += textHeight;
        maxWidth = max(maxWidth, textWidth);
    }

    *width = maxWidth + 2 * m_marginW;
    *height = totalHeight + buttons.size() * m_marginH;
}

void Rhytmia::getWindowPos(int *x, int *y)
{
    *x = std::stoi(m_config["APP"]["x_pos"]);
    *y = std::stoi(m_config["APP"]["y_pos"]);

    if (*x < 0)
    {
        *x = SDL_WINDOWPOS_UNDEFINED;
    }

    if (*y < 0)
    {
        *y = SDL_WINDOWPOS_UNDEFINED;
    }
}

Rhytmia::AppCode Rhytmia::aboutButtonAction(SDL_Event &event, int *selectedIndex, int buttonIndexes, int x, int y)
{
    int contactWidth, contactHeight;
    TTF_SizeText(m_smallFont, "Discord: prefectsol", &contactWidth, &contactHeight);
    m_aboutPageW = contactWidth + 2 * m_marginW;

    SDL_Window *childWindow = SDL_CreateWindow((m_config["APP"]["app_name"] + "-about").c_str(),
                                                x, y, m_aboutPageW, m_aboutPageH,
                                                SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI |
                                                SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALWAYS_ON_TOP);
    if (childWindow == NULL)
    {
        return AppCode::sdlError;
    }

    SDL_SetWindowIcon(childWindow, m_iconSurface);
    SDL_SetWindowOpacity(childWindow, std::stof(m_config["APP"]["window_opacity"]));

    SDL_Renderer *childRenderer = SDL_CreateRenderer(childWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (m_renderer == NULL)
    {
        return AppCode::sdlError;
    }

    SDL_SetRenderDrawColor(childRenderer, m_colors.background.r, m_colors.background.g, m_colors.background.b, m_colors.background.a);

    SDL_Surface *appnameSurface = TTF_RenderText_Solid(m_font, m_config["APP"]["app_name"].c_str(), m_colors.text);
    SDL_Texture *appnameTexture = SDL_CreateTextureFromSurface(childRenderer, appnameSurface);
    int appnameWidth, appnameHeight;
    TTF_SizeText(m_font, m_config["APP"]["app_name"].c_str(), &appnameWidth, &appnameHeight);
    const SDL_Rect appnameRect = { (m_aboutPageW - appnameWidth) / 2, m_marginH, appnameSurface->w, appnameSurface->h };

    SDL_Texture *previewTexture = SDL_CreateTextureFromSurface(childRenderer, m_iconSurface);
    const int previewHeight = (m_marginH * 3 + appnameHeight) / 2;
    const SDL_Rect previewRect = { (m_aboutPageW - 100) / 2, previewHeight, 100, 100 };

    SDL_Surface *devnameSurface = TTF_RenderText_Solid(m_smallFont, "by Prefect", m_colors.text);
    SDL_Texture *devnameTexture = SDL_CreateTextureFromSurface(childRenderer, devnameSurface);
    int devnameWidth, devnameHeight;
    TTF_SizeText(m_smallFont, "by Prefect", &devnameWidth, &devnameHeight);
    const SDL_Rect devnameRect = { (m_aboutPageW - devnameWidth) / 2, previewHeight + 100, devnameSurface->w, devnameSurface->h};

    const std::string versionText = "version: " + m_config["APP"]["version"];
    SDL_Surface *versionSurface = TTF_RenderText_Solid(m_smallFont, versionText.c_str(), m_colors.text);
    SDL_Texture *versionTexture = SDL_CreateTextureFromSurface(childRenderer, versionSurface);
    int versionWidth, versionHeight;
    TTF_SizeText(m_smallFont, versionText.c_str(), &versionWidth, &versionHeight);
    const int versionPosH = m_aboutPageH - versionHeight;
    const SDL_Rect versionRect = { m_marginW, versionPosH, versionSurface->w, versionSurface->h };

    SDL_Surface *contactSurface = TTF_RenderText_Solid(m_smallFont, "Discord: prefectsol", m_colors.text);
    SDL_Texture *contactTexture = SDL_CreateTextureFromSurface(childRenderer, contactSurface);
    const SDL_Rect contactRect = { m_marginW, (versionPosH + previewHeight + 100) / 2, contactSurface->w, contactSurface->h };

    SDL_RenderClear(childRenderer);
    SDL_RenderCopy(childRenderer, previewTexture, NULL, &previewRect);
    SDL_RenderCopy(childRenderer, appnameTexture, NULL, &appnameRect);
    SDL_RenderCopy(childRenderer, devnameTexture, NULL, &devnameRect);
    SDL_RenderCopy(childRenderer, versionTexture, NULL, &versionRect);
    SDL_RenderCopy(childRenderer, contactTexture, NULL, &contactRect);

    SDL_RenderPresent(childRenderer);

    while (SDL_WaitEvent(&event))
    {    
        if (event.type == SDL_KEYDOWN && (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_DOWN))
        {
            if (event.key.keysym.sym == SDLK_UP && *selectedIndex > 0)
            {
                (*selectedIndex)--;
            }
            else if (event.key.keysym.sym == SDLK_DOWN && *selectedIndex < buttonIndexes)
            {
                (*selectedIndex)++;
            }

            SDL_HideWindow(childWindow);
            break;
        }
    }

    SDL_DestroyTexture(contactTexture);
    SDL_FreeSurface(contactSurface);

    SDL_DestroyTexture(versionTexture);
    SDL_FreeSurface(versionSurface);

    SDL_DestroyTexture(devnameTexture);
    SDL_FreeSurface(devnameSurface);

    SDL_DestroyTexture(appnameTexture);
    SDL_FreeSurface(appnameSurface);

    SDL_DestroyTexture(previewTexture);

    SDL_DestroyRenderer(childRenderer);
    SDL_DestroyWindow(childWindow);
   
    return AppCode::success;
}

Rhytmia::AppCode Rhytmia::installPluginsButtonAction(SDL_Event &event, int *selectedIndex, int buttonIndexes, int x, int y)
{
    SDL_Window *childWindow = SDL_CreateWindow((m_config["APP"]["app_name"] + "-install-plugins").c_str(),
                                                x, y, m_installPluginsPageW, m_installPluginsPageH,
                                                SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI |
                                                SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALWAYS_ON_TOP);
    if (childWindow == NULL)
    {
        return AppCode::sdlError;
    }

    SDL_SetWindowIcon(childWindow, m_iconSurface);
    SDL_SetWindowOpacity(childWindow, std::stof(m_config["APP"]["window_opacity"]));

    SDL_Renderer *childRenderer = SDL_CreateRenderer(childWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (m_renderer == NULL)
    {
        return AppCode::sdlError;
    }

    SDL_SetRenderDrawColor(childRenderer, m_colors.background.r, m_colors.background.g, m_colors.background.b, m_colors.background.a);

    SDL_RenderClear(childRenderer);
    SDL_RenderPresent(childRenderer);

    while (SDL_WaitEvent(&event))
    {
        if (event.type == SDL_KEYDOWN && (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_DOWN))
        {
            if (event.key.keysym.sym == SDLK_UP && *selectedIndex > 0)
            {
                (*selectedIndex)--;
            }
            else if (event.key.keysym.sym == SDLK_DOWN && *selectedIndex < buttonIndexes)
            {
                (*selectedIndex)++;
            }

            SDL_HideWindow(childWindow);
            break;
        }
    }

    SDL_DestroyRenderer(childRenderer);
    SDL_DestroyWindow(childWindow);

    return AppCode::success;
}

void Rhytmia::renderWindow(int buttonCount, int selectedButtonIndex, int windowWidth, int windowHeight)
{
    SDL_RenderClear(m_renderer);

    for (size_t i = 0; i < buttonCount; i++)
    {
        if (i == selectedButtonIndex)
        {
            SDL_SetRenderDrawColor(m_renderer, m_colors.sText.r, m_colors.sText.g, m_colors.sText.b, m_colors.sText.a);
        }

        SDL_Surface *textSurface = TTF_RenderText_Solid(m_font, m_menuButtons[i].c_str(), m_colors.text);
        SDL_Texture *textTexture = SDL_CreateTextureFromSurface(m_renderer, textSurface);

        SDL_Rect textRect{};
        textRect.x = m_marginW;
        textRect.y = i * windowHeight / buttonCount + m_marginH;
        textRect.w = windowWidth - m_marginW;
        textRect.h = textSurface->h;

        SDL_RenderFillRect(m_renderer, &textRect);

        textRect.x = m_marginW;
        textRect.w = textSurface->w;
        SDL_RenderCopy(m_renderer, textTexture, NULL, &textRect);

        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);

        SDL_SetRenderDrawColor(m_renderer, m_colors.background.r, m_colors.background.g, m_colors.background.b, m_colors.background.a);
    }

    SDL_RenderPresent(m_renderer);
}

Rhytmia::AppCode Rhytmia::initialize(int *windowWidth, int *windowHeight, int *x, int *y)
{
    getWindowSizes(windowWidth, windowHeight, m_menuButtons);
    getWindowSizes(&m_installPluginsPageW, &m_installPluginsPageH, m_toInstallPlugins);
    getWindowPos(x, y);

    m_window = SDL_CreateWindow(m_config["APP"]["app_name"].c_str(), *x, *y, *windowWidth, *windowHeight,
                                SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_BORDERLESS |
                                SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_INPUT_FOCUS);
    if (m_window == NULL)
    {
        return AppCode::sdlError;
    }

    SDL_SetWindowIcon(m_window, m_iconSurface);

    if (SDL_SetWindowOpacity(m_window, std::stof(m_config["APP"]["window_opacity"])) < 0)
    {
        return AppCode::sdlError;
    }

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (m_renderer == NULL)
    {
        return AppCode::sdlError;
    }

    m_colors.text = getColor("text_color");
    m_colors.sText = getColor("selected_text_color");
    m_colors.background = getColor("background_color");
    SDL_SetRenderDrawColor(m_renderer, m_colors.background.r, m_colors.background.g, m_colors.background.b, m_colors.background.a);

    return AppCode::success;
}

Rhytmia::AppCode Rhytmia::parseConfig()
{
    const std::string configFile = "config.ini";
    std::ifstream file(configFile);

    if (!file.is_open())
    {
        return AppCode::openConfigFileError;
    }

    std::string line;
    std::string currentSection;

    while (std::getline(file, line))
    {
        if (line.front() == '[' && line.back() == ']') 
        {
            currentSection = line.substr(1, line.length() - 2);
        }
        else 
        {
            const size_t equalsPos = line.find('=');
            if (equalsPos != std::string::npos) 
            {
                std::string key = line.substr(0, equalsPos);
                std::string value = line.substr(equalsPos + 1);

                m_config[currentSection][key] = value;
            }
        }
    }

    file.close();

    return AppCode::success;
}

Rhytmia::AppCode Rhytmia::initButtons()
{
    m_appCode = getPlugins(&m_toInstallPlugins, &m_installedPlugins);

    if (m_appCode != AppCode::success)
    {
        return m_appCode;
    }

    if (m_toInstallPlugins.size() > 0)
    {
        m_menuButtons.push_back("install plugins");

       
    }

    for (const std::string &entry : m_installedPlugins)
    {
        m_menuButtons.push_back(entry);
    }

    m_menuButtons.push_back("about");
    m_menuButtons.push_back("exit");

    return AppCode::success;
}

Rhytmia::AppCode Rhytmia::getPlugins(std::vector<std::string> *toInstall, std::vector<std::string> *installed)
{
    const std::string pluginsPath = m_config["PLUGINS"]["plugins_path"];

    for (const fs::path &entry : fs::directory_iterator(pluginsPath))
    {
        if (fs::is_directory(entry))
        {
            installed->push_back(entry.filename().string());
        }
    }

    m_appCode = getAllPlugins(toInstall);
    if (m_appCode != AppCode::success)
    {
        return m_appCode;
    }

    toInstall->erase(std::remove_if(toInstall->begin(), toInstall->end(), [&](std::string str) 
    { 
        return std::find(installed->begin(), installed->end(), str) != installed->end();
    }), toInstall->end());

    return AppCode::success;
}

Rhytmia::AppCode Rhytmia::getAllPlugins(std::vector<std::string> *all)
{
    const std::string url = m_config["PLUGINS"]["all_plugins_url"];
    std::string response;

    const WebWrapper::ResultCode isGetResp = m_webWrapper->getResponse(url, &response);
    if (isGetResp != WebWrapper::ResultCode::success)
    {
        return AppCode::webWrapperError;
    }

    std::vector<std::string> words;
    std::stringstream ss(response);
    std::string word;

    while (ss >> word) 
    {
        all->push_back(word);
    }

    return AppCode::success;
}

SDL_Color Rhytmia::getColor(const char *type)
{
    const char delim = ':';

    std::vector<std::string> result;
    std::stringstream ss(m_config["APP"][type]);
    std::string item;

    while (std::getline(ss, item, delim)) 
    {
        result.push_back(item);
    }
    
    const SDL_Color color
    {
        std::stoi(result[0]),
        std::stoi(result[1]),
        std::stoi(result[2]),
        std::stoi(result[3])
    };

    return color;
}

std::string Rhytmia::Rhytmia_GetError()
{
    return ("ERROR_CODE: " + std::to_string(m_appCode));
}
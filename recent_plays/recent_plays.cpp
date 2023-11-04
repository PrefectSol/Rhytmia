// recent_plays.cpp : Определяет экспортируемые функции для DLL.
//

#include <SDL.h>

#include "pch.h"
#include "framework.h"
#include "recent_plays.h"



extern "C" RECENTPLAYS_API void DrawRectangle(SDL_Window * window) {
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Failed to create SDL2 renderer: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Красный цвет
    SDL_RenderClear(renderer);

    SDL_Rect rect = { 100, 100, 200, 200 }; // Координаты и размеры прямоугольника
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Зеленый цвет
    SDL_RenderFillRect(renderer, &rect);

    SDL_RenderPresent(renderer);
    SDL_DestroyRenderer(renderer);
}
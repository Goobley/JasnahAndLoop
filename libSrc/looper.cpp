/* ==========================================================================
   $File: looper.cpp $
   $Version: 1.0 $
   $Notice: (C) Copyright 2015 Chris Osborne. All Rights Reserved. $
   $License: MIT: http://opensource.org/licenses/MIT $
   ========================================================================== */
#include "../src/LethaniGlobalDefines.h"
#include "looper.hpp"
#include <string>
#include <iostream>
// Only temporarily
#include <cstdio>

FileScope void* Init(CompleteState* state)
{
}

FileScope void AnotherFunc()
{
    std::cout << "Here's another func...\n";
}

FileScope void Close(CompleteState* state)
{
}

FileScope void Reload(CompleteState* state)
{
    printf("Reloaded!\n");
    // printf("And Again!\n");
}

FileScope void Unload(CompleteState* state)
{
}

FileScope bool Update(CompleteState* state)
{
    #if 0
    std::cout << "Do you want to continue? [Y/n]" << std::endl;
    LocalPersist std::string response;
    std::getline(std::cin, response);
    if (response == "n" || response == "N")
        return false;
    std::cout << "Interesting..." << std::endl;
    AnotherFunc();
    #else

    assert(sizeof(GameState) <= state->persistantHeapSize);
    GameState* gameState = static_cast<GameState*>(state->persistantHeap);
    if (!gameState->initialized)
    {
        // TODO(Chris): Move SDL_Init to main
        SDL_Init(SDL_INIT_EVERYTHING);
        SDL_DisplayMode displayMode;
        if (SDL_GetDesktopDisplayMode(0, &displayMode) != 0)
        {
            // TODO(Chris): Logging!!
            printf("%s", SDL_GetError());
            // SDL_GetError();
            return false;
        }

        if (SDL_CreateWindowAndRenderer(displayMode.w / 2, displayMode.h / 2,
                                    SDL_WINDOW_SHOWN,
                                    &gameState->window, &gameState->renderer)
            != 0)
        {
            // TODO(Chris): Logging!!
            // SDL_GetError();
            printf("%s", SDL_GetError());
            return false;
        }
        gameState->initialized = true;
    }

    assert(sizeof(WorkingState) <= state->workingHeapSize);
    WorkingState* workState = static_cast<WorkingState*>(state->workingHeap);
    if (!workState->initialized)
    {
        InitializeArena(&(workState->workingArena), state->workingHeapSize - sizeof(workState),
                        static_cast<u8*>(state->workingHeap) + sizeof(workState));
        workState->initialized = true;
    }


    // SDL_SetRenderDrawColor(gameState->renderer, 0, 0, 0, 0);
    SDL_RenderClear(gameState->renderer);
    SDL_RenderPresent(gameState->renderer);

    SDL_Event e;
    while (SDL_PollEvent(&e) != 0)
    {
        switch (e.type)
        {
        case SDL_QUIT:
            return false;
            break;

        default:
            break;
        }
    }


    #endif

    return true;
}

const LoopAPI loopAPI = {Init, Close, Reload, Unload, Update};

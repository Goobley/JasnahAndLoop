/* ==========================================================================
   $File: main.cpp $
   $Version: 1.0 $
   $Notice: (C) Copyright 2015 Chris Osborne. All Rights Reserved. $
   $License: MIT: http://opensource.org/licenses/MIT $
   ========================================================================== */

#include "LethaniGlobalDefines.h"
#include "../libSrc/looper.hpp"
#include "../libSrc/MemoryLayout.hpp"
#include <sys/stat.h>
#include <SDL2/SDL.h>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>

FileScope constexpr const char* libraryName = "bin/libLoop.so";

struct Loop
{
    void* handle;
    time_t updateTime;
    CompleteState* state;
    LoopAPI api;
    MemoryIndex totalHeapSize;
    void* heapMemory;
};

FileScope void LooperLoad(Loop* loop)
{
    struct stat libData;
    if ((stat(libraryName, &libData) == 0) && (loop->updateTime != libData.st_mtime))
    {
        if (loop->handle)
        {
            loop->api.Unload(loop->state);
            SDL_UnloadObject(loop->handle);
        }
        void *handle = SDL_LoadObject(libraryName);
        if (handle)
        {
            loop->handle = handle;
            loop->updateTime = libData.st_mtime;
            auto* api = reinterpret_cast<const LoopAPI*>(SDL_LoadFunction(loop->handle, "loopAPI"));
            if (api != nullptr)
            {
                loop->api = *api;
                // if (loop->state == nullptr)
                //     loop->state = loop->api.Init();
                loop->api.Reload(loop->state);
            }
            else
            {
                SDL_UnloadObject(loop->handle);
                loop->handle = nullptr;
                loop->updateTime = 0;
            }
        }
        else
        {
            loop->handle = nullptr;
            loop->updateTime = 0;
        }
    }
}

FileScope void LooperUnload(Loop* loop)
{
    if (loop->handle != nullptr)
    {
        loop->api.Close(loop->state);
        loop->state = nullptr;
        SDL_UnloadObject(loop->handle);
        loop->handle = nullptr;
        loop->updateTime = 0;
    }
}

int main(void)
{
    CompleteState libState{};
    libState.normalHeapSize = Megabytes(128);
    libState.workingHeapSize = Megabytes(128);

    Loop loop{};
    loop.totalHeapSize = libState.normalHeapSize + libState.workingHeapSize;
    loop.heapMemory = calloc(loop.totalHeapSize, sizeof(u8));

    libState.normalHeap = loop.heapMemory;
    libState.tempHeap = (void*)((u8*)libState.normalHeap + libState.normalHeapSize);

    loop.state = &libState;

    while (true)
    {
        LooperLoad(&loop);
        if (loop.handle != nullptr)
        {
            if (!loop.api.Update(loop.state))
                break;
        }
        else
        {
            std::cout << SDL_GetError() << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    LooperUnload(&loop);
    return 0;
}

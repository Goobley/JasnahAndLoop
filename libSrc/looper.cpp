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

struct LoopState
{
    std::string initStr;
    std::string closeStr;
    std::string unloadStr;
    std::string reloadStr;
    std::string updateStr;
};

FileScope LoopState* Init()
{
    auto* state = new LoopState;
    state->initStr = "Initialising\n";
    state->closeStr = "Closing\n";
    state->unloadStr = "Unloading\n";
    state->reloadStr = "Reloading\n";
    state->updateStr = "Updating...\n";
    std::cout << state->initStr;
    return state;
}

FileScope void AnotherFunc()
{
    std::cout << "Here's another func...\n";
}

FileScope void Close(LoopState* state)
{
    std::cout << state->closeStr;
    delete state;
}

FileScope void Reload(LoopState* state)
{
    std::cout << state->reloadStr;
}

FileScope void Unload(LoopState* state)
{
    std::cout << state->unloadStr;
}

FileScope bool Update(LoopState* state)
{
    std::cout << state->updateStr << "Do you want to continue? [Y/n]" << std::endl;
    LocalPersist std::string response;
    std::getline(std::cin, response);
    if (response == "n" || response == "N")
        return false;
    std::cout << state->updateStr << "Intersting..." << std::endl;
    // AnotherFunc();

    return true;
}

const LoopAPI loopAPI = {Init, Close, Reload, Unload, Update};

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

FileScope void* Init(LoopState* state)
{
}

FileScope void AnotherFunc()
{
    std::cout << "Here's another func...\n";
}

FileScope void Close(LoopState* state)
{
}

FileScope void Reload(LoopState* state)
{
}

FileScope void Unload(LoopState* state)
{
}

FileScope bool Update(LoopState* state)
{
    std::cout << "Do you want to continue? [Y/n]" << std::endl;
    LocalPersist std::string response;
    std::getline(std::cin, response);
    if (response == "n" || response == "N")
        return false;
    std::cout << "Interesting..." << std::endl;
    AnotherFunc();

    return true;
}

const LoopAPI loopAPI = {Init, Close, Reload, Unload, Update};

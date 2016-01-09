// -*- c++ -*-
#if !defined(LOOPER_H)
/* ==========================================================================
   $File: looper.hpp $
   $Version: 1.0 $
   $Notice: (C) Copyright 2015 Chris Osborne. All Rights Reserved. $
   $License: MIT: http://opensource.org/licenses/MIT $
   ========================================================================== */

#define LOOPER_H
#include "../src/LethaniGlobalDefines.h"
#include "MemoryLayout.hpp"

struct LoopAPI
{
    /// init clean state
    void* (*Init)(CompleteState* state);
    /// Close state
    void (*Close)(CompleteState* state);
    /// Reload State - called once upon reload
    void (*Reload)(CompleteState* state);
    /// Unload State - called once before reload
    void (*Unload)(CompleteState* state);

    /// Update - return true to continue
    bool (*Update)(CompleteState* state);
};

/// The structure of data called from the shared library by the platform code
extern "C" const LoopAPI loopAPI;

#endif

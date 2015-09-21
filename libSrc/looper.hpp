// -*- c++ -*-
#if !defined(LOOPER_H)
/* ==========================================================================
   $File: looper.hpp $
   $Version: 1.0 $
   $Notice: (C) Copyright 2015 Chris Osborne. All Rights Reserved. $
   $License: MIT: http://opensource.org/licenses/MIT $
   ========================================================================== */

#define LOOPER_H

extern "C" struct LoopState;

extern "C" struct LoopAPI
{
    /// Return clean state
    LoopState* (*Init)();
    /// Close state
    void (*Close)(LoopState* state);
    /// Reload State - called once upon reload
    void (*Reload)(LoopState* state);
    /// Unload State - called once before reload
    void (*Unload)(LoopState* state);

    /// Update - return true to continue
    bool (*Update)(LoopState* state);
};

extern "C" const LoopAPI loopAPI;

#endif

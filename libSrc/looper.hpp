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
#include <cstddef>

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

typedef std::size_t MemoryIndex;

typedef struct LoopState
{
    /// Normal heap is fully persistent
    MemoryIndex normalHeapSize;
    void* normalHeap;

    // Temp heap is cleared every frame
    MemoryIndex tempHeapSize;
    void* tempHeap;
} LoopState;

struct LoopAPI
{
    /// init clean state
    void* (*Init)(LoopState* state);
    /// Close state
    void (*Close)(LoopState* state);
    /// Reload State - called once upon reload
    void (*Reload)(LoopState* state);
    /// Unload State - called once before reload
    void (*Unload)(LoopState* state);

    /// Update - return true to continue
    bool (*Update)(LoopState* state);
};

/// The structure of data called from the shared library by the platform code
extern "C" const LoopAPI loopAPI;

#endif

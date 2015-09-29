// -*- c++ -*-
#if !defined(MEMORYLAYOUT_H)
/* ==========================================================================
   $File: MemoryLayout.hpp $
   $Version: 1.0 $
   $Notice: (C) Copyright 2015 Chris Osborne. All Rights Reserved. $
   $License: MIT: http://opensource.org/licenses/MIT $
   ========================================================================== */

#define MEMORYLAYOUT_H
#include "../src/LethaniGlobalDefines.h"
#include <cassert>
#include <cstring>
#include <SDL2/SDL.h>

#define Kilobytes(value) ((value)*1024LL)
#define Megabytes(value) (Kilobytes(value)*1024LL)
#define Gigabytes(value) (Megabytes(value)*1024LL)
#define Terabytes(value) (Gigabytes(value)*1024LL)

#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]));

using std::size_t;
typedef std::size_t MemoryIndex;

typedef struct CompleteState
{
    /// Normal heap is fully persistent
    MemoryIndex persistantHeapSize;
    void* persistantHeap;

    MemoryIndex workingHeapSize;
    void* workingHeap;
} CompleteState;

struct MemoryArena
{
    MemoryIndex size;
    u8* blockStart;
    MemoryIndex fillPoint;

    i32 tempCount;
};

struct TempBlock
{
    MemoryArena* arena;
    MemoryIndex fillPoint;
};

struct GameState
{
    bool initialized;
    f32 time;
    SDL_Window* window;
    SDL_Renderer* renderer;
};

struct WorkingState
{
    bool initialized;
    MemoryArena workingArena;
};

inline void
InitializeArena(MemoryArena* arena, MemoryIndex size, void* blockStart)
{
    arena->size = size;
    arena->blockStart = static_cast<u8*>(blockStart);
    arena->fillPoint = 0;
    arena->tempCount = 0;
}

inline MemoryIndex
GetAlignmentOffset(const MemoryArena* arena, MemoryIndex alignment)
{
    MemoryIndex offset = 0;
    MemoryIndex blockEnd = (MemoryIndex)arena->blockStart + arena->fillPoint;
    MemoryIndex memoryMask = alignment - 1;

    if (blockEnd & memoryMask)
    {
        offset = alignment - (blockEnd & memoryMask);
    }

    return offset;
}

inline MemoryIndex
GetRemainingArenaSize(const MemoryArena* arena, MemoryIndex alignment = 4)
{
    return arena->size - (arena->fillPoint + GetAlignmentOffset(arena, alignment));
}

inline void*
PushBlock(MemoryArena* arena, MemoryIndex size, MemoryIndex alignment = 4)
{
    MemoryIndex offset = GetAlignmentOffset(arena, alignment);
    size += offset;

    assert((arena->fillPoint + size) <= arena->size);
    void* blockStart = (arena->blockStart + arena->fillPoint + offset);
    arena->fillPoint += size;

    return blockStart;
}

template <typename T>
inline T*
PushArray(MemoryArena* arena, size_t num, MemoryIndex alignment = 4)
{
    return static_cast<T*>(PushBlock(arena, num*sizeof(T), alignment));
}

template <typename T>
inline T*
PushStruct(MemoryArena* arena, MemoryIndex alignment = 4)
{
    return static_cast<T*>(PushBlock(arena, sizeof(T), alignment));
}

inline char*
PushString(MemoryArena* arena, const char* src)
{
    size_t size = strlen(src);
    char* dest = static_cast<char*>(PushBlock(arena, size));
    strcpy(dest, src);
    return dest;
}

inline TempBlock
CreateTempBlock(MemoryArena* arena)
{
    TempBlock mem;
    mem.arena = arena;
    mem.fillPoint = arena->fillPoint;
    ++(arena->tempCount);

    return mem;
}

inline void
ClearTempBlock(TempBlock block)
{
    MemoryArena* arena = block.arena;
    assert(arena->fillPoint >= block.fillPoint);
    arena->fillPoint = block.fillPoint;
    assert(arena->tempCount > 0);
    --(arena->tempCount);
}

inline void
CheckArenaNoTemps(MemoryArena* arena)
{
    assert(arena->tempCount == 0);
}

inline void
CreateSubArena(MemoryArena* sub, MemoryArena* arena, MemoryIndex size, MemoryIndex alignment = 16)
{
    sub->size = size;
    sub->blockStart = (u8*)PushBlock(arena, size, alignment);
    sub->fillPoint = 0;
    sub->tempCount = 0;
}

inline void
ZeroBlock(void* block, MemoryIndex size)
{
    u8* byte = static_cast<u8*>(block);
    // C stdlib may provide a better way of doing this
    while (size--)
    {
        *(byte++) = 0;
    }
}

template <typename T>
inline void
ZeroStruct(T* obj)
{
    ZeroBlock(static_cast<void*>(obj), sizeof(obj));
}

template <typename T>
inline void
ZeroArray(T* obj, size_t num)
{
    ZeroBlock(static_cast<void*>(obj), num * sizeof(obj[0]));
}

inline void
CopyBlock(void* dest, const void* src, size_t numBytes)
{
    memcpy(dest, src, numBytes);
}

template <typename T, typename U>
inline void
Copy(T* dest, const U* src, size_t numBytes)
{
    CopyBlock(static_cast<void*>(dest), static_cast<const void*>(src), numBytes);
}

template <typename T>
inline void
CopyStruct(T* dest, const T* src)
{
    CopyBlock(static_cast<void*>(dest), static_cast<const void*>(src), sizeof(T));
}




#endif

/** @file
 * Qin - Global Memory Management Unit.
 * This module allocating and releasing the heap memory in the
 * virtual address space of the process, keeping a track of the operation
 * and maintaining the statistics data.
 */

/*
 *  Qin is Copyright (C) 2016, The 1st Middle School in
 *  Yongsheng Lijiang, Yunnan Province, ZIP 674200 China
 *
 *  This project is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License(GPL)
 *  as published by the Free Software Foundation; either version 2.1
 *  of the License, or (at your option) any later version.
 *
 *  This project is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 */

/*******************************************************************************
*   Header Files                                                               *
*******************************************************************************/
#include <new>
#include <cstdlib>

#include "util/assert.h"
#include "util/types.h"
#include "util/error.h"

#include "memory/mmu.h"

/*******************************************************************************
*   Typedefs and structures                                                    *
*******************************************************************************/

/*
 * Memory track header
 */
struct MEMTRACKHDR
{
  /** magic number for debugging */
  uint32_t      magic;
  /** tag description */
  MemoryTag     tag;
  /** the length of this block */
  std::size_t   len;
};

#define MEMBLOCK_MAGIC (0x514d424b)


/*******************************************************************************
*   Global Variables                                                           *
*******************************************************************************/

static std::size_t  bytesMemAllocated[_MAX_MEM_TAG_NUM] = {0};
static char *reservedMem = 0;

////////////////////////////////////////////////////////////////////////////////

inline void
initMemTrackHdr(MEMTRACKHDR *src, std::size_t count, MemoryTag tag)
{
  src->magic = MEMBLOCK_MAGIC;
  src->tag = tag;
  src->len = count;
}

/*
 * the global reloading of operator new.
 * do NOT place any heavy work at there, and be sure not to
 * allocate memory using new operator anymore within the following methods.
 */
void *
operator new(std::size_t count, MemoryTag tag) throw(std::bad_alloc)
{
  register std::size_t size = count + sizeof(MEMTRACKHDR);
  MEMTRACKHDR *p = (MEMTRACKHDR*)malloc(size);
  if (!p)
    {
      std::bad_alloc e;
      throw(e);
    }

  initMemTrackHdr(p, count, tag); p++;

  V_ASSERT( count < MEM_ALIGNMENT ||
            !((uintptr_t)p & (MEM_ALIGNMENT - 1)) ||
            ( (count & MEM_ALIGNMENT) + ((uintptr_t)p & MEM_ALIGNMENT)) == MEM_ALIGNMENT );

  bytesMemAllocated[tag] += size;

  return (void*)(p);
}

void *
operator new(std::size_t count) throw(std::bad_alloc)
{
  return ::operator new(count, MEM_TAG_DEFAULT);
}

void *
operator new(std::size_t count, MemoryTag tag, const std::nothrow_t&t) throw()
{
  register std::size_t size = count + sizeof(MEMTRACKHDR);
  MEMTRACKHDR *p = (MEMTRACKHDR*)malloc(size);
  if (p)
    {
      initMemTrackHdr(p, count, tag); p++;

      V_ASSERT( count < MEM_ALIGNMENT ||
                !((uintptr_t)p & (MEM_ALIGNMENT - 1)) ||
                ( (count & MEM_ALIGNMENT) + ((uintptr_t)p & MEM_ALIGNMENT)) == MEM_ALIGNMENT );

      bytesMemAllocated[tag] += size;
    }
  return (void*)p;
}

void *
operator new(std::size_t count, const std::nothrow_t&t) throw()
{
  return ::operator new(count, MEM_TAG_DEFAULT, t);
}

void *
operator new[](std::size_t count, MemoryTag tag) throw(std::bad_alloc)
{
  return ::operator new(count, tag);
}

void *
operator new[](std::size_t count) throw(std::bad_alloc)
{
  return ::operator new[](count, MEM_TAG_DEFAULT);
}

void *
operator new[](std::size_t count, MemoryTag tag, const std::nothrow_t&t) throw()
{
  return ::operator new(count, tag, t);
}

void *
operator new[](std::size_t count, const std::nothrow_t&t) throw()
{
  return ::operator new(count, MEM_TAG_DEFAULT, t);
}

void
operator delete(void* pointer)
{
  MEMTRACKHDR *p = (MEMTRACKHDR*)pointer;
  p--;

  V_ASSERT(p->magic == MEMBLOCK_MAGIC);
  V_ASSERT(p->tag >= MEM_TAG_DEFAULT && p->tag < _MAX_MEM_TAG_NUM);

  bytesMemAllocated[p->tag] -= p->len;

  free(pointer);
}

void
operator delete[](void* pointer)
{
  ::operator delete(pointer);
}

/**
 * Allocate the reserved memory for the emergent case.
 * When the memory got exhausted, we will use this reserved memory to
 * do the termination work.
 *
 * @return status code.
 */
int AllocReservedMem()
{
  reservedMem = new (MEM_TAG_RESERVED, std::nothrow) char[MEM_RESERVED_SIZE];
  if (!reservedMem)
    {
      return VERR_ALLOC_MEMORY;
    }
  return VINF_SUCCEEDED;
}

/**
 * Release the reserved memory so that the other module can make use
 * of it. don't call this until AllocReservedMem() is called successfully,
 * and this function is disposable.
 */
void
ReleaseReservedMem()
{
  delete reservedMem;
}

/**
 * Get the bytes allocated at the heap.
 * @return the result.
 */
std::size_t GetBytesMemAllocated()
{
  std::size_t total = 0;
  for (int i = 0; i < _MAX_MEM_TAG_NUM; i++)
    {
      total += bytesMemAllocated[i];
    }
  return total;
}

/**
 * Get the bytes allocated at the heap.
 * @param tag           Tag description of memory block.
 * @return the result.
 */
std::size_t GetBytesMemAllocated(MemoryTag tag)
{
  V_ASSERT(tag >= MEM_TAG_DEFAULT && tag < _MAX_MEM_TAG_NUM);
  return bytesMemAllocated[tag];
}

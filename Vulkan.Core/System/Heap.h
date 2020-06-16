#pragma once

#include "..\Common\Common.h"

using namespace Graphics;

char *					va(const char *fmt, ...);


void *		Mem_Alloc16(const size_t size, const MemoryTag tag);
void		Mem_Free16(void *ptr);
void *		Mem_ClearedAlloc(const int size, const MemoryTag tag);

inline void *	Mem_Alloc(const size_t size, const MemoryTag tag) { return Mem_Alloc16(size, tag); }
inline void		Mem_Free(void *ptr) { Mem_Free16(ptr); }



inline void *operator new(size_t s, MemoryTag tag) {
	return Mem_Alloc(s, tag);
}
inline void operator delete(void *p, MemoryTag tag) {
	Mem_Free(p);
}

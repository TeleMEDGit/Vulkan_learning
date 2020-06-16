#include "Heap.h"
#include <malloc.h>
#include <memory.h>
#include <stdarg.h>
#include <stdio.h>


/*
============
va

does a varargs printf into a temp buffer
NOTE: not thread safe
============
*/
char *va(const char *fmt, ...) {
	va_list argptr;
	static int index = 0;
	static char string[4][16384];	// in case called by nested functions
	char *buf;

	buf = string[index];
	index = (index + 1) & 3;

	va_start(argptr, fmt);
	//vsprintf_s(buf, fmt, argptr);
	va_end(argptr);

	return buf;
}

void * Mem_Alloc16(const size_t size, const MemoryTag tag)
{
	if (!size) {
		return nullptr;
	}
	const int paddedSize = (size + 15) & ~15;
	return _aligned_malloc(paddedSize, 16);
}

void Mem_Free16(void * ptr)
{
	if (ptr == NULL)
	{
		return;
	}
	_aligned_free(ptr);


}

void * Mem_ClearedAlloc(const int size, const MemoryTag tag)
{
	void * mem = Mem_Alloc(size, tag);
	//SIMDProcessor->Memset(mem, 0, size);
	memset(mem, 0, size);
	return mem;
	return nullptr;
}

#include "precompiled.h"

int gAllocatedMemoryBlocks = 0;


#pragma function(memset)
void * __cdecl memset(void *pTarget, int value, size_t cbTarget) {
	unsigned char *p = static_cast<unsigned char *>(pTarget);
	while (cbTarget-- > 0) {
		*p++ = static_cast<unsigned char>(value);
	}
	return pTarget;
}

void * operator new( size_t size ) {
	++gAllocatedMemoryBlocks;
	return HeapAlloc( GetProcessHeap(), 0, size );
}

void operator delete( void * ptr ) {
	if( !ptr ) {
		return;
	}
	--gAllocatedMemoryBlocks;
	HeapFree( GetProcessHeap(), 0, ptr );
}

void * operator new[](size_t size) {
	++gAllocatedMemoryBlocks;
	return HeapAlloc( GetProcessHeap(), 0, size );
}

void operator delete[](void *ptr) {
	if( !ptr ) {
		return;
	}
	--gAllocatedMemoryBlocks;
	HeapFree( GetProcessHeap(), 0, ptr );
}




void * Realloc( void * ptr, size_t newSize ) {
	if( !ptr ) {
		++gAllocatedMemoryBlocks;
		return HeapAlloc( GetProcessHeap(), 0, newSize );
	}
	if( newSize == 0 ) {
		--gAllocatedMemoryBlocks;
		HeapFree( GetProcessHeap(), 0, ptr );
		return nullptr;
	} else {
		return HeapReAlloc( GetProcessHeap(), 0, ptr, newSize );
	}
}


namespace Memory {

FPtr_memcpy Copy;
FPtr_memmove Move;

void LoadFunctions() {
	HINSTANCE crt = LoadLibraryA( "msvcrt.dll" );
	Copy = (FPtr_memcpy)GetProcAddress( crt, "memcpy" );
	Move = (FPtr_memmove)GetProcAddress( crt, "memmove" );
}

};
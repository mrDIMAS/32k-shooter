#include "precompiled.h"
#include "cstring.h"

FPtr_sprintf Sprintf;

namespace String {

FPtr_sprintf Format;
FPtr_strcpy Copy;
FPtr_strlen Length;
FPtr_atof ToNumber;
FPtr_strtok Tokenize;


void LoadFunctions() {
	HINSTANCE crt = LoadLibraryA( "msvcrt.dll" );
	Format = (FPtr_sprintf)GetProcAddress( crt, "sprintf" );
	Sprintf = (FPtr_sprintf)GetProcAddress( crt, "sprintf" );
	Copy = (FPtr_strcpy)GetProcAddress( crt, "strcpy" );
	Length = (FPtr_strlen)GetProcAddress( crt, "strlen" );
	ToNumber = (FPtr_atof)GetProcAddress( crt, "atof" );
	Tokenize = (FPtr_strtok)GetProcAddress( crt, "strtok" );
}

}
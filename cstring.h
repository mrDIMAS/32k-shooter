#pragma once


typedef int (*FPtr_strlen)( const char * str );
typedef char* (*FPtr_strcpy)( char * dst, const char * src );

typedef double (*FPtr_atof)( const char * str );
typedef double (*FPtr_atof)( const char * str );
typedef char * (*FPtr_strtok) ( char * string, const char * delim );


// Wrapper for functions from C runtime
namespace String {



	extern FPtr_sprintf Format;
	extern FPtr_strcpy Copy;
	extern FPtr_strlen Length;
	extern FPtr_atof ToNumber;
	extern FPtr_strtok Tokenize;

	void LoadFunctions();
};


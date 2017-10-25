//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <sstream>
#include "Utils.h"
//#include <varargs.h>
#include <stdarg.h>
#include <cwchar>

#define PRINT                PrintDbg
#define PRINT_VAR( var )	 PrintVar( #var, var )
#define PRINT_VAR_INL( var ) PrintVar( #var, var, true )
#define PRINT_FUNC           PrintFunc( __FUNCTION__ );
#define VAR_TO_STR( var )    VarToStr( #var, var )
#define VAR_TO_CHAR( var )   VarToStr( #var, var ).c_str()
#define VAR_TO_ANSI( var )   VarToAnsi( #var, var ).c_str()

//-----------------------------------------------------------------------------
template<class T>
inline void PrintVar( const char* a_VarName, const T& a_Value, bool a_SameLine = false )
{
    std::stringstream l_StringStream;
    l_StringStream << a_VarName << " = " << a_Value;
    if( !a_SameLine ) l_StringStream << std::endl;
#if _WIN32||_WIN64    
    OutputDebugStringA( l_StringStream.str().c_str() );
#else
    printf( "%s", l_StringStream.str().c_str() );
#endif
}

//-----------------------------------------------------------------------------
inline void PrintVar( const char* a_VarName, const std::wstring& a_Value, bool a_SameLine = false )
{    
#if _WIN32||_WIN64    
    OutputDebugStringA( a_VarName );
    OutputDebugStringW(std::wstring( L" = " + a_Value).c_str() );
    if( !a_SameLine )
    {
        OutputDebugStringA( "\r\n" );
    }
#else
    printf( "%s", a_VarName );
    wprintf(std::wstring( L" = " + a_Value).c_str() );
    if( !a_SameLine )
    {
        printf( "\r\n" );
    }
#endif
}

//-----------------------------------------------------------------------------
template<class T>
inline std::wstring VarToStr( const char* a_VarName, const T& a_Value )
{
    std::stringstream l_StringStream;
    l_StringStream << a_VarName << " = " << a_Value << std::endl;
    return s2ws( l_StringStream.str() );
}

//-----------------------------------------------------------------------------
template<class T>
inline std::string VarToAnsi( const char* a_VarName, const T& a_Value )
{
    std::stringstream l_StringStream;
    l_StringStream << a_VarName << " = " << a_Value;
    return l_StringStream.str();
}

//-----------------------------------------------------------------------------
inline void PrintFunc( const char* a_Function )
{
    std::string func = Format( "%s TID: %u\n", a_Function, GetCurrentThreadId() );
#if _WIN32||_WIN64
    OutputDebugStringA( func.c_str() );
#else
    printf( func.c_str() );
#endif
}

//-----------------------------------------------------------------------------
inline void PrintDbg( const char* msg, ... )
{
    va_list ap;
    va_start( ap, msg );
#if _WIN32||_WIN64
    const int BUFF_SIZE = 4096;
    char text[BUFF_SIZE] = { 0, };
    vsnprintf_s( text, BUFF_SIZE - 1, msg, ap );
    OutputDebugStringA(text);
#else
    vprintf(msg, ap);
#endif
    va_end( ap );
}

//-----------------------------------------------------------------------------
inline void PrintDbg( const wchar_t* msg, ... )
{
    va_list ap;
    va_start( ap, msg );
#if _WIN32||_WIN64
    const int BUFF_SIZE = 4096;
    wchar_t text[BUFF_SIZE] = { 0, };
    _vsnwprintf_s( text, BUFF_SIZE - 1, msg, ap );
    OutputDebugStringW( text );
#else
    vwprintf( msg, ap );
#endif
    va_end( ap );
}

//-----------------------------------------------------------------------------
inline void PrintDbg( const std::string & a_Msg )
{
#if _WIN32||_WIN64
    OutputDebugStringA(a_Msg.c_str());
#else
    printf( "%s", a_Msg.c_str());
#endif
}

//-----------------------------------------------------------------------------
inline void PrintDbg( const std::wstring & a_Msg )
{
#if _WIN32||_WIN64
    OutputDebugStringW( a_Msg.c_str() );
#else
    wprintf(a_Msg.c_str());
#endif
}

//-----------------------------------------------------------------------------
inline void PrintLastError()
{
    PrintVar( "Last error: ", GetLastErrorAsString() );
}
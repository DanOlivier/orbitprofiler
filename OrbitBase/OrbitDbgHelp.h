//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#if _WIN32||_WIN64
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef UNICODE
#define DBGHELP_TRANSLATE_TCHAR
#endif

#pragma warning( push )
#pragma warning( disable : 4091 )
#include <DbgHelp.h>
#pragma warning( pop )

#endif

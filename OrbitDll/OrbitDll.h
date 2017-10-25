#pragma once

#if defined(_MSC_VER)
//  Microsoft 
#define EXPORT __declspec(dllexport)
#define IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
//  GCC
#define EXPORT __attribute__((visibility("default")))
#define IMPORT
#else
//  do nothing and hope for the best?
#define EXPORT
#define IMPORT
#pragma warning Unknown dynamic link import/export semantics.
#endif

extern "C"
{
    EXPORT void __cdecl OrbitInit( void* a_Host );
    EXPORT void __cdecl OrbitInitRemote( void* a_Host );
    EXPORT bool __cdecl OrbitIsConnected();
    EXPORT bool __cdecl OrbitStart();
    EXPORT bool __cdecl OrbitStop();
}

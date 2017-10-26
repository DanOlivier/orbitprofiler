//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "ProcessUtils.h"
#include <string>

#if !(_WIN32||_WIN64)
#define WINAPI
#endif

class Injection
{
public:
    Injection();

    bool Inject( const std::wstring & a_Dll, const Process & a_Process, const std::string & ProcName, const std::wstring& captureHost, int capturePort);
    DWORD GetProcessID() const { return m_InjectedProcessID; }
    HANDLE GetProcessHandle() const { return m_InjectedProcessHandle; }

    static HANDLE GetTargetProcessHandle( const std::string & a_Target, DWORD & o_ProcessID );
    static HMODULE WINAPI GetRemoteModuleHandle( HANDLE hProcess, LPCSTR lpModuleName );
    static FARPROC WINAPI GetRemoteProcAddress( HANDLE hProcess, HMODULE hModule, LPCSTR lpProcName, UINT Ordinal = 0, BOOL UseOrdinal = FALSE );

protected:
    void* RemoteWrite( const char* a_Data, int a_NumBytes );
    void* RemoteWrite( const std::string & a_String );
    void* RemoteWrite( const std::wstring & a_String );

private:
    DWORD  m_InjectedProcessID;
    HANDLE m_InjectedProcessHandle;
};


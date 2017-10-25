//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "OrbitDll.h"

#include "OrbitLib.h"
#include "TcpClient.h"
#include "PrintVar.h"

using namespace std;

extern "C"
{
    void OrbitInit( void* a_Host )
    {
        PRINT_FUNC;
        std::string host = (char*)a_Host;
        Orbit::Init( host );
    }

    void OrbitInitRemote( void* a_Host )
    {
        PRINT_FUNC;
        std::string host = (char*)a_Host;
        Orbit::InitRemote( host );
    }

    bool OrbitIsConnected()
    {
        return GTcpClient && GTcpClient->IsValid();
    }

    bool OrbitStart()
    {
        if( OrbitIsConnected() )
        {
            Orbit::Start();
            return true;
        }

        return false;
    }

    bool OrbitStop()
    {
        if( OrbitIsConnected() )
        {
            Orbit::Stop();
            return true;
        }

        return false;
    }
}

#if _WIN32||_WIN64
BOOL WINAPI DllMain( _In_ HINSTANCE hinstDLL
                   , _In_ DWORD fdwReason
                   , _In_ LPVOID lpvReserved )
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        //OutputDebugString(L"DLL_PROCESS_ATTACH\n");
        break;

    case DLL_PROCESS_DETACH:
        //OutputDebugString(L"DLL_PROCESS_DETACH\n");
        break;

    case DLL_THREAD_ATTACH:
        //OutputDebugString(L"DLL_THREAD_ATTACH\n");
        break;

    case DLL_THREAD_DETACH:
        //OutputDebugString(L"DLL_THREAD_DETACH\n");
        break;
    default:
        //OutputDebugString(L"DLL_UNKNOWN\n");
        break;
    }

    return TRUE;
}
#endif

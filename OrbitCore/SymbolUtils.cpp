//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "SymbolUtils.h"
#include "OrbitDbgHelp.h"
#include "Capture.h"
#include "dia2.h" // #includes rpcndr.h -->  error C2872: 'byte': ambiguous symbol, if <cstddef> was #included first
#include "OrbitDia.h"
#include "OrbitModule.h"
#include "PrintVar.h"
#include "Path.h"

#include <tlhelp32.h>
#include <psapi.h>

using namespace std;
//-----------------------------------------------------------------------------
SymUtils::ModuleMap_t SymUtils::ListModules( HANDLE a_ProcessHandle)
{
    SCOPE_TIMER_LOG( L"SymUtils::ListModules" );

    const DWORD ModuleArraySize = 1024;
    DWORD NumModules = 0;
    TCHAR ModuleNameBuffer[MAX_PATH] = { 0 };
    TCHAR ModuleFullNameBuffer[MAX_PATH] = { 0 };
    HMODULE ModuleArray[1024];
    ModuleMap_t o_ModuleMap;

    /* Get handles to all the modules in the target process */
    SetLastError(NO_ERROR);
    if (!::EnumProcessModulesEx(a_ProcessHandle, &ModuleArray[0], ModuleArraySize * sizeof(HMODULE), &NumModules, LIST_MODULES_ALL))
    {
        string EnumProcessModulesExError = GetLastErrorAsString();
        PRINT_VAR(EnumProcessModulesExError);
        return o_ModuleMap;
    }

    NumModules /= sizeof(HMODULE);
    if (NumModules > ModuleArraySize)
    {
        PRINT_VAR("NumModules > ModuleArraySize");
        return o_ModuleMap;
    }

    for (DWORD i = 0; i <= NumModules; ++i)
    {
        HMODULE hModule = ModuleArray[i];
        GetModuleBaseName(a_ProcessHandle, hModule, ModuleNameBuffer, sizeof(ModuleNameBuffer));
        GetModuleFileNameEx(a_ProcessHandle, hModule, ModuleFullNameBuffer, sizeof(ModuleFullNameBuffer));

        MODULEINFO moduleInfo;
        memset(&moduleInfo, 0, sizeof(moduleInfo));
        GetModuleInformation(a_ProcessHandle, hModule, &moduleInfo, sizeof(MODULEINFO));

        shared_ptr<Module> module = make_shared<Module>();
        module->m_Name = ModuleNameBuffer;
        module->m_FullName = ModuleFullNameBuffer;
        module->m_Directory = Path::GetDirectory( module->m_FullName );
        module->m_AddressStart = (DWORD64)moduleInfo.lpBaseOfDll;
        module->m_AddressEnd = (DWORD64)moduleInfo.lpBaseOfDll + moduleInfo.SizeOfImage;
        module->m_EntryPoint = (DWORD64)moduleInfo.EntryPoint;

        fs::path filePath = module->m_FullName;
        filePath.replace_extension( ".pdb" );
        if( fs::exists( filePath ) )
        {
            module->m_FoundPdb = true;
            module->m_PdbSize = ::fs::file_size( filePath );
            module->m_PdbName = filePath.wstring();
        }
        else if( Contains( module->m_FullName, L"qt" ) )
        {
            wstring pdbName = Path::GetFileName( filePath.wstring() );
            filePath = wstring( L"C:\\Qt\\5.8\\msvc2015_64\\bin\\" ) + pdbName;
            if( fs::exists( filePath ) )
            {
                module->m_FoundPdb = true;
                module->m_PdbSize = ::fs::file_size( filePath );
                module->m_PdbName = filePath.wstring();
            }
        }

        module->m_ModuleHandle = hModule;

        if( module->m_AddressStart != 0 )
        {
            o_ModuleMap[module->m_AddressStart] = module;
        }
    }
    return o_ModuleMap;
}

//-----------------------------------------------------------------------------
bool SymUtils::GetLineInfo( DWORD64 a_Address, LineInfo & o_LineInfo )
{
    shared_ptr<Process> process = Capture::GTargetProcess;

    if( process )
    {
        return process->LineInfoFromAddress( a_Address, o_LineInfo );
    }

    return false;
}

//-----------------------------------------------------------------------------
ScopeSymCleanup::ScopeSymCleanup( HANDLE a_Handle ) : m_Handle( a_Handle )
{
}

//-----------------------------------------------------------------------------
ScopeSymCleanup::~ScopeSymCleanup()
{
    if (!SymCleanup( m_Handle ) )
    {
        ORBIT_ERROR;
    }
}

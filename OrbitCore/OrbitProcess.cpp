//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------


#include "OrbitProcess.h"
#include "OrbitModule.h"
#include "SymbolUtils.h"
#include "Pdb.h"
#include "Path.h"
#include "OrbitType.h"
#include "OrbitSession.h"
#include "OrbitThread.h"
#include "Injection.h"
#include "ScopeTimer.h"
#include "Serialization.h"

#include <llvm/DebugInfo/PDB/PDBSymbol.h>

#include <proc/readproc.h>
#include <proc/sysinfo.h> // uptime

using namespace std;
namespace fs = std::experimental::filesystem;

//-----------------------------------------------------------------------------
Process::Process(DWORD a_ID, ProcHandle_t handle) : 
    m_ID(a_ID), 
    m_Handle(std::move(handle)), 
    m_Name(m_Handle->cmd)
{
    m_FullName = Path::GetExecutableName();

    //m_Is64Bit = 
    m_IsElevated = m_Handle->ruid < 0 || m_Handle->ruid != m_Handle->euid;

    m_UpdateCpuTimer.Start();
}

//-----------------------------------------------------------------------------
Process::~Process()
{
    /*if( m_DebugInfoLoaded )
    {
        OrbitSymCleanup( m_Handle );
    }*/
}

//-----------------------------------------------------------------------------
void Process::LoadDebugInfo()
{
    if( !m_DebugInfoLoaded )
    {
        /*if( m_Handle == nullptr )
        {
            m_Handle = GetCurrentProcess();
        }*/

        // Initialize dbghelp
        //SymInit(m_Handle);

        // Load module information
        //wstring symbolPath = this->GetFullName().parent_path().c_str();
        //SymSetSearchPath(m_Handle, symbolPath.c_str());

        // List threads
        EnumerateThreads();
        m_DebugInfoLoaded = true;
    }
}

//-----------------------------------------------------------------------------
Process::ModuleMap_t Process::ListModules()
{
    SCOPE_TIMER_LOG( L"ListModules" );

    ClearTransients();
    m_Modules = SymUtils::ListModules(m_ID);

    for( auto & pair : m_Modules )
    {
        shared_ptr<Module>& module = pair.second;
        //wstring name = ToLower( module->m_Name );
        m_NameToModuleMap[module->m_Name] = module;
        module->LoadDebugInfo();
    }
    return m_Modules;
}

//-----------------------------------------------------------------------------
void Process::ClearTransients()
{
    m_Functions.clear();
    m_Types.clear();
    m_Globals.clear();
    m_WatchedVariables.clear();
    m_NameToModuleMap.clear();
}

//-----------------------------------------------------------------------------
void Process::EnumerateThreads()
{
    m_Threads.clear();
    m_ThreadIds.clear();

    uid_t pids[2];
    pids[0] = m_ID;
    pids[1] = 0;
    PROCTAB* procTable = openproc(PROC_PID | PROC_FILLMEM | PROC_FILLSTAT | PROC_FILLSTATUS | PROC_LOOSE_TASKS,
        pids, 1);

    while (1)
    {
        ProcHandle_t p(readeither(procTable, 0));
        if(!p)
            break;

        auto thread = make_shared<Thread>(p->tid, std::move(p));
        m_Threads.push_back( thread );
    }

    closeproc(procTable);
    
    for (shared_ptr<Thread> & thread : m_Threads)
    {
        m_ThreadIds.insert(thread->m_TID);
    }
}

//-----------------------------------------------------------------------------
void Process::UpdateCpuTime()
{
    static double uptime_sav;
    double uptime_cur;
    uptime(&uptime_cur, NULL);
    float ellapsedTime = uptime_cur - uptime_sav;
    if (ellapsedTime < 0.01) ellapsedTime = 0.005;
    uptime_sav = uptime_cur;

    long Hertz = sysconf(_SC_CLK_TCK);

    unsigned long long total_time = m_Handle->utime + m_Handle->stime;
    //total_time += (m_Handle->cutime + m_Handle->cstime);
    unsigned int seconds = uptime_cur - (double(m_Handle->start_time) / Hertz);
    m_CpuUsage = 100 * ((double(total_time) / Hertz) / seconds);
}

//-----------------------------------------------------------------------------
void Process::UpdateThreadUsage()
{
    for( auto & thread : m_Threads )
    {
        thread->UpdateUsage();
    }
}

//-----------------------------------------------------------------------------
void Process::SortThreadsByUsage()
{
    sort( m_Threads.begin(), m_Threads.end(),
        [](shared_ptr<Thread> & a_T0, shared_ptr<Thread> & a_T1)
        { 
            return a_T1->m_Usage.Latest() < a_T0->m_Usage.Latest(); 
        } );
}

//-----------------------------------------------------------------------------
void Process::SortThreadsById()
{
    sort( m_Threads.begin(), m_Threads.end(),
        [](shared_ptr<Thread> & a_T1, shared_ptr<Thread> & a_T0)
        { 
            return a_T1->m_TID < a_T0->m_TID; 
        } );
}

//-----------------------------------------------------------------------------
shared_ptr<Module> Process::FindModule( const fs::path& a_ModuleName )
{
    fs::path moduleName = a_ModuleName.stem();
    for( auto & it : m_Modules )
    {
        shared_ptr<Module> & module = it.second;
        if( module->m_Name.stem() == moduleName )
        {
            return module;
        }
    }

    return nullptr;
}

//-----------------------------------------------------------------------------
Function* Process::GetFunctionFromAddress( DWORD64 a_Address, bool a_IsExact )
{
    DWORD64 address = (DWORD64)a_Address;
    auto it = m_Modules.upper_bound( address );
    if( !m_Modules.empty() && it != m_Modules.begin() )
    {
        --it;
        shared_ptr<Module> & module = it->second;
        if( address < module->m_AddressEnd )
        {
            if( module->m_Pdb != nullptr )
            {
                if( a_IsExact )
                {
                    return module->m_Pdb->GetFunctionFromExactAddress( a_Address );
                }
                else
                {
                    return module->m_Pdb->GetFunctionFromProgramCounter( a_Address );
                }
            }
        }
    }

    return nullptr;
}

//-----------------------------------------------------------------------------
shared_ptr<Module> Process::GetModuleFromAddress( DWORD64 a_Address )
{
    DWORD64 address = (DWORD64)a_Address;
    auto it = m_Modules.upper_bound(address);
    if (!m_Modules.empty() && it != m_Modules.begin())
    {
        --it;
        shared_ptr<Module> & module = it->second;
        return module;
    }

    return nullptr;
}

//-----------------------------------------------------------------------------
std::unique_ptr<llvm::pdb::PDBSymbol> Process::SymbolFromAddress( DWORD64 a_Address )
{
    shared_ptr<Module> module = GetModuleFromAddress( a_Address );
    if( module && module->m_Pdb )
    {
        return module->m_Pdb->SymbolFromAddress( a_Address );
    }
    return nullptr;
}

//-----------------------------------------------------------------------------
bool Process::LineInfoFromAddress( DWORD64 a_Address, LineInfo & o_LineInfo )
{
    shared_ptr<Module> module = GetModuleFromAddress( a_Address );
    if( module && module->m_Pdb )
    {
        return module->m_Pdb->LineInfoFromAddress( a_Address, o_LineInfo );
    }

    return false;
}

//-----------------------------------------------------------------------------
void Process::RefreshWatchedVariables()
{
    for( shared_ptr<Variable> var : m_WatchedVariables )
    {
        var->SyncValue();
    }
}

//-----------------------------------------------------------------------------
void Process::ClearWatchedVariables()
{
    m_WatchedVariables.clear();
}

//-----------------------------------------------------------------------------
void Process::AddType(Type & a_Type)
{
    /*bool isPtr = a_Type.m_Name.find(L"Pointer to") != string::npos;
    if (!isPtr)
    {
        unsigned long long typeHash = a_Type.Hash();
        auto it = m_UniqueTypeHash.insert(typeHash);
        if (it.second == true)
        {
            m_Types.push_back(&a_Type);
        }
    }*/
}

//-----------------------------------------------------------------------------
void Process::AddModule( shared_ptr<Module> & a_Module )
{
    m_Modules[a_Module->m_AddressStart] = a_Module;
}

#include <llvm/DebugInfo/CodeView/GUID.h>
std::string GuidToString(llvm::codeview::GUID guid)
{
    ostringstream os;
    //os << guid;
    return os.str();
}

//-----------------------------------------------------------------------------
void Process::FindPdbs( const vector<  fs::path >& a_SearchLocations )
{
    unordered_map< wstring, vector<fs::path> > nameToPaths;

    // Populate list of all available pdb files
    for( const fs::path& dir : a_SearchLocations )
    {
        vector<fs::path> pdbFiles = Path::ListFiles( dir, L".pdb" );
        for( const fs::path& pdb : pdbFiles )
        {
            wstring pdbLower = ToLower( pdb.filename().wstring() );
            nameToPaths[pdbLower].push_back( pdb );
        }
    }

    // Find matching pdb
    for( auto & modulePair : m_Modules )
    {
        shared_ptr<Module> module =  modulePair.second;

        if( !module->m_FoundPdb )
        {
            fs::path moduleName = module->m_Name;
            wstring pdbName = Path::StripExtension( moduleName ).wstring() + L".pdb";

            const vector<fs::path>& pdbs = nameToPaths[pdbName];

            for( const fs::path& pdb : pdbs )
            {
                module->m_PdbName = pdb;
                module->m_FoundPdb = true;
                module->LoadDebugInfo();

                wstring signature = s2ws( GuidToString( module->m_Pdb->GetGuid() ) );

                if( Contains( module->m_DebugSignature, signature ) )
                {
                    // Found matching pdb
                    module->m_PdbSize = fs::file_size( module->m_PdbName );
                    break;
                }
                else
                {
                    module->m_FoundPdb = false;
                }
            }
        }       
    }
}

//-----------------------------------------------------------------------------
bool Process::SetPrivilege( LPCTSTR a_Name, bool a_Enable )
{
    /*HANDLE hToken;
    if( !OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken ) )
    {
        ORBIT_ERROR;
        PRINT_VAR( GetLastErrorAsString() );
    }

    TOKEN_PRIVILEGES tp;
    LUID luid;
    if( !LookupPrivilegeValue(NULL, a_Name, &luid ) )
    {
        ORBIT_ERROR;
        PRINT( "LookupPrivilegeValue error: " );
        PRINT_VAR( GetLastErrorAsString() );
        return false;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = a_Enable ? SE_PRIVILEGE_ENABLED : 0;

    if( !AdjustTokenPrivileges( hToken, FALSE, &tp, sizeof( TOKEN_PRIVILEGES ), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL ) )
    {
        ORBIT_ERROR;
        PRINT( "AdjustTokenPrivileges error: " );
        PRINT_VAR( GetLastErrorAsString() );
        return false;
    }

    if( GetLastError() == ERROR_NOT_ALL_ASSIGNED )
    {
        PRINT( "The token does not have the specified privilege. \n" );
        return false;
    }
    */
    return true;
}

//-----------------------------------------------------------------------------
DWORD64 Process::GetOutputDebugStringAddress()
{
    /*auto it = m_NameToModuleMap.find( L"kernelbase.dll" );
    if( it != m_NameToModuleMap.end() )
    {
        shared_ptr<Module> module = it->second;
        auto remoteAddr = Injection::GetRemoteProcAddress( GetHandle(), 
            module->m_ModuleHandle, "OutputDebugStringA" );
        return (DWORD64)remoteAddr;
    }
    */
    return 0;
}

//-----------------------------------------------------------------------------
DWORD64 Process::GetRaiseExceptionAddress()
{
    /*auto it = m_NameToModuleMap.find( L"kernelbase.dll" );
    if (it != m_NameToModuleMap.end())
    {
        shared_ptr<Module> module = it->second;
        auto remoteAddr = Injection::GetRemoteProcAddress(GetHandle(), 
            module->m_ModuleHandle, "RaiseException");
        return (DWORD64)remoteAddr;
    }*/

    return 0;
}

//-----------------------------------------------------------------------------
void Process::FindCoreFunctions()
{
    return;
#if 0 // XXX: Unreachable code
    SCOPE_TIMER_LOG(L"FindCoreFunctions");

    const auto prio = oqpi::task_priority::normal;
    oqpi_tk::scheduler().workersCount( prio );
    //int numWorkers = oqpi::thread::hardware_concurrency();

    oqpi_tk::parallel_for( "FindAllocFreeFunctions", (int32_t)m_Functions.size(), [&]( int32_t, int32_t a_ElementIndex )
    {
        Function* func = m_Functions[a_ElementIndex];
        const wstring & name = func->Lower();

        if( Contains( name, L"operator new" ) || Contains( name, L"FMallocBinned::Malloc" ) )
        {
            func->Select();
            func->m_OrbitType = Function::ALLOC;
        }
        else if( Contains( name, L"operator delete" ) || name == L"FMallocBinned::Free" )
        {
            func->Select();
            func->m_OrbitType = Function::FREE;
        } 
        else if( Contains( name, L"realloc" ) )
        {
            func->Select();
            func->m_OrbitType = Function::REALLOC;
        }
    } );
#endif
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE( Process, 0 )
{
    ORBIT_NVP_VAL( 0, m_Name );
    ORBIT_NVP_VAL( 0, m_FullName );
    ORBIT_NVP_VAL( 0, m_ID );
    ORBIT_NVP_VAL( 0, m_IsElevated );
    ORBIT_NVP_VAL( 0, m_CpuUsage );
    ORBIT_NVP_VAL( 0, m_Is64Bit );
    ORBIT_NVP_VAL( 0, m_DebugInfoLoaded );
    ORBIT_NVP_VAL( 0, m_IsRemote );
    ORBIT_NVP_VAL( 0, m_Modules );
    ORBIT_NVP_VAL( 0, m_NameToModuleMap );
    ORBIT_NVP_VAL( 0, m_ThreadIds );
}

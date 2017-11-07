//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "PTM/OrbitProcess.h"
#include "PTM/OrbitModule.h"
#include "PTM/ModuleUtils.h"
#include "Path.h"
#include "PTM/OrbitThread.h"
#include "ScopeTimer.h"
#include "Serialization.h"

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
}

//-----------------------------------------------------------------------------
void Process::PopulateModules()
{
    SCOPE_TIMER_LOG( L"Process::ListModules" );

    //ClearTransients();
    m_Modules = ModuleUtils::ListModules(m_ID);

    for( auto & pair : m_Modules )
    {
        shared_ptr<Module>& module = pair.second;
        //wstring name = ToLower( module->m_Name );
        m_NameToModuleMap[module->m_Name] = module;
        //module->LoadDebugInfo();
    }
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
ORBIT_SERIALIZE( Process, 0 )
{
    ORBIT_NVP_VAL( 0, m_Name );
    ORBIT_NVP_VAL( 0, m_FullName );
    ORBIT_NVP_VAL( 0, m_ID );
    ORBIT_NVP_VAL( 0, m_IsElevated );
    ORBIT_NVP_VAL( 0, m_CpuUsage );
    ORBIT_NVP_VAL( 0, m_Is64Bit );
    ORBIT_NVP_VAL( 0, m_IsRemote );
    ORBIT_NVP_VAL( 0, m_Modules );
    ORBIT_NVP_VAL( 0, m_NameToModuleMap );
    ORBIT_NVP_VAL( 0, m_ThreadIds );
}

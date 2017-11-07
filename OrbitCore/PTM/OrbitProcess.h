//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "BaseTypes.h"
#include "SerializationMacros.h"
#include "Threading.h"
#include "PTM/OrbitThread.h"

#include <unordered_set>
#include <memory>
#include <map>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

//class Thread;
struct Module;

//#include <proc/readproc.h>
/*struct proc_t;
extern "C" void freeproc(proc_t* p);

struct ProcDeleter {
    void operator()(proc_t* b) { freeproc(b); }
};
typedef std::unique_ptr<proc_t, ProcDeleter> ProcHandle_t;
*/

//-----------------------------------------------------------------------------
class Process
{
public:
    //Process(DWORD a_ID); // remote (minidump)
    Process(DWORD a_ID, ProcHandle_t handle);
    ~Process();

    typedef std::map< DWORD64, std::shared_ptr<Module> > ModuleMap_t;
    typedef std::map< fs::path, std::shared_ptr<Module> > ModuleMapByName_t;

    void PopulateModules();
    void EnumerateThreads();
    void UpdateCpuTime();
    void UpdateThreadUsage();
    void SortThreadsByUsage();
    void SortThreadsById();
    bool IsElevated() const { return m_IsElevated; }
    bool HasThread( DWORD a_ThreadId ) const { return m_ThreadIds.find( a_ThreadId ) != m_ThreadIds.end(); }
    void AddModule( std::shared_ptr<Module> & a_Module );

    static bool SetPrivilege( LPCTSTR a_Name, bool a_Enable );

    ModuleMap_t& GetModules() { return m_Modules; }
    ModuleMapByName_t& GetNameToModulesMap() { return m_NameToModuleMap; }
    std::shared_ptr<Module> FindModule( const fs::path& a_ModuleName );

    const fs::path& GetName() const { return m_Name; }
    const fs::path& GetFullName() const { return m_FullName; }

    DWORD GetID() const { return m_ID; }
    double GetCpuUsage() const { return m_CpuUsage; }
    const ProcHandle_t& GetHandle() const { return m_Handle; }
    bool GetIs64Bit() const { return m_Is64Bit; }
    bool GetIsRemote() const { return m_IsRemote; }
    void SetIsRemote( bool val ) { m_IsRemote = val; }

    std::shared_ptr<Module> GetModuleFromAddress( DWORD64 a_Address );

    std::vector<std::shared_ptr<Thread> >& GetThreads(){ return m_Threads; }

    ORBIT_SERIALIZABLE;

private:
    DWORD       m_ID;
    ProcHandle_t m_Handle;
    fs::path    m_Name;
    fs::path    m_FullName;
    bool        m_IsElevated = false;

    double      m_CpuUsage = 0.0;
    Timer       m_UpdateCpuTimer;
    bool        m_Is64Bit = false;
    bool        m_IsRemote = false;

    ModuleMap_t m_Modules;
    ModuleMapByName_t m_NameToModuleMap;
    std::vector<std::shared_ptr<Thread> >   m_Threads;
    std::unordered_set<DWORD>               m_ThreadIds;
};


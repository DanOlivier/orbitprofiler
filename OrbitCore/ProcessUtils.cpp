//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "ProcessUtils.h"
#include "Log.h"

#include <proc/readproc.h>
using namespace std;

//-----------------------------------------------------------------------------
ProcessList::ProcessList()
{
    Refresh();
}

//-----------------------------------------------------------------------------
void ProcessList::Clear()
{
    m_Processes.clear();
    m_ProcessesMap.clear();
}

//-----------------------------------------------------------------------------
void ProcessList::Refresh()
{
    m_Processes.clear();
    unordered_map< DWORD, shared_ptr< Process > > previousProcessesMap = m_ProcessesMap;
    m_ProcessesMap.clear();

    uid_t uids[2];
    uids[0] = getuid();
    uids[1] = 0;
    PROCTAB* procTable = openproc(PROC_UID | PROC_FILLCOM | PROC_FILLMEM | PROC_FILLSTAT | PROC_FILLSTATUS,
        uids, 1);
    while (1)
    {
        ProcHandle_t p(readproc(procTable, 0));
        if(!p)
            break;

        if (p->tgid == getpid())
            continue;

        auto tgid = p->tgid;
        auto it = previousProcessesMap.find( p->tgid );
        if( it != previousProcessesMap.end() )
        {
            // Add existing process
            m_Processes.push_back( it->second );
        }
        else
        {
            // Process was not there previously
            if(p->cmdline)
            {
                auto process = make_shared<Process>(p->tgid, std::move(p));
                m_Processes.push_back( process );
            }
        }

        m_ProcessesMap[tgid] = m_Processes.back();
    }

    SortByCPU();
    closeproc(procTable);
}

//-----------------------------------------------------------------------------
void ProcessList::SortByID()
{
    sort( m_Processes.begin(), m_Processes.end(), []( shared_ptr<Process> & a_P1, shared_ptr<Process> & a_P2 )
    { 
        return a_P1->GetID() < a_P2->GetID();
    } );
}

//-----------------------------------------------------------------------------
void ProcessList::SortByName()
{
    sort( m_Processes.begin(), m_Processes.end(), []( shared_ptr<Process> & a_P1, shared_ptr<Process> & a_P2 )
    { 
        return a_P1->GetName() < a_P2->GetName(); 
    } );
}

//-----------------------------------------------------------------------------
void ProcessList::SortByCPU()
{
    sort(m_Processes.begin(), m_Processes.end(), []( shared_ptr<Process> & a_P1, shared_ptr<Process> & a_P2 ){ return a_P1->GetCpuUsage() < a_P2->GetCpuUsage(); });
}

//-----------------------------------------------------------------------------
void ProcessList::UpdateCpuTimes()
{
    for( shared_ptr< Process > & process : m_Processes )
    {
        process->UpdateCpuTime();
    }
}

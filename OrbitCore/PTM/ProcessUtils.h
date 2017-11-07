//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

//#include "PTM/OrbitProcess.h"
#include "BaseTypes.h"

#include <unordered_map>
#include <vector>
#include <memory>

class Process;

//-----------------------------------------------------------------------------
struct ProcessList
{
    ProcessList();
    void Refresh();
    void Clear();
    void SortByID();
    void SortByName();
    void SortByCPU();
    void UpdateCpuTimes();

    std::vector< std::shared_ptr<Process> > m_Processes;
    std::unordered_map< DWORD, std::shared_ptr<Process> > m_ProcessesMap;
};


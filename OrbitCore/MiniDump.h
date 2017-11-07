//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "OrbitType.h"
#include "PTM/OrbitModule.h"

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;    

//-----------------------------------------------------------------------------
namespace google_breakpad { class Minidump; }
class Process;

//-----------------------------------------------------------------------------
class MiniDump
{
public:
    MiniDump( const fs::path& a_FileName );
    ~MiniDump();

    std::shared_ptr<Process> ToOrbitProcess(const std::vector<fs::path>& symbolLocations);

protected:
    std::vector<Module>        m_Modules;
    std::unique_ptr<google_breakpad::Minidump> m_MiniDump;
};
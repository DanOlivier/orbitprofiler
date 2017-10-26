//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "Message.h"

#include <string>
#include <vector>
#include <atomic>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

class Debugger
{
public:
    Debugger();
    ~Debugger();

    void LaunchProcess( const fs::path & a_ProcessName, const fs::path & a_WorkingDir, const std::wstring & a_Args );
    void MainTick();
    void SendThawMessage();

protected:
    void DebuggerThread( const fs::path & a_ProcessName, const fs::path & a_WorkingDir, const std::wstring & a_Args );

private:
    OrbitWaitLoop     m_WaitLoop;
    std::atomic<bool> m_LoopReady;
    DWORD             m_ProcessID;
};

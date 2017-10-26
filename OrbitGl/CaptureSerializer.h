//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>
#include <unordered_map>
#include "OrbitType.h"
#include "SerializationMacros.h"

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;    

//-----------------------------------------------------------------------------
class CaptureSerializer
{
public:
    CaptureSerializer();
    void Save( const fs::path& a_FileName );
    void Load( const fs::path& a_FileName );

    template <class T> void Save( T & a_Archive );

    class  TimeGraph*        m_TimeGraph;
    class  SamplingProfiler* m_SamplingProfiler;

    fs::path                 m_CaptureName;
    int                      m_Version;
    int                      m_TimerVersion;
    int                      m_NumTimers;
    int                      m_SizeOfTimer;

    ORBIT_SERIALIZABLE;
};

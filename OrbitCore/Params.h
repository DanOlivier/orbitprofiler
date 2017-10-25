//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "SerializationMacros.h"
#include "BaseTypes.h"
#include <experimental/filesystem>

struct Params
{
    Params();
    void Load();
    void Save();
    
    void AddToPdbHistory( const std::string & a_PdbName );
    void ScanPdbCache();

public:
    bool  m_LoadTypeInfo;
    bool  m_SendCallStacks;
    bool  m_TrackContextSwitches;
    bool  m_TrackSamplingEvents;
    bool  m_UnrealSupport;
    bool  m_UnitySupport;
    bool  m_StartPaused;
    bool  m_AllowUnsafeHooking;
    bool  m_HookOutputDebugString;
    int   m_MaxNumTimers;
    float m_FontSize;
    int   m_Port;
    DWORD64 m_NumBytesAssembly;
    std::string m_DiffExe;
    std::string m_DiffArgs;
    std::vector< std::string > m_PdbHistory;

    //namespace fs = std::experimental::filesystem;
    std::unordered_map< std::string, std::experimental::filesystem::path > m_CachedPdbsMap;

    ORBIT_SERIALIZABLE;
};

extern Params GParams;


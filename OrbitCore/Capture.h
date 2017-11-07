//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once


#include "CallstackTypes.h"
#include <string>
#include <chrono>
#include "OrbitType.h"
#include "Threading.h"

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;    

class Process;
class Session;
class SamplingProfiler;
class Function;
struct CallStack;
class TcpServer;
class TextBox;
class EventTracer;

class Capture
{
public:
    Capture();
    
    bool Inject( bool a_WaitForConnection = true );
    bool Connect();
    bool InjectRemote();
    void SetTargetProcess(const std::shared_ptr<Process>& a_Process);
    bool StartCapture();
    void StopCapture();
    void ToggleRecording();
    void ClearCaptureData();
    void PreFunctionHooks();
    void SendFunctionHooks();
    void SendDataTrackingInfo();
    void StartSampling();
    void StopSampling();
    bool IsCapturing();
    void Update();
    void DisplayStats();
    void TestHooks();
    void OpenCapture( const fs::path & a_CaptureName );
    bool IsOtherInstanceRunning();
    void LoadSession( const std::shared_ptr<Session> & a_Session );
    void SaveSession( const fs::path & a_FileName );
    void NewSamplingProfiler();
    bool IsTrackingEvents();
    bool IsRemote();
    void RegisterZoneName( DWORD64 a_ID, char* a_Name );
    void AddCallstack( CallStack & a_CallStack );
    std::shared_ptr<CallStack> GetCallstack( CallstackID a_ID );
    //void CheckForUnrealSupport();
    void PreSave();
    
    typedef void (*LoadPdbAsyncFunc)( const std::vector<fs::path> & a_Modules );
    void SetLoadPdbAsyncFunc( LoadPdbAsyncFunc a_Func ){ m_LoadPdbAsync = a_Func; }

public:
    bool         m_Injected = false;
    bool         m_IsConnected = false;
    std::string  m_InjectedProcess;
    std::wstring m_InjectedProcessW;
    double       m_OpenCaptureTime = 0;
    int          m_CapturePort = 0;
    std::wstring m_CaptureHost = L"localhost";
    std::wstring m_PresetToLoad;  // TODO: allow multiple presets
    std::wstring m_ProcessToInject;
    bool         m_IsSampling = false;
    bool         m_IsTesting = false;
    int          m_NumSamples = 0;
    int          m_NumSamplingTicks = 0;
    int          m_FunctionIndex = -1;
    int          m_NumInstalledHooks = 0;
    bool         m_HasSamples = false;
    bool         m_HasContextSwitches = false;

    Timer        m_TestTimer;
    //ULONG64      m_MainFrameFunction = 0;
    ULONG64      m_NumContextSwitches = 0;
    ULONG64      m_NumProfileEvents = 0;

    std::shared_ptr<SamplingProfiler> m_SamplingProfiler;
    std::shared_ptr<Process>          m_TargetProcess;
    std::shared_ptr<Session>          m_SessionPresets;
    std::shared_ptr<CallStack>        m_SelectedCallstack;

    void (*m_ClearCaptureDataFunc)();
    void (*m_SamplingDoneCallback)(std::shared_ptr<SamplingProfiler>& a_SamplingProfiler);

    std::map<ULONG64, Function*> m_SelectedFunctionsMap;
    std::map<ULONG64, Function*> m_VisibleFunctionsMap;
    std::unordered_map<ULONG64, ULONG64> m_FunctionCountMap;
    std::vector<ULONG64> m_SelectedAddressesByType[Function::NUM_TYPES];
    std::unordered_map<DWORD64, std::shared_ptr<CallStack> > m_Callstacks;
    std::unordered_map<DWORD64, std::string> m_ZoneNames;
    TextBox* m_SelectedTextBox = 0;
    ThreadID m_SelectedThreadId = 0;
    Timer m_CaptureTimer;
    std::chrono::system_clock::time_point m_CaptureTimePoint;
    Mutex m_CallstackMutex;
    LoadPdbAsyncFunc m_LoadPdbAsync;
    //bool m_UnrealSupported = false;
    std::unique_ptr<EventTracer> m_EventTracer;
};

extern std::shared_ptr<Capture> GCapture;
//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "DataViewTypes.h"
#include "CoreApp.h"
#include "CrashHandler.h"
#include "Message.h"
#include "ModuleManager.h"

#include <functional>
#include <memory>
#include <string>
#include <queue>
#include <map>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

struct CallStack;
class Process;
class Rule;

class ProcessesDataView;
class ModulesDataView;
class FunctionsDataView;
class LiveFunctionsDataView;

class LiveFunctionsDataView;
class CallStackDataView;
class TypesDataView;
class GlobalsDataView;
class SessionsDataView;
class CaptureWindow;
class LogDataView;
class RuleEditor;

class DataViewModel;
class SamplingProfiler;
class SamplingReport;
class Debugger;
class EventTracer;
struct Module;
class Capture;

//-----------------------------------------------------------------------------
class OrbitApp : public CoreApp
{
public:
    OrbitApp();
    ~OrbitApp();

    static bool Init();
    static void PostInit();
    static int  OnExit();
    
    void MainTick();

    void CheckLicense();
    void SetLicense( const std::wstring & a_License );
    std::string GetVersion();
    void CheckForUpdate();
    void CheckDebugger();

    fs::path GetCaptureFileName();
    fs::path GetSessionFileName();
    void OnSaveSession( const fs::path& a_FileName );
    void OnLoadSession( const fs::path& a_FileName );
    void OnSaveCapture( const fs::path& a_FileName );
    void OnLoadCapture( const fs::path& a_FileName );
    void ClearCaptureData();    
    void OnOpenPdb(const fs::path& a_FileName);
    void OnLaunchProcess(const fs::path& a_ProcessName, 
        const fs::path& a_WorkingDir, const std::wstring a_Args );
    void Inject(const fs::path& a_FileName);
    void StartCapture();
    void StopCapture();
    void ToggleCapture();
    void OnOpenCapture(const fs::path& a_FileName);
    void OnDisconnect();
    void OnPdbLoaded();
    void LogMsg( const std::wstring & a_Msg ) override;
    void CallHome();
    void CallHomeThread();
    void SetCallStack( std::shared_ptr<CallStack> a_CallStack );
    void LoadFileMapping();
    void LoadSymbolsFile();
    void ListSessions();
    void SetRemoteProcess( std::shared_ptr<Process> a_Process );
    void AddWatchedVariable( Variable* a_Variable );
    void UpdateVariable( Variable* a_Variable ) override;
    void ClearWatchedVariables();
    void RefreshWatch();
    virtual void Disassemble( Function* a_Function, const char* a_MachineCode, int a_Size ) override;

    int* GetScreenRes() { return m_ScreenRes; }

    void RegisterProcessesDataView( ProcessesDataView* a_Processes );
    void RegisterModulesDataView( ModulesDataView* a_Modules );
    void RegisterFunctionsDataView( FunctionsDataView* a_Functions );
    void RegisterLiveFunctionsDataView( LiveFunctionsDataView* a_Functions );
    void RegisterCallStackDataView( CallStackDataView* a_Callstack );
    void RegisterTypesDataView( TypesDataView* a_Types );
    void RegisterGlobalsDataView( GlobalsDataView* a_Globals );
    void RegisterSessionsDataView( SessionsDataView* a_Sessions );
    void RegisterCaptureWindow( CaptureWindow* a_Capture );
    void RegisterOutputLog( LogDataView* a_Log );
    void RegisterRuleEditor( RuleEditor* a_RuleEditor );

    void Unregister( DataViewModel* a_Model );
    bool SelectProcess( const fs::path& a_Process );
    bool SelectProcess( unsigned long a_ProcessID );
    bool Inject( unsigned long a_ProcessId );
    static void AddSamplingReport( std::shared_ptr<SamplingProfiler> & a_SamplingProfiler );
    static void AddSelectionReport( std::shared_ptr<SamplingProfiler> & a_SamplingProfiler );
    void GoToCode( DWORD64 a_Address );

    // Callbacks
    typedef std::function< void( DataViewType a_Type ) > RefreshCallback;
    void AddRefreshCallback(RefreshCallback a_Callback){ m_RefreshCallbacks.push_back(a_Callback); }
    
    typedef std::function< void( std::shared_ptr<SamplingReport> ) > SamplingReportCallback;
    void AddSamplingReoprtCallback(SamplingReportCallback a_Callback) { m_SamplingReportsCallbacks.push_back(a_Callback); }
    void AddSelectionReportCallback(SamplingReportCallback a_Callback) { m_SelectionReportCallbacks.push_back( a_Callback ); }
    
    typedef std::function< void( Variable* a_Variable ) > WatchCallback;
    void AddWatchCallback( WatchCallback a_Callback ){ m_AddToWatchCallbacks.push_back( a_Callback ); }
    void AddUpdateWatchCallback( WatchCallback a_Callback ){ m_UpdateWatchCallbacks.push_back( a_Callback ); }
    
    void FireRefreshCallbacks( DataViewType a_Type = DataViewType::ALL );
    void Refresh( DataViewType a_Type = DataViewType::ALL ){ FireRefreshCallbacks( a_Type ); }
    void AddUiMessageCallback( std::function< void( const std::wstring & ) > a_Callback );
    
    typedef std::function< std::wstring( const std::wstring & a_Caption, const std::wstring & a_Dir, const std::wstring & a_Filter ) > FindFileCallback;
    void SetFindFileCallback( FindFileCallback a_Callback ){ m_FindFileCallback = a_Callback; }
    std::wstring FindFile( const std::wstring & a_Caption, const std::wstring & a_Dir, const std::wstring & a_Filter );

    void SetCommandLineArguments( const std::vector<std::string> & a_Args );

    void SendToUiAsync( const std::wstring & a_Msg ) override;
    void SendToUiNow( const std::wstring & a_Msg ) override;
    void NeedsRedraw();

    const std::map<std::wstring, std::wstring>& GetFileMapping() { return m_FileMapping; }

    void EnqueueModuleToLoad( const std::shared_ptr<Module> & a_Module );
    void LoadModules();

    void SetTrackContextSwitches( bool a_Value );
    bool GetTrackContextSwitches();

    void EnableUnrealSupport( bool a_Value );
    virtual bool GetUnrealSupportEnabled() override;

    void EnableSampling( bool a_Value );
    virtual bool GetSamplingEnabled() override;

    void EnableUnsafeHooking( bool a_Value );
    virtual bool GetUnsafeHookingEnabled() override;

    void EnableOutputDebugString( bool a_Value );
    virtual bool GetOutputDebugStringEnabled() override;

    void RequestThaw(){ m_NeedsThawing = true; }
    void OnMiniDump( const Message & a_Message );
    void LaunchRuleEditor( Function* a_Function );

    RuleEditor* GetRuleEditor() { return m_RuleEditor; }

private:
    std::vector<RefreshCallback>        m_RefreshCallbacks;
    std::vector<WatchCallback>          m_AddToWatchCallbacks;
    std::vector<WatchCallback>          m_UpdateWatchCallbacks;
    std::vector<SamplingReportCallback> m_SamplingReportsCallbacks;
    std::vector<SamplingReportCallback> m_SelectionReportCallbacks;
    std::vector<DataViewModel*>         m_Panels;
    FindFileCallback                    m_FindFileCallback;
    
    ProcessesDataView*      m_ProcessesDataView = nullptr;
    ModulesDataView*        m_ModulesDataView = nullptr;
    FunctionsDataView*      m_FunctionsDataView = nullptr;
    LiveFunctionsDataView*  m_LiveFunctionsDataView = nullptr;
    CallStackDataView*      m_CallStackDataView = nullptr;
    TypesDataView*          m_TypesDataView = nullptr;
    GlobalsDataView*        m_GlobalsDataView = nullptr;
    SessionsDataView*       m_SessionsDataView = nullptr;
    CaptureWindow*          m_CaptureWindow = nullptr;
    LogDataView*            m_Log = nullptr;
    RuleEditor*             m_RuleEditor = nullptr;
    int                     m_ScreenRes[2];
    bool                    m_HasPromptedForUpdate = false;
    bool                    m_NeedsThawing = false;
    bool                    m_UnrealEnabled = true;

    ModuleManager           m_ModuleManager;
    std::vector<fs::path>   m_SymbolLocations;
    //std::shared_ptr<Capture> m_Capture;
    
    std::vector< std::shared_ptr<SamplingReport> > m_SamplingReports;
    std::map<std::wstring, std::wstring> m_FileMapping;
    std::function< void( const std::wstring & ) > m_UiCallback;

    std::wstring m_User;
    std::wstring m_License;

    std::queue< std::shared_ptr<Module> > m_ModulesToLoad;

    std::unique_ptr<Debugger>    m_Debugger;
    int                          m_NumTicks = 0;
    CrashHandler       m_CrashHandler;
};

//-----------------------------------------------------------------------------
extern OrbitApp* GOrbitApp;


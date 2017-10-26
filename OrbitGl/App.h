//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "DataViewTypes.h"
#include "CoreApp.h"
#include "CrashHandler.h"
#include "Message.h"

#include <functional>
#include <memory>
#include <string>
#include <queue>
#include <map>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

struct CallStack;
class Process;

//-----------------------------------------------------------------------------
class OrbitApp : public CoreApp
{
public:
    OrbitApp();
    ~OrbitApp();

    static bool Init();
    static void PostInit();
    static int  OnExit();
    static void MainTick();
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

    void RegisterProcessesDataView( class ProcessesDataView* a_Processes );
    void RegisterModulesDataView( class ModulesDataView* a_Modules );
    void RegisterFunctionsDataView( class FunctionsDataView* a_Functions );
    void RegisterLiveFunctionsDataView( class LiveFunctionsDataView* a_Functions );
    void RegisterCallStackDataView( class CallStackDataView* a_Callstack );
    void RegisterTypesDataView( class TypesDataView* a_Types );
    void RegisterGlobalsDataView( class GlobalsDataView* a_Globals );
    void RegisterSessionsDataView( class SessionsDataView* a_Sessions );
    void RegisterCaptureWindow( class CaptureWindow* a_Capture );
    void RegisterOutputLog( class LogDataView* a_Log );
    void RegisterRuleEditor( class RuleEditor* a_RuleEditor );

    void Unregister( class DataViewModel* a_Model );
    bool SelectProcess( const fs::path& a_Process );
    bool SelectProcess( unsigned long a_ProcessID );
    bool Inject( unsigned long a_ProcessId );
    static void AddSamplingReport( std::shared_ptr< class SamplingProfiler> & a_SamplingProfiler );
    static void AddSelectionReport( std::shared_ptr<SamplingProfiler> & a_SamplingProfiler );
    void GoToCode( DWORD64 a_Address );

    // Callbacks
    typedef std::function< void( DataViewType a_Type ) > RefreshCallback;
    void AddRefreshCallback(RefreshCallback a_Callback){ m_RefreshCallbacks.push_back(a_Callback); }
    typedef std::function< void( std::shared_ptr<class SamplingReport> ) > SamplingReportCallback;
    void AddSamplingReoprtCallback(SamplingReportCallback a_Callback) { m_SamplingReportsCallbacks.push_back(a_Callback); }
    void AddSelectionReportCallback( SamplingReportCallback a_Callback ) { m_SelectionReportCallbacks.push_back( a_Callback ); }
    typedef std::function< void( Variable* a_Variable ) > WatchCallback;
    void AddWatchCallback( WatchCallback a_Callback ){ m_AddToWatchCallbacks.push_back( a_Callback ); }
    void AddUpdateWatchCallback( WatchCallback a_Callback ){ m_UpdateWatchCallbacks.push_back( a_Callback ); }
    void FireRefreshCallbacks( DataViewType a_Type = DataViewType::ALL );
    void Refresh( DataViewType a_Type = DataViewType::ALL ){ FireRefreshCallbacks( a_Type ); }
    void AddUiMessageCallback( std::function< void( const std::wstring & ) > a_Callback );
    typedef std::function< std::wstring( const std::wstring & a_Caption, const std::wstring & a_Dir, const std::wstring & a_Filter ) > FindFileCallback;
    void SetFindFileCallback( FindFileCallback a_Callback ){ m_FindFileCallback = a_Callback; }
    std::wstring FindFile( const std::wstring & a_Caption, const std::wstring & a_Dir, const std::wstring & a_Filter );

    void SetCommandLineArguments( const std::vector< std::string > & a_Args );
    const std::vector< std::string > & GetCommandLineArguments(){ return m_Arguments; }

    void SendToUiAsync( const std::wstring & a_Msg ) override;
    void SendToUiNow( const std::wstring & a_Msg ) override;
    void NeedsRedraw();

    const std::map< std::wstring, std::wstring > & GetFileMapping() { return m_FileMapping; }

    void EnqueueModuleToLoad( const std::shared_ptr<struct Module> & a_Module );
    void LoadModules();
    bool IsLoading();
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
    virtual const std::unordered_map<DWORD64, std::shared_ptr<class Rule> >* GetRules() override;

private:
    std::vector< std::string >            m_Arguments;
    std::vector< RefreshCallback >        m_RefreshCallbacks;
    std::vector< WatchCallback >          m_AddToWatchCallbacks;
    std::vector< WatchCallback >          m_UpdateWatchCallbacks;
    std::vector< SamplingReportCallback > m_SamplingReportsCallbacks;
    std::vector< SamplingReportCallback > m_SelectionReportCallbacks;
    std::vector< class DataViewModel* >   m_Panels;
    FindFileCallback                      m_FindFileCallback;
    
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

    std::vector< std::shared_ptr< class SamplingReport> > m_SamplingReports;
    std::map< std::wstring, std::wstring > m_FileMapping;
    std::function< void( const std::wstring & ) > m_UiCallback;

    std::wstring m_User;
    std::wstring m_License;

    std::queue< std::shared_ptr<struct Module> > m_ModulesToLoad;

    class EventTracer* m_EventTracer = nullptr;
    class Debugger*    m_Debugger = nullptr;
    int m_NumTicks = 0;
    CrashHandler       m_CrashHandler;
};

//-----------------------------------------------------------------------------
extern class OrbitApp* GOrbitApp;


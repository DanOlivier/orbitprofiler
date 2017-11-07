//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Capture.h"
#include "ServerTimerManager.h"
#include "TcpServer.h"
#include "TcpForward.h"
#include "Path.h"
//#include "Injection.h"
#include "ProcessUtils.h"
#include "SamplingProfiler.h"
#include "OrbitSession.h"
#include "Serialization.h"
#include "Pdb.h"
#include "Log.h"
#include "Params.h"
#include "EventTracer.h"
#include "OrbitUnreal.h"
//#include "CoreApp.h"
#include "Params.h"
#include "OrbitRule.h"
#include "VariableTracing.h"

#include <fstream>
#include <ostream>

using namespace std;

shared_ptr<Capture> GCapture;
vector< shared_ptr<SamplingProfiler> > GOldSamplingProfilers;

//-----------------------------------------------------------------------------
Capture::Capture()
{
    m_CapturePort = GParams.m_Port;
}

//-----------------------------------------------------------------------------
bool Capture::Inject( bool a_WaitForConnection )
{
    /*XXX
    Injection inject;
    wstring dllName = Path::GetDllPath( m_TargetProcess->GetIs64Bit() );

    GTcpServer->Disconnect();

    m_Injected = inject.Inject( dllName.c_str(), *m_TargetProcess, "OrbitInit", m_CaptureHost, m_CapturePort );
    if( m_Injected )
    {
        ORBIT_LOG( Format( "Injected in %s", m_TargetProcess->GetName().c_str() ) );
        m_InjectedProcessW = m_TargetProcess->GetName();
        m_InjectedProcess = ws2s(m_InjectedProcessW);
    }

    if( a_WaitForConnection )
    {
        int numTries = 50;
        while( !GTcpServer->HasConnection() && numTries-- > 0 )
        {
            ORBIT_LOG( Format( "Waiting for connection on port %i", m_CapturePort ) );
            Sleep(100);
        }

        m_Injected = m_Injected && GTcpServer->HasConnection();
    }

    return m_Injected;
    */
    return false;
}

//-----------------------------------------------------------------------------
bool Capture::InjectRemote()
{
    /*XXX
    Injection inject;
    wstring dllName = Path::GetDllPath( m_TargetProcess->GetIs64Bit() );
    GTcpServer->Disconnect();

    m_Injected = inject.Inject( dllName.c_str(), *m_TargetProcess, "OrbitInitRemote", m_CaptureHost, m_CapturePort );
    
    if( m_Injected )
    {
        ORBIT_LOG( Format( "Injected in %s", m_TargetProcess->GetName().c_str() ) );
        m_InjectedProcessW = m_TargetProcess->GetName();
        m_InjectedProcess = ws2s( m_InjectedProcessW );
    }

    return m_Injected;
    */
    return false;
}

//-----------------------------------------------------------------------------
void Capture::SetTargetProcess( const shared_ptr< Process > & a_Process )
{
    if(a_Process != m_TargetProcess)
	{
        if( !a_Process->GetIsRemote() )
        {
            // In the case of a remote process, 
            // connection is already active
            GTcpServer->Disconnect();
        }

        m_TargetProcess->LoadDebugInfo();
        m_SamplingProfiler = make_shared<SamplingProfiler>( a_Process );
        m_SelectedFunctionsMap.clear();
        m_SessionPresets = nullptr;
        //m_OrbitUnreal.Clear();
        m_TargetProcess->ClearWatchedVariables();
	}
}

//-----------------------------------------------------------------------------
bool Capture::Connect()
{
    if( !m_Injected )
    {
        Inject();
    }

    return m_Injected;
}

//-----------------------------------------------------------------------------
bool Capture::StartCapture()
{
    SCOPE_TIMER_LOG( L"Capture::StartCapture" );

    if( m_TargetProcess->GetName().empty() )
        return false;

    m_CaptureTimer.Start();
    m_CaptureTimePoint = chrono::system_clock::now();

    if( !IsRemote() )
    {
        if( !Connect() )
        {
            return false;
        }
    }

    m_Injected = true;
    ++Message::GSessionID;
    GTcpServer->Send( Msg_NewSession );
    GServerTimerManager->StartRecording();
    
    ClearCaptureData();
    SendFunctionHooks();

    if( IsTrackingEvents() )
    {
        m_EventTracer->Start();
    }

    /*if( m_SelectedFunctionsMap.size() > 0 )
    {
        m_CoreApp->SendToUiNow( L"startcapture" );
    }*/
    
    return true;
}

//-----------------------------------------------------------------------------
void Capture::StopCapture()
{
    if( IsTrackingEvents() )
    {
        m_EventTracer->Stop();
    }

    if (!m_Injected)
    {
        return;
    }

    GTcpServer->Send( Msg_StopCapture );
    GServerTimerManager->StopRecording();
}

//-----------------------------------------------------------------------------
void Capture::ToggleRecording()
{
    if( GServerTimerManager )
    {
        if( GServerTimerManager->m_IsRecording )
            StopCapture();
        else
            StartCapture();
    }
}

//-----------------------------------------------------------------------------
void Capture::ClearCaptureData()
{
    m_SelectedFunctionsMap.clear();
    m_FunctionCountMap.clear();
    m_ZoneNames.clear();
    m_SelectedTextBox = nullptr;
    m_SelectedThreadId = 0;
    m_NumProfileEvents = 0;
    GTcpServer->ResetStats();
    //m_OrbitUnreal.NewSession();
    m_HasSamples = false;
    m_HasContextSwitches = false;
}

//-----------------------------------------------------------------------------
MessageType GetMessageType( Function::OrbitType a_type )
{
    static map<Function::OrbitType, MessageType> typeMap;
    if( typeMap.size() == 0 )
    {
        typeMap[Function::NONE]                      = Msg_FunctionHook;
        typeMap[Function::ORBIT_TIMER_START]         = Msg_FunctionHookZoneStart;
        typeMap[Function::ORBIT_TIMER_STOP]          = Msg_FunctionHookZoneStop;
        typeMap[Function::ORBIT_LOG]                 = Msg_FunctionHook;
        typeMap[Function::ORBIT_OUTPUT_DEBUG_STRING] = Msg_FunctionHookOutputDebugString;
        typeMap[Function::UNREAL_ACTOR]              = Msg_FunctionHookUnrealActor;
        typeMap[Function::ALLOC]                     = Msg_FunctionHookAlloc;
        typeMap[Function::FREE]                      = Msg_FunctionHookFree;
        typeMap[Function::REALLOC]                   = Msg_FunctionHookRealloc;
        typeMap[Function::ORBIT_DATA]                = Msg_FunctionHookOrbitData;
    }

    assert(typeMap.size() == Function::OrbitType::NUM_TYPES );
    
    return typeMap[a_type];
}

//-----------------------------------------------------------------------------
void Capture::PreFunctionHooks()
{
    // Clear selected functions
    for( int i = 0; i < Function::NUM_TYPES; ++i )
    {
        m_SelectedAddressesByType[i].clear();
    }

    // Clear current argument tracking data
    GTcpServer->Send( Msg_ClearArgTracking );

    // Find OutputDebugStringA
    if( GParams.m_HookOutputDebugString )
    {
        if( DWORD64 outputAddr = m_TargetProcess->GetOutputDebugStringAddress() )
        {
            m_SelectedAddressesByType[Function::ORBIT_OUTPUT_DEBUG_STRING].push_back( outputAddr );
        }
    }

    // Find alloc/free functions
    m_TargetProcess->FindCoreFunctions();

    // Unreal
    //CheckForUnrealSupport();
}

//-----------------------------------------------------------------------------
void Capture::SendFunctionHooks()
{
    PreFunctionHooks();

    for( Function* func : m_TargetProcess->GetFunctions() )
    {
        if( func->IsSelected() || func->IsOrbitFunc() )
        {
            func->PreHook();
            DWORD64 address = func->GetVirtualAddress();
            m_SelectedAddressesByType[func->m_OrbitType].push_back(address);
            m_SelectedFunctionsMap[(ULONG64)address] = func;
            func->ResetStats();
            m_FunctionCountMap[address] = 0;
        }
    }

    m_VisibleFunctionsMap = m_SelectedFunctionsMap;

    // XXX: Originally configured from GlCanvas ctor
    if( m_ClearCaptureDataFunc )
    {
        m_ClearCaptureDataFunc();
    }

    GTcpServer->Send( Msg_StartCapture );

    // Unreal
    /*if( m_UnrealSupported )
    {
        const OrbitUnrealInfo & info = m_OrbitUnreal.GetUnrealInfo();
        GTcpServer->Send( Msg_OrbitUnrealInfo, info );
    }*/

    // Send argument tracking info
    SendDataTrackingInfo();

    // Send all hooks by type
    for( int i = 0; i < Function::NUM_TYPES; ++i )
    {
        vector<DWORD64> & addresses = m_SelectedAddressesByType[i];
        if( addresses.size() )
        {
            MessageType msgType = GetMessageType( (Function::OrbitType)i );
            GTcpServer->Send( msgType, addresses );
        }
    }
}

//-----------------------------------------------------------------------------
void Capture::SendDataTrackingInfo()
{
    // Send information about arguments we want to track
    std::unordered_map<DWORD64, std::shared_ptr<Rule> > rules;
    for( auto & pair : rules) //*m_CoreApp->GetRules() )
    {
        const shared_ptr<Rule> rule = pair.second;
        Function* func = rule->m_Function;
        Message msg( Msg_ArgTracking );
        ArgTrackingHeader& header = msg.m_Header.m_ArgTrackingHeader;
        ULONG64 address = (ULONG64)func->m_Pdb->GetHModule() + (ULONG64)func->m_Address;
        header.m_Function = address;
        header.m_NumArgs = (int)rule->m_TrackedVariables.size();

        // TODO: Argument tracking was hijacked by data tracking
        //       We should separate both concepts and revive argument
        //       tracking.
        vector<Argument> args;
        for( const shared_ptr<Variable> var : rule->m_TrackedVariables )
        {
            Argument arg;
            arg.m_Offset = (DWORD)var->m_Address;
            arg.m_NumBytes = var->m_Size;
            args.push_back(arg);
        }

        msg.m_Size = (int)args.size() * sizeof(Argument);
        
        GTcpServer->Send( msg, (void*)args.data() );
    }
}

//-----------------------------------------------------------------------------
void Capture::TestHooks()
{
    if( !m_IsTesting )
    {
        m_IsTesting = true;
        m_FunctionIndex = 0;
        m_TestTimer.Start();
    }
    else
    {
        m_IsTesting = false;
    }
}

//-----------------------------------------------------------------------------
void Capture::StartSampling()
{
    if( !m_IsSampling && IsTrackingEvents() && !m_TargetProcess->GetName().empty() )
    {
        SCOPE_TIMER_LOG( L"Capture::StartSampling" );

        m_CaptureTimer.Start();
        m_CaptureTimePoint = chrono::system_clock::now();

        ClearCaptureData();
        GServerTimerManager->StartRecording();
        m_EventTracer->Start();

        m_IsSampling = true;
    }
}

//-----------------------------------------------------------------------------
void Capture::StopSampling()
{
    if( m_IsSampling )
    {
        if( IsTrackingEvents() )
        {
            m_EventTracer->Stop();
        }

        GServerTimerManager->StopRecording();
    }
}

//-----------------------------------------------------------------------------
bool Capture::IsCapturing()
{
    return GServerTimerManager && GServerTimerManager->m_IsRecording;
}

//-----------------------------------------------------------------------------
void Capture::Update()
{
    if( m_IsSampling )
    {
        if( m_SamplingProfiler->ShouldStop() )
        {
            m_SamplingProfiler->StopCapture();
        }

        if( m_SamplingProfiler->GetState() == SamplingProfiler::DoneProcessing )
        {
            if( m_SamplingDoneCallback )
            {
                m_SamplingDoneCallback( m_SamplingProfiler );
            }
            m_IsSampling = false;
        }
    }

    if( GPdbDbg )
    {
        GPdbDbg->Update();
    }

    if( m_Injected && !GTcpServer->HasConnection() )
    {
        StopCapture();
        m_Injected = false;
    }

    m_HasSamples = m_EventTracer->GetEventBuffer().HasEvent();
}

//-----------------------------------------------------------------------------
void Capture::DisplayStats()
{
    if (m_SamplingProfiler)
    {
        TRACE_VAR(m_SamplingProfiler->GetNumSamples());
    }
}

//-----------------------------------------------------------------------------
void Capture::OpenCapture( const fs::path &  )
{
    LocalScopeTimer Timer( &m_OpenCaptureTime );
    SCOPE_TIMER_LOG( L"OpenCapture" );

    // TODO!
}

//-----------------------------------------------------------------------------
bool Capture::IsOtherInstanceRunning()
{
    /*XXX
    DWORD procID = 0;
    HANDLE procHandle = Injection::GetTargetProcessHandle( ORBIT_EXE_NAME, procID );
    PRINT_FUNC;
    bool otherInstanceFound = procHandle != NULL;
    PRINT_VAR( otherInstanceFound );
    return otherInstanceFound;
    */
    return false;
}

//-----------------------------------------------------------------------------
void Capture::LoadSession( const shared_ptr<Session> & a_Session )
{
    m_SessionPresets = a_Session;

    vector<fs::path> modulesToLoad;
    for( auto & it : a_Session->m_Modules )
    {
        SessionModule& module = it.second;
        ORBIT_LOG_DEBUG( module.m_Name );
        modulesToLoad.push_back( it.first );
    }

    if( m_LoadPdbAsync )
    {
        m_LoadPdbAsync( modulesToLoad );
    }
}

//-----------------------------------------------------------------------------
void Capture::SaveSession( const fs::path & a_FileName )
{
    Session session;
    session.m_ProcessFullPath = m_TargetProcess->GetFullName();
    
    for( Function* func : m_TargetProcess->GetFunctions() )
    {
        if( func->IsSelected() )
        {
            auto& mod = session.m_Modules[func->m_Pdb->GetName()];
            mod.m_FunctionHashes.push_back( func->Hash() );
        }
    }

    fs::path saveFileName = a_FileName;
    if( a_FileName.extension() != L".opr" )
    {
        saveFileName += L".opr";
    }

    SCOPE_TIMER_LOG( Format( L"Saving Orbit session in %s", saveFileName.c_str() ) );
    ofstream file( saveFileName, ios::binary );
    cereal::BinaryOutputArchive archive(file);
    //archive( cereal::make_nvp("Session", session) );
}

//-----------------------------------------------------------------------------
void Capture::NewSamplingProfiler()
{
    if( m_SamplingProfiler )
    {
        // To prevent destruction while processing data...
        GOldSamplingProfilers.push_back( m_SamplingProfiler );
    }

    m_SamplingProfiler = make_shared< SamplingProfiler >( m_TargetProcess, true );
}

//-----------------------------------------------------------------------------
bool Capture::IsTrackingEvents()
{
    static bool yieldEvents = false;
    if( yieldEvents && IsOtherInstanceRunning() && m_TargetProcess )
    {
        if( m_TargetProcess->GetName().filename() == L"Orbit.exe" )
        {
            return false;
        }
    }

    if( m_TargetProcess->GetIsRemote() && !GTcpServer->IsLocalConnection() )
    {
        return false;
    }
    
    return GParams.m_TrackContextSwitches || GParams.m_TrackSamplingEvents;
}

//-----------------------------------------------------------------------------
bool Capture::IsRemote()
{
    return m_TargetProcess && m_TargetProcess->GetIsRemote();
}

//-----------------------------------------------------------------------------
void Capture::RegisterZoneName( DWORD64 a_ID, char* a_Name )
{
    m_ZoneNames[a_ID] = a_Name;
}

//-----------------------------------------------------------------------------
void Capture::AddCallstack( CallStack & a_CallStack )
{
    ScopeLock lock( m_CallstackMutex );
    m_Callstacks[a_CallStack.m_Hash] = make_shared<CallStack>(a_CallStack);
}

//-----------------------------------------------------------------------------
shared_ptr<CallStack> Capture::GetCallstack( CallstackID a_ID )
{
    ScopeLock lock( m_CallstackMutex );
    
    auto it = m_Callstacks.find( a_ID );
    if( it != m_Callstacks.end() )
    {
        return it->second;
    }

    return nullptr;
}

//-----------------------------------------------------------------------------
/*void Capture::CheckForUnrealSupport()
{
    m_UnrealSupported = m_CoreApp->GetUnrealSupportEnabled() && m_OrbitUnreal.HasFnameInfo();
}*/

//-----------------------------------------------------------------------------
void Capture::PreSave()
{
    // Add selected functions' exact address to sampling profiler
    for( auto & pair : m_SelectedFunctionsMap )
    {
        m_SamplingProfiler->AddAddress( pair.first );
    }
}

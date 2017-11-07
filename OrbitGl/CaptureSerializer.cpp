
#include "CaptureSerializer.h"
#include "Serialization.h"
#include "ScopeTimer.h"

#include "Capture.h"
#include "TextBox.h"
#include "TimeGraph.h"
#include "Callstack.h"
#include "SamplingProfiler.h"
#include "Pdb.h"
#include "PrintVar.h"
#include "App.h"
#include "OrbitProcess.h"
#include "OrbitModule.h"

#include <fstream>
#include <memory>

using namespace std;

//-----------------------------------------------------------------------------
CaptureSerializer::CaptureSerializer()
{
    m_Version = 2;
    m_TimerVersion = Timer::Version;
    m_SizeOfTimer = sizeof(Timer);
}

//-----------------------------------------------------------------------------
void CaptureSerializer::Save( const fs::path& a_FileName )
{
    GCapture->PreSave();

    basic_ostream<char> Stream( &GStreamCounter );
    cereal::BinaryOutputArchive CountingArchive( Stream );
    GStreamCounter.Reset();
    Save( CountingArchive );
    PRINT_VAR( GStreamCounter.Size() );
    GStreamCounter.Reset();

    // Binary
    m_CaptureName = a_FileName;
    ofstream myfile( a_FileName, ios::binary );
    if( !myfile.fail() )
    {
        SCOPE_TIMER_LOG( Format( L"Saving capture in %s", a_FileName.c_str() ) );
        cereal::BinaryOutputArchive archive( myfile );
        Save( archive );
        myfile.close();
    }
}

//-----------------------------------------------------------------------------
template <class T> void CaptureSerializer::Save( T & a_Archive )
{
    m_NumTimers = m_TimeGraph->m_TextBoxes.size();

    // Header
    a_Archive( cereal::make_nvp( "Capture", *this ) );

    // Functions
    {
        ORBIT_SIZE_SCOPE( "Functions" );
        vector<Function> functions;
        for( auto & pair : GCapture->m_SelectedFunctionsMap )
        {
            Function * func = pair.second;
            if( func )
            {
                functions.push_back(*func);
                functions.back().m_Address = func->GetVirtualAddress();
            }
        }

        a_Archive(functions);
    }

    // Function Count
    a_Archive( GCapture->m_FunctionCountMap );

    // Process
    {
        ORBIT_SIZE_SCOPE( "GCapture->m_TargetProcess" );
        a_Archive( GCapture->m_TargetProcess );
    }

    // Callstacks
    {
        ORBIT_SIZE_SCOPE( "GCapture->m_Callstacks" );
        a_Archive( GCapture->m_Callstacks );
    }

    // Sampling profiler
    {
        ORBIT_SIZE_SCOPE( "SamplingProfiler" );
        a_Archive( GCapture->m_SamplingProfiler );
    }

    // Event buffer
    {
        ORBIT_SIZE_SCOPE( "Event Buffer" );
        a_Archive( GEventTracer.GetEventBuffer() );
    }

    // Timers
    int numWrites = 0;
    for( TextBox & box : m_TimeGraph->m_TextBoxes )
    {
        a_Archive( cereal::binary_data( (char*)&box.GetTimer(), sizeof( Timer ) ) );

        if( ++numWrites > m_NumTimers )
        {
            break;
        }
    }
}

//-----------------------------------------------------------------------------
void CaptureSerializer::Load( const fs::path& a_FileName )
{
    SCOPE_TIMER_LOG( Format( L"Loading capture %s", a_FileName.c_str() ) );

    // Binary
    ifstream file( a_FileName, ios::binary );
    if( !file.fail() )
    {
        // header
        cereal::BinaryInputArchive archive( file );
        archive( *this );

        // functions
        shared_ptr<Module> module = make_shared<Module>();
        GCapture->m_TargetProcess->AddModule(module);
        module->m_Pdb = make_shared<Pdb>( a_FileName.c_str() );
        archive( module->m_Pdb->GetFunctions() );
        module->m_Pdb->ProcessData();
        GPdbDbg = module->m_Pdb;
        GCapture->m_SelectedFunctionsMap.clear();
        for( Function & func : module->m_Pdb->GetFunctions() )
        {
            GCapture->m_SelectedFunctionsMap[func.m_Address] = &func;
        }
        GCapture->m_VisibleFunctionsMap = GCapture->m_SelectedFunctionsMap;

        // Function count
        archive( GCapture->m_FunctionCountMap );

        // Process
        //archive( GCapture->m_TargetProcess );

        // Callstacks
        archive( GCapture->m_Callstacks );

        // Sampling profiler
        archive( GCapture->m_SamplingProfiler );
        GCapture->m_SamplingProfiler->SortByThreadUsage();
        GOrbitApp->AddSamplingReport( GCapture->m_SamplingProfiler );
        GCapture->m_SamplingProfiler->SetLoadedFromFile( true );

        // Event buffer
        archive( GEventTracer.GetEventBuffer() );

        // Timers
        Timer timer;
        while( file.read( (char*)&timer, sizeof(Timer) ) )
        {
            m_TimeGraph->ProcessTimer( timer );
            m_TimeGraph->UpdateThreadDepth( timer.m_TID, timer.m_Depth );
        }

        GOrbitApp->FireRefreshCallbacks();
    }
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE( CaptureSerializer, 0 )
{
    //XXX:ORBIT_NVP_VAL( 0, m_CaptureName );
    ORBIT_NVP_VAL( 0, m_Version );
    ORBIT_NVP_VAL( 0, m_TimerVersion );
    ORBIT_NVP_VAL( 0, m_NumTimers );
    ORBIT_NVP_VAL( 0, m_SizeOfTimer );
}

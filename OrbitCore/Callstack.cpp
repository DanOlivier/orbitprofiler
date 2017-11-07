//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Callstack.h"
#include "PrintVar.h"
#include "PTM/OrbitProcess.h"
#include "Capture.h"
#include "OrbitType.h"
#include "Serialization.h"

using namespace std;

//-----------------------------------------------------------------------------
CallStack::CallStack( CallStackPOD a_CS ) : CallStack()
{
    m_Hash     = a_CS.m_Hash;
    m_ThreadId = a_CS.m_ThreadId;
    m_Depth    = a_CS.m_Depth;
    m_Data.resize( a_CS.m_Depth );
    memcpy( m_Data.data(), a_CS.m_Data, a_CS.m_Depth*sizeof(a_CS.m_Data[0]) );
}

//-----------------------------------------------------------------------------
void CallStack::Print()
{
    PRINT_VAR( m_Hash );
    PRINT_VAR( m_Depth );
    PRINT_VAR( m_ThreadId );

    for( int i = 0; i < m_Depth; ++i )
    {
        string address = ws2s( VAR_TO_STR( (void*) m_Data[i] ) );
        PRINT_VAR_INL( address );
    }
}

//-----------------------------------------------------------------------------
wstring CallStack::GetString()
{
    wstring callstackString;
    
    ScopeLock lock( GCapture->m_TargetProcess->GetDataMutex() );
    for( int i = 0; i < m_Depth; ++i )
    {
        DWORD64 addr = m_Data[i];
        Function* func = GCapture->m_TargetProcess->GetFunctionFromAddress( addr, false );

        if( func )
        {
            callstackString += func->PrettyName() + L"\n";
        }
        else
        {
            callstackString += Format( L"%I64x\n", addr );
        }
    }

    return callstackString;
}

//-----------------------------------------------------------------------------
StackFrame::StackFrame( HANDLE a_Thread )
{
    // http://www.codeproject.com/threads/StackWalker.asp

    STACKFRAME64 & s = m_StackFrame;
    CONTEXT & c = m_Context;

    memset( this, 0, sizeof( *this ) );
    c.ContextFlags = CONTEXT_FULL;

	GetThreadContext( a_Thread, &m_Context );

#ifdef _M_IX86
    // normally, call ImageNtHeader() and use machine info from PE header
    m_ImageType = IMAGE_FILE_MACHINE_I386;
    s.AddrPC.Offset = c.Eip;
    s.AddrPC.Mode = AddrModeFlat;
    s.AddrFrame.Offset = c.Ebp;
    s.AddrFrame.Mode = AddrModeFlat;
    s.AddrStack.Offset = c.Esp;
    s.AddrStack.Mode = AddrModeFlat;
#elif _M_X64
    m_ImageType = IMAGE_FILE_MACHINE_AMD64;
    s.AddrPC.Offset = c.Rip;
    s.AddrPC.Mode = AddrModeFlat;
    s.AddrFrame.Offset = c.Rsp;
    s.AddrFrame.Mode = AddrModeFlat;
    s.AddrStack.Offset = c.Rsp;
    s.AddrStack.Mode = AddrModeFlat;
#elif _M_IA64
    m_ImageType = IMAGE_FILE_MACHINE_IA64;
    s.AddrPC.Offset = c.StIIP;
    s.AddrPC.Mode = AddrModeFlat;
    s.AddrFrame.Offset = c.IntSp;
    s.AddrFrame.Mode = AddrModeFlat;
    s.AddrBStore.Offset = c.RsBSP;
    s.AddrBStore.Mode = AddrModeFlat;
    s.AddrStack.Offset = c.IntSp;
    s.AddrStack.Mode = AddrModeFlat;
#else
#error "Platform not supported!"
#endif
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE( CallStack, 0 )
{
    ORBIT_NVP_VAL( 0, m_Data );
    ORBIT_NVP_VAL( 0, m_Hash );
    ORBIT_NVP_VAL( 0, m_Depth );
    ORBIT_NVP_VAL( 0, m_ThreadId );
}

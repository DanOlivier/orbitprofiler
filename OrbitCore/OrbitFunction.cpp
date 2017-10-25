//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------


#include "OrbitFunction.h"
#include "OrbitModule.h"
#include "Serialization.h"
#include "Pdb.h"
#include "Capture.h"
#include "Log.h"
#include "Params.h"
#include "OrbitProcess.h"
#include "TcpServer.h"
#include "OrbitDia.h"
#include "SamplingProfiler.h"
#include "Utils.h"
#include "SymbolUtils.h"
//#include "DiaParser.h"

//#include <dia2.h>

using namespace std;

//-----------------------------------------------------------------------------
Function::~Function()
{

}

//-----------------------------------------------------------------------------
void Function::SetAsMainFrameFunction()
{
    Capture::GMainFrameFunction = (ULONG64)m_Pdb->GetHModule() + this->m_Address;
    m_Selected = true;
}

//-----------------------------------------------------------------------------
const wstring & Function::PrettyName()
{
    if( m_PrettyName.size() == 0 )
    {
        m_PrettyName = m_Name;
    }

    return m_PrettyName;
}

//-----------------------------------------------------------------------------
bool Function::Hookable()
{
    // Don't allow hooking in asm implemented functions (strcpy, stccat...) 
    // TODO: give this better thought.  Here is the theory:
    // Functions that loop back to first 5 bytes of instructions will explode as
    // the IP lands in the middle of the relative jump instruction...
    // Ideally, we would detect such a loop back and not allow hooking.
    if( m_File.find( L".asm" ) != wstring::npos )
    {
        return false;
    }

    CV_call_e conv = (CV_call_e)m_CallConv;
    return ( ( conv == CV_CALL_NEAR_C || CV_CALL_THISCALL ) && m_Size >= 5 )
        || ( GParams.m_AllowUnsafeHooking && m_Size == 0 );
}

//-----------------------------------------------------------------------------
void Function::PreHook()
{
    // Unreal
    if( Capture::GUnrealSupported )
    {
        Type* parent = GetParentType();
        if( parent && parent->IsA( L"UObject" ) )
        {
            m_OrbitType = Function::UNREAL_ACTOR;
        }
    }

    // MemFunc
    if( IsMemoryFunc() )
    {
        //if( this->IsMemberFunction() )
    }
}

//-----------------------------------------------------------------------------
DWORD64 Function::GetVirtualAddress() const
{
    return m_Address + ( m_Pdb ? (DWORD64)m_Pdb->GetHModule() : 0 );
}

//-----------------------------------------------------------------------------
wstring Function::GetModuleName()
{
    if( m_Pdb )
    {
        return m_Pdb->GetName();
    }
    else
    {
        shared_ptr<Module> module = Capture::GTargetProcess->GetModuleFromAddress( m_Address );
        return module ? module->m_Name : L"";
    }
}

//-----------------------------------------------------------------------------
Type * Function::GetParentType()
{
    return m_ParentId ? &m_Pdb->GetTypeFromId( m_ParentId ) : nullptr;
}

//-----------------------------------------------------------------------------
void Function::ResetStats()
{
    if( m_Stats == nullptr )
    {
        m_Stats = make_shared<FunctionStats>();
    }
    else
    {
        m_Stats->Reset();
    }
}

//-----------------------------------------------------------------------------
void Function::GetDisassembly()
{
    if( m_Pdb && Capture::Connect() )
    {
        Message msg( Msg_GetData );
        ULONG64 address = (ULONG64)m_Pdb->GetHModule() + (ULONG64)m_Address;
        msg.m_Header.m_DataTransferHeader.m_Address = address;
        msg.m_Header.m_DataTransferHeader.m_Type = DataTransferHeader::Code;
        DWORD64 size = m_Size;

        // dll
        if( m_Size == 0 )
        {
            shared_ptr<Module> module = Capture::GTargetProcess->GetModuleFromAddress( address );
            if( module )
            {
                DWORD64 maxSize = module->m_AddressEnd - address;
                size = min( GParams.m_NumBytesAssembly, maxSize );
            }
        }

        msg.m_Size = (int)size;
        GTcpServer->Send( msg );
    }
}

//-----------------------------------------------------------------------------
const char* Function::GetCallingConventionString( int a_CallConv )
{
    const char* CallingConv[] = {
        "NEAR_C",      //CV_CALL_NEAR_C      = 0x00, // near right to left push, caller pops stack
        "FAR_C",       //CV_CALL_FAR_C       = 0x01, // far right to left push, caller pops stack
        "NEAR_PASCAL", //CV_CALL_NEAR_PASCAL = 0x02, // near left to right push, callee pops stack
        "FAR_PASCAL",  //CV_CALL_FAR_PASCAL  = 0x03, // far left to right push, callee pops stack
        "NEAR_FAST",   //CV_CALL_NEAR_FAST   = 0x04, // near left to right push with regs, callee pops stack
        "FAR_FAST",    //CV_CALL_FAR_FAST    = 0x05, // far left to right push with regs, callee pops stack
        "SKIPPED",     //CV_CALL_SKIPPED     = 0x06, // skipped (unused) call index
        "NEAR_STD",    //CV_CALL_NEAR_STD    = 0x07, // near standard call
        "FAR_STD",     //CV_CALL_FAR_STD     = 0x08, // far standard call
        "NEAR_SYS",    //CV_CALL_NEAR_SYS    = 0x09, // near sys call
        "FAR_SYS",     //CV_CALL_FAR_SYS     = 0x0a, // far sys call
        "THISCALL",    //CV_CALL_THISCALL    = 0x0b, // this call (this passed in register)
        "MIPSCALL",    //CV_CALL_MIPSCALL    = 0x0c, // Mips call
        "GENERIC",     //CV_CALL_GENERIC     = 0x0d, // Generic call sequence
        "ALPHACALL",   //CV_CALL_ALPHACALL   = 0x0e, // Alpha call
        "PPCCALL",     //CV_CALL_PPCCALL     = 0x0f, // PPC call
        "SHCALL",      //CV_CALL_SHCALL      = 0x10, // Hitachi SuperH call
        "ARMCALL",     //CV_CALL_ARMCALL     = 0x11, // ARM call
        "AM33CALL",    //CV_CALL_AM33CALL    = 0x12, // AM33 call
        "TRICALL",     //CV_CALL_TRICALL     = 0x13, // TriCore Call
        "SH5CALL",     //CV_CALL_SH5CALL     = 0x14, // Hitachi SuperH-5 call
        "M32RCALL",    //CV_CALL_M32RCALL    = 0x15, // M32R Call
        "CLRCALL",     //CV_CALL_CLRCALL     = 0x16, // clr call
        "INLINE",      //CV_CALL_INLINE      = 0x17, // Marker for routines always inlined and thus lacking a convention
        "NEAR_VECTOR", //CV_CALL_NEAR_VECTOR = 0x18, // near left to right push with regs, callee pops stack
        "RESERVED" };  //CV_CALL_RESERVED    = 0x19  // first unused call enumeration

    return a_CallConv >= 0 ? CallingConv[a_CallConv] : "UnknownCallConv";
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE( Function, 1 )
{
    ORBIT_NVP_VAL( 0, m_Name );
    ORBIT_NVP_VAL( 0, m_PrettyName );
    ORBIT_NVP_VAL( 0, m_Address );
    ORBIT_NVP_VAL( 0, m_Size );
    ORBIT_NVP_VAL( 0, m_Module );
    ORBIT_NVP_VAL( 0, m_File );
    ORBIT_NVP_VAL( 0, m_Line );
    ORBIT_NVP_VAL( 0, m_ModBase );
    ORBIT_NVP_VAL( 0, m_CallConv );
    ORBIT_NVP_VAL( 1, m_Stats );
}

//-----------------------------------------------------------------------------
//Argument::Register Argument::GetArgRegister( int a_ArgIndex, bool isFloat )
//{
//    const Register IntRegs[] = { RCX, RDX, R8, R9 };
//    const Register FltRegs[] = { XMM0, XMM1, XMM2, XMM3 };
//
//    assert( a_ArgIndex <= 4 );
//
//    return !isFloat ? IntRegs[a_ArgIndex] : FltRegs[a_ArgIndex]; 
//}

//-----------------------------------------------------------------------------
bool FunctionParam::InRegister( int a_Index )
{
    return a_Index < 4;
}

//-----------------------------------------------------------------------------
bool FunctionParam::IsFloat()
{
    return ( m_Type.find( L"float" ) != wstring::npos ||
        m_Type.find( L"double" ) != wstring::npos );
}

//-----------------------------------------------------------------------------
void Function::ProcessArgumentInfo()
{
    m_ArgInfo.clear();
    m_ArgInfo.reserve( m_Params.size() );

    int argIndex = IsMemberFunction() ? 1 : 0;

    for( FunctionParam & param : m_Params )
    {
        Argument arg;
        arg.m_Index = argIndex;
        arg.m_Reg = (CV_HREG_e)param.m_SymbolInfo.Register;
        arg.m_Offset = (DWORD)param.m_SymbolInfo.Address;
        arg.m_NumBytes = param.m_SymbolInfo.Size;
        m_ArgInfo.push_back( arg );

        ++argIndex;
    }
}

//-----------------------------------------------------------------------------
bool Function::IsMemberFunction()
{
    // TODO
    return false;
}

//-----------------------------------------------------------------------------
void Function::Print()
{
    if( !m_Pdb )
    {
        return;
    }

    std::unique_ptr<llvm::pdb::PDBSymbol> diaSymbol = m_Pdb->GetDiaSymbolFromId( this->m_Id );
    if( diaSymbol )
    {
        OrbitDia::DiaDump( diaSymbol );

        if( m_ParentId )
        {
            Type & type = m_Pdb->GetTypeFromId( m_ParentId );
            type.LoadDiaInfo();
            type.GenerateDiaHierarchy();
        }

        DiaParser parser;
        parser.PrintFunctionType( diaSymbol );
        ORBIT_VIZ( parser.m_Log );
    }

    LineInfo lineInfo;
    SymUtils::GetLineInfo( m_Address + (DWORD64)m_Pdb->GetHModule(), lineInfo );
    ORBIT_VIZV( lineInfo.m_File );
    ORBIT_VIZV( lineInfo.m_Line );
    ORBIT_VIZV( lineInfo.m_Address );

    ORBIT_VIZV( m_Address );
    ORBIT_VIZV( m_Selected );

    if( m_Params.size() )
    {
        ORBIT_VIZ( "\nParams:" );
        for( auto & var : m_Params )
        {
            ORBIT_VIZV( var.m_Name );
            ORBIT_VIZV( var.m_Address );
            ORBIT_VIZV( var.m_ParamType );
            ORBIT_VIZV( var.m_Type );
        }
    }
}

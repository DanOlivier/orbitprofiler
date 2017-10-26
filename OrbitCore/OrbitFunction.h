//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "OrbitDbgHelp.h"
#include "cvconst.h"
#include "BaseTypes.h"
#include "FunctionStats.h"
#include "SerializationMacros.h"
#include "Utils.h"
#include "FunctionArgs.h"

#include <string>
#include <winpr/tchar.h>
#include <memory>

class Pdb;

//-----------------------------------------------------------------------------
struct FunctionParam
{
    FunctionParam(){ std::memset( this, 0, sizeof( FunctionParam ) ); }
    std::wstring     m_Name;
    std::wstring     m_ParamType;
    std::wstring     m_Type;
    std::wstring     m_Address;
    //SYMBOL_INFO m_SymbolInfo;

    bool InRegister( int a_Index );
    bool IsPointer() { return m_Type.find( L"*" ) != std::wstring::npos; }
    bool IsRef() { return m_Type.find( L"&" ) != std::wstring::npos; }
    bool IsFloat();
};

//-----------------------------------------------------------------------------
class Function
{
public:
    Function() : m_Address( 0 )
               , m_Size( 0 )
               , m_Line( 0 )
               , m_ModBase( 0 )
               , m_Selected( 0 )
               , m_CallConv( -1 )
               , m_Id( 0 )
               , m_ParentId( 0 )
               , m_Pdb( nullptr )
               , m_NameHash( 0 )
               , m_OrbitType( OrbitType::NONE )
    {}

    ~Function();

    void Print();
    void SetAsMainFrameFunction();
    void AddParameter( const FunctionParam & a_Param ){ m_Params.push_back( a_Param ); }
    const std::wstring & PrettyName();
    inline const std::string & PrettyNameStr() { if( m_PrettyNameStr.size() == 0 ) m_PrettyNameStr = ws2s( m_PrettyName ); return m_PrettyNameStr; }
    inline const std::wstring & Lower() { if( m_PrettyNameLower.size() == 0 ) m_PrettyNameLower = ToLower( m_PrettyName ); return m_PrettyNameLower; }
    static const wchar_t* GetCallingConventionString( int a_CallConv );
    void ProcessArgumentInfo();
    bool IsMemberFunction();
    unsigned long long Hash() { if( m_NameHash == 0 ) { m_NameHash = StringHash( m_PrettyName ); } return m_NameHash; }
    bool Hookable();
    void Select(){ if( Hookable() ) m_Selected = true; }
    void PreHook();
    void UnSelect(){ m_Selected = false; }
    void ToggleSelect() { /*if( Hookable() )*/ m_Selected = !m_Selected; }
    bool IsSelected() const { return m_Selected; }
    DWORD64 GetVirtualAddress() const;
    bool IsOrbitFunc() { return m_OrbitType != OrbitType::NONE; }
    bool IsOrbitZone() { return m_OrbitType == ORBIT_TIMER_START || m_OrbitType == ORBIT_TIMER_STOP; }
    bool IsOrbitStart(){ return m_OrbitType == ORBIT_TIMER_START; }
    bool IsOrbitStop() { return m_OrbitType == ORBIT_TIMER_STOP; }
    bool IsRealloc()   { return m_OrbitType == REALLOC; }
    bool IsAlloc()     { return m_OrbitType == ALLOC; }
    bool IsFree()      { return m_OrbitType == FREE; }
    bool IsMemoryFunc(){ return IsFree() || IsAlloc() || IsRealloc(); }
    std::wstring GetModuleName();
    class Type* GetParentType();
    void ResetStats();
    void GetDisassembly();

    enum MemberID
    {
        NAME,
        ADDRESS,
        MODULE,
        FILE,
        LINE,
        SELECTED,
        INDEX,
        SIZE,
        CALL_CONV,
        NUM_EXPOSED_MEMBERS
    };

    enum OrbitType
    {
        NONE,
        ORBIT_TIMER_START,
        ORBIT_TIMER_STOP,
        ORBIT_LOG,
        ORBIT_OUTPUT_DEBUG_STRING,
        UNREAL_ACTOR,
        ALLOC,
        FREE,
        REALLOC,
        ORBIT_DATA,
        NUM_TYPES
    };

    ORBIT_SERIALIZABLE;

public:
    std::wstring  m_Name;
    std::wstring  m_PrettyName;
    std::string   m_PrettyNameStr;
    std::wstring  m_PrettyNameLower;
    DWORD64  m_Address;
    ULONG    m_Size;
    std::wstring  m_Module;
    std::wstring  m_File;
    int      m_Line;
    ULONG64  m_ModBase;
    int      m_CallConv;
    ULONG    m_Id;
    DWORD    m_ParentId;
    std::vector<FunctionParam> m_Params;
    std::vector<Argument>      m_ArgInfo;
    Pdb*                  m_Pdb;
    unsigned long long    m_NameHash;
    OrbitType             m_OrbitType;
    std::shared_ptr<FunctionStats> m_Stats;

protected:
    bool     m_Selected;
};
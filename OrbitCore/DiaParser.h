//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>
#include "BaseTypes.h"

#include <llvm/DebugInfo/PDB/PDBTypes.h>

//-----------------------------------------------------------------------------
struct DiaParser
{
    static std::wstring GetSymTag( DWORD dwSymTag );
    static std::wstring GetLocation( std::unique_ptr<llvm::pdb::PDBSymbol> );
    static std::wstring GetSymbolType( std::unique_ptr<llvm::pdb::PDBSymbol> );
    static std::wstring GetName( std::unique_ptr<llvm::pdb::PDBSymbol> );

    std::wstring GetBasicType(DWORD a_BaseType);
    ULONGLONG GetSize( std::unique_ptr<llvm::pdb::PDBSymbol> );
    DWORD GetSymbolID( std::unique_ptr<llvm::pdb::PDBSymbol> );
    DWORD GetTypeID( std::unique_ptr<llvm::pdb::PDBSymbol> );
    void GetData( std::unique_ptr<llvm::pdb::PDBSymbol>, class Type* );
    std::wstring GetData( std::unique_ptr<llvm::pdb::PDBSymbol> );

    void PrintPublicSymbol( std::unique_ptr<llvm::pdb::PDBSymbol> );
    void PrintGlobalSymbol( std::unique_ptr<llvm::pdb::PDBSymbol> );
    void OrbitAddGlobalSymbol( std::unique_ptr<llvm::pdb::PDBSymbol> );
    void PrintSymbol( std::unique_ptr<llvm::pdb::PDBSymbol>, DWORD );
    void PrintSymTag( DWORD );
    void PrintName( std::unique_ptr<llvm::pdb::PDBSymbol> );
    void PrintUndName( std::unique_ptr<llvm::pdb::PDBSymbol> );
    void PrintThunk( std::unique_ptr<llvm::pdb::PDBSymbol> );
    void PrintCompilandDetails( std::unique_ptr<llvm::pdb::PDBSymbol> );
    void PrintCompilandEnv( std::unique_ptr<llvm::pdb::PDBSymbol> );
    void PrintLocation( std::unique_ptr<llvm::pdb::PDBSymbol> );
    void PrintConst( std::unique_ptr<llvm::pdb::PDBSymbol> );
    void PrintUDT( std::unique_ptr<llvm::pdb::PDBSymbol> );
    void PrintSymbolType( std::unique_ptr<llvm::pdb::PDBSymbol> );
    void PrintSymbolTypeNoPrefix( std::unique_ptr<llvm::pdb::PDBSymbol> );
    void PrintType( std::unique_ptr<llvm::pdb::PDBSymbol> );
    void PrintBound( std::unique_ptr<llvm::pdb::PDBSymbol> );
    void PrintData( std::unique_ptr<llvm::pdb::PDBSymbol> );
    void PrintUdtKind( std::unique_ptr<llvm::pdb::PDBSymbol> );
    void PrintTypeInDetail( std::unique_ptr<llvm::pdb::PDBSymbol>, DWORD );
    void PrintFunctionType( std::unique_ptr<llvm::pdb::PDBSymbol> );
    void PrintSourceFile( std::unique_ptr<llvm::pdb::IPDBSourceFile> );
    void PrintLines( std::unique_ptr<llvm::pdb::IPDBSession>, std::unique_ptr<llvm::pdb::PDBSymbol> );
    void PrintLines( std::unique_ptr<llvm::pdb::IPDBEnumLineNumbers> );
    void PrintSource( std::unique_ptr<llvm::pdb::IPDBSourceFile> );
    void PrintSecContribs( std::unique_ptr<llvm::pdb::SectionContrib> );
    //void PrintStreamData( std::unique_ptr<IDiaEnumDebugStreamData> );
    //void PrintFrameData( std::unique_ptr<IDiaFrameData> );
    //void PrintPropertyStorage( std::unique_ptr<IDiaPropertyStorage> );
    void PrintCallSiteInfo( std::unique_ptr<llvm::pdb::PDBSymbol> );
    void PrintHeapAllocSite( std::unique_ptr<llvm::pdb::PDBSymbol> );
    void PrintCoffGroup( std::unique_ptr<llvm::pdb::PDBSymbol> );
    void TypeLogSymTag( DWORD dwSymTag );
    void PrintNameTypeLog( std::unique_ptr<llvm::pdb::PDBSymbol> );

    void PrintClassHierarchy( std::unique_ptr<llvm::pdb::PDBSymbol>, DWORD, std::unique_ptr<llvm::pdb::PDBSymbol> a_Parent = nullptr );

    void GetTypeInformation( class Type* a_Type, DWORD a_TagType );
    void GetTypeInformation( class Type* a_Type, std::unique_ptr<llvm::pdb::PDBSymbol> pSymbol, DWORD a_TagType, DWORD dwIndent );

    /*template<typename... Args>
    inline void LOGF( _In_z_ _Printf_format_string_ const wchar_t* const _Format, Args&&... args )
    {
        m_Log += Format( _Format, std::forward<Args>( args )... );
    }*/

    std::wstring m_Log;
};
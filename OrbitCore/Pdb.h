//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "OrbitDbgHelp.h"
#include "OrbitType.h"
#include "Variable.h"

#include <vector>
#include <functional>
#include <thread>
#include <atomic>
#include <experimental/filesystem>

namespace llvm { 
    namespace pdb {
        class PDBSymbol;
        class IPDBSession;
    }
    namespace codeview {
        struct GUID;
    }
}

namespace fs = std::experimental::filesystem;    

class Pdb
{
public:
    Pdb( const fs::path& a_PdbName );
    ~Pdb();

    void Init();
    
    virtual bool LoadPdb( const fs::path& a_PdbName );
    virtual void LoadPdbAsync( const fs::path& a_PdbName, std::function<void()> a_CompletionCallback );

    bool LoadDataFromPdb();
    bool LoadPdbDia();
    void Update();
    void AddFunction( Function & a_Function );
    void CheckOrbitFunction( Function & a_Function );
    void AddType( const Type & a_Type );
    void AddGlobal( const Variable & a_Global );
    void PrintFunction( Function & a_Func );
    void OnReceiveMessage( const Message & a_Msg );
    void AddArgumentRegister( const std::string & a_Reg, const std::string & a_Function );

    const fs::path& GetName() const             { return m_Name; }
    const fs::path& GetFileName() const         { return m_FileName; }
    std::vector<Function>&    GetFunctions()              { return m_Functions; }
    std::vector<Type>&        GetTypes()                  { return m_Types; }
    std::vector<Variable>&    GetGlobals()                { return m_Globals; }
    HMODULE              GetHModule()                { return m_MainModule; }
    Type &               GetTypeFromId( ULONG a_Id ) { return m_TypeMap[a_Id]; }
    Type*                GetTypePtrFromId( ULONG a_ID );
    
    llvm::codeview::GUID GetGuid();

    
    void SetMainModule( HMODULE a_Module ){ m_MainModule = a_Module; }

    void Print() const;
    void PrintGlobals() const;
    void PopulateFunctionMap();
    void PopulateStringFunctionMap();
    void Clear();
    void Reserve();
    void ApplyPresets();

    Function* GetFunctionFromExactAddress( DWORD64 a_Address );
    Function* GetFunctionFromProgramCounter( DWORD64 a_Address );
    std::unique_ptr<llvm::pdb::PDBSymbol> SymbolFromAddress( DWORD64 a_Address );
    bool LineInfoFromAddress( DWORD64 a_Address, struct LineInfo & o_LineInfo );

    void SetLoadTime( float a_LoadTime ) { m_LastLoadTime = a_LoadTime; }
    float GetLoadTime() { return m_LastLoadTime; }

    std::wstring GetCachedName();
    std::wstring GetCachedKey();
    bool Load( const fs::path& a_CachedPdb );
    void Save();

    bool IsLoading() const { return m_IsLoading; }

    template <class Archive>
    void serialize( Archive & ar, std::uint32_t const version )
    {
        /*ar( CEREAL_NVP(m_Name)
          , CEREAL_NVP(m_FileName)
          , CEREAL_NVP(m_FileNameW)
          , CEREAL_NVP(m_Functions)
          , CEREAL_NVP(m_Types)
          , CEREAL_NVP(m_Globals)
          , CEREAL_NVP(m_ModuleInfo)
          , CEREAL_NVP(m_TypeMap) );*/
    }

    std::unique_ptr<llvm::pdb::PDBSymbol> GetDiaSymbolFromId( ULONG a_Id );
    void ProcessData();
protected:
    void SendStatusToUi();

protected:
    // State
    std::unique_ptr<std::thread>        m_LoadingThread;
    std::atomic<bool>                   m_FinishedLoading = {false};
    std::atomic<bool>                   m_IsLoading = {false};
    std::atomic<bool>                   m_IsPopulatingFunctionMap = {false};
    std::atomic<bool>                   m_IsPopulatingFunctionStringMap = {false};
    std::function<void()>               m_LoadingCompleteCallback;
    HMODULE                             m_MainModule = 0;
    float                               m_LastLoadTime = 0.0;
    bool                                m_LoadedFromCache = false;
    //std::vector<Variable>               m_WatchedVariables;
    std::set<std::string>               m_ArgumentRegisters;
    std::map<std::string, std::vector<std::string> >  m_RegFunctionsMap;

    // Data
    fs::path                            m_Name;
    fs::path                            m_FileName;
    std::vector<Function>               m_Functions;
    std::vector<Type>                   m_Types;
    std::vector<Variable>               m_Globals;
    //IMAGEHLP_MODULE64                   m_ModuleInfo;
    std::unordered_map<ULONG, Type>     m_TypeMap;
    std::map<DWORD64, Function*>        m_FunctionMap;
    std::unordered_map<unsigned long long, Function*> m_StringFunctionMap;
    Timer*                              m_LoadTimer;
    
    // DIA
    std::unique_ptr<llvm::pdb::IPDBSession> m_DiaSession;
    std::unique_ptr<llvm::pdb::PDBSymbol> m_DiaGlobalSymbol;
};

extern std::shared_ptr<Pdb> GPdbDbg;


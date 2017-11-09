#pragma once

//#include "PTM/OrbitProcess.h"

class Function;
class Type;
class Variable;
class Process;

namespace llvm { namespace pdb {
    class PDBSymbol;
    class IPDBSession;
}}

class ProcessTransients
{
    //void LoadDebugInfo();
    //void FindPdbs( const std::vector<fs::path> & a_SearchLocations );

    //Function* GetFunctionFromAddress( DWORD64 a_Address, bool a_IsExact = true );
    //std::unique_ptr<llvm::pdb::PDBSymbol> SymbolFromAddress( DWORD64 a_Address );
    //bool LineInfoFromAddress( DWORD64 a_Address, struct LineInfo & o_LineInfo );

    //std::vector<Function*>& GetFunctions() { return m_Functions; }
    //std::vector<Type*>&     GetTypes()     { return m_Types; }
    //std::vector<Variable*>& GetGlobals()   { return m_Globals; }

    //void AddWatchedVariable( std::shared_ptr<Variable> a_Variable ) { m_WatchedVariables.push_back( a_Variable ); }
    //const std::vector< std::shared_ptr<Variable> > & GetWatchedVariables(){ return m_WatchedVariables; }
    //void RefreshWatchedVariables();
    //void ClearWatchedVariables();

    //void AddType( Type & a_Type );

    //Mutex& GetDataMutex() { return m_DataMutex; }

    //DWORD64 GetOutputDebugStringAddress();
    //DWORD64 GetRaiseExceptionAddress();
    //void FindCoreFunctions();

protected:
    //void ClearTransients();

private:
    //Mutex       m_DataMutex;

    // Transients
    //std::vector< Function* >    m_Functions;
    //std::vector< Type* >        m_Types;
    //std::vector< Variable* >    m_Globals;
    //std::vector< std::shared_ptr<Variable> > m_WatchedVariables;
    
    //std::unordered_set< unsigned long long > m_UniqueTypeHash;
};

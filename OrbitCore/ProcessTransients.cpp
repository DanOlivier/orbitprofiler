#include "PTM/OrbitProcess.h"
#include "Injection.h"

//-----------------------------------------------------------------------------
void Process::LoadDebugInfo()
{
    EnumerateThreads();
}

//-----------------------------------------------------------------------------
void Process::ClearTransients()
{
    m_Functions.clear();
    m_Types.clear();
    m_Globals.clear();
    //m_WatchedVariables.clear();
    m_NameToModuleMap.clear();
}

//-----------------------------------------------------------------------------
Function* Process::GetFunctionFromAddress( DWORD64 a_Address, bool a_IsExact )
{
    DWORD64 address = (DWORD64)a_Address;
    auto it = m_Modules.upper_bound( address );
    if( !m_Modules.empty() && it != m_Modules.begin() )
    {
        --it;
        shared_ptr<Module> & module = it->second;
        if( address < module->m_AddressEnd )
        {
            if( module->m_Pdb != nullptr )
            {
                if( a_IsExact )
                {
                    return module->m_Pdb->GetFunctionFromExactAddress( a_Address );
                }
                else
                {
                    return module->m_Pdb->GetFunctionFromProgramCounter( a_Address );
                }
            }
        }
    }

    return nullptr;
}

//-----------------------------------------------------------------------------
std::unique_ptr<llvm::pdb::PDBSymbol> Process::SymbolFromAddress( DWORD64 a_Address )
{
    shared_ptr<Module> module = GetModuleFromAddress( a_Address );
    if( module && module->m_Pdb )
    {
        return module->m_Pdb->SymbolFromAddress( a_Address );
    }
    return nullptr;
}

//-----------------------------------------------------------------------------
bool Process::LineInfoFromAddress( DWORD64 a_Address, LineInfo & o_LineInfo )
{
    shared_ptr<Module> module = GetModuleFromAddress( a_Address );
    if( module && module->m_Pdb )
    {
        return module->m_Pdb->LineInfoFromAddress( a_Address, o_LineInfo );
    }

    return false;
}

//-----------------------------------------------------------------------------
void Process::RefreshWatchedVariables()
{
    for( shared_ptr<Variable> var : m_WatchedVariables )
    {
        var->SyncValue();
    }
}

//-----------------------------------------------------------------------------
void Process::ClearWatchedVariables()
{
    m_WatchedVariables.clear();
}

//-----------------------------------------------------------------------------
void Process::AddType(Type & a_Type)
{
    bool isPtr = a_Type.m_Name.find(L"Pointer to") != string::npos;
    if (!isPtr)
    {
        unsigned long long typeHash = a_Type.Hash();
        auto it = m_UniqueTypeHash.insert(typeHash);
        if (it.second == true)
        {
            m_Types.push_back(&a_Type);
        }
    }
}

//-----------------------------------------------------------------------------
void Process::FindPdbs( const vector<fs::path>& a_SearchLocations )
{
    unordered_map< wstring, vector<fs::path> > nameToPaths;

    // Populate list of all available pdb files
    for( const fs::path& dir : a_SearchLocations )
    {
        vector<fs::path> pdbFiles = Path::ListFiles( dir, L".pdb" );
        for( const fs::path& pdb : pdbFiles )
        {
            wstring pdbLower = ToLower( pdb.filename().wstring() );
            nameToPaths[pdbLower].push_back( pdb );
        }
    }

    // Find matching pdb
    for( auto & modulePair : m_Modules )
    {
        shared_ptr<Module> module =  modulePair.second;

        if( !module->m_FoundPdb )
        {
            fs::path moduleName = module->m_Name;
            wstring pdbName = Path::StripExtension( moduleName ).wstring() + L".pdb";

            const vector<fs::path>& pdbs = nameToPaths[pdbName];

            for( const fs::path& pdb : pdbs )
            {
                module->m_PdbName = pdb;
                module->m_FoundPdb = true;
                module->LoadDebugInfo();

                wstring signature = s2ws( GuidToString( module->m_Pdb->GetGuid() ) );

                if( Contains( module->m_DebugSignature, signature ) )
                {
                    // Found matching pdb
                    module->m_PdbSize = fs::file_size( module->m_PdbName );
                    break;
                }
                else
                {
                    module->m_FoundPdb = false;
                }
            }
        }       
    }
}

//-----------------------------------------------------------------------------
DWORD64 Process::GetOutputDebugStringAddress()
{
    auto it = m_NameToModuleMap.find( L"kernelbase.dll" );
    if( it != m_NameToModuleMap.end() )
    {
        shared_ptr<Module> module = it->second;
        auto remoteAddr = Injection::GetRemoteProcAddress( GetHandle(), 
            module->m_ModuleHandle, "OutputDebugStringA" );
        return (DWORD64)remoteAddr;
    }
    return 0;
}

//-----------------------------------------------------------------------------
DWORD64 Process::GetRaiseExceptionAddress()
{
    auto it = m_NameToModuleMap.find( L"kernelbase.dll" );
    if (it != m_NameToModuleMap.end())
    {
        shared_ptr<Module> module = it->second;
        auto remoteAddr = Injection::GetRemoteProcAddress(GetHandle(), 
            module->m_ModuleHandle, "RaiseException");
        return (DWORD64)remoteAddr;
    }

    return 0;
}

//-----------------------------------------------------------------------------
void Process::FindCoreFunctions()
{
    return; // XXX: Unreachable code
    SCOPE_TIMER_LOG(L"FindCoreFunctions");

    const auto prio = oqpi::task_priority::normal;
    oqpi_tk::scheduler().workersCount( prio );
    //int numWorkers = oqpi::thread::hardware_concurrency();

    oqpi_tk::parallel_for( "FindAllocFreeFunctions", (int32_t)m_Functions.size(), [&]( int32_t, int32_t a_ElementIndex )
    {
        Function* func = m_Functions[a_ElementIndex];
        const wstring & name = func->Lower();

        if( Contains( name, L"operator new" ) || Contains( name, L"FMallocBinned::Malloc" ) )
        {
            func->Select();
            func->m_OrbitType = Function::ALLOC;
        }
        else if( Contains( name, L"operator delete" ) || name == L"FMallocBinned::Free" )
        {
            func->Select();
            func->m_OrbitType = Function::FREE;
        } 
        else if( Contains( name, L"realloc" ) )
        {
            func->Select();
            func->m_OrbitType = Function::REALLOC;
        }
    } );
}


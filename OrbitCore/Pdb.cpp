//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------


#include "Pdb.h"
#include <algorithm>
#include <functional>
#include <fstream>
#include <ppl.h>
#include <tchar.h>

#include "SymbolUtils.h"

#include "Params.h"
#include "Utils.h"
#include "Tcp.h"
#include "TcpServer.h"
#include "Log.h"
#include "Capture.h"
#include "Serialization.h"
#include "OrbitSession.h"
#include "OrbitType.h"
#include "OrbitUnreal.h"
#include "DiaManager.h"
#include "Path.h"

#include "external/DIA2Dump/dia2dump.h"
#include "external/DIA2Dump/PrintSymbol.h"

using namespace std;

shared_ptr<Pdb> GPdbDbg;

//-----------------------------------------------------------------------------
Pdb::Pdb( const wchar_t* a_PdbName ) : m_FileName( a_PdbName )
                                     , m_MainModule(0)
                                     , m_LastLoadTime(0)
                                     , m_LoadedFromCache(false)
                                     , m_FinishedLoading(false)
                                     , m_IsLoading(false)
                                     , m_IsPopulatingFunctionMap(false)
                                     , m_IsPopulatingFunctionStringMap(false)
                                     , m_DiaSession(nullptr)
                                     , m_DiaGlobalSymbol(nullptr)
{
    m_Name = Path::GetFileName( m_FileName );
    memset( &m_ModuleInfo, 0, sizeof(IMAGEHLP_MODULE64) );
    m_ModuleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);
    m_LoadTimer = new Timer();
}

//-----------------------------------------------------------------------------
Pdb::~Pdb()
{
    if( m_DiaSession )
    {
        m_DiaSession->Release();
    }

    if( m_DiaGlobalSymbol )
    {
        m_DiaGlobalSymbol->Release();
    }
}

//-----------------------------------------------------------------------------
void Pdb::AddFunction( Function & a_Function )
{
    if( Contains( a_Function.m_Name, L"OutputDebugString" ) )
    {
        PRINT_VAR( a_Function.m_Address );
        PRINT_VAR( this->m_MainModule );
    }
    CheckOrbitFunction( a_Function );
    m_Functions.emplace_back( a_Function );
}

//-----------------------------------------------------------------------------
void Pdb::CheckOrbitFunction( Function & a_Function )
{
    const wstring & name = a_Function.m_PrettyName;
    if( name == L"OrbitStart" )
    {
        a_Function.m_OrbitType = Function::ORBIT_TIMER_START;
    }
    else if (name == L"OrbitStop")
    {
        a_Function.m_OrbitType = Function::ORBIT_TIMER_STOP;
    }
    else if (name == L"OrbitLog")
    {
        a_Function.m_OrbitType = Function::ORBIT_LOG;
    }
    else if( name == L"OutputDebugStringA" )
    {
        a_Function.m_OrbitType = Function::ORBIT_OUTPUT_DEBUG_STRING;
    }
    else if( name == L"OrbitSendData" )
    {
        a_Function.m_OrbitType = Function::ORBIT_DATA;
    }
}

//-----------------------------------------------------------------------------
void Pdb::AddType( const Type & a_Type )
{
    if( m_TypeMap.find( a_Type.m_Id ) == m_TypeMap.end() )
    {
        m_TypeMap[a_Type.m_Id] = a_Type;
        m_Types.push_back( a_Type );
    }
}

//-----------------------------------------------------------------------------
void Pdb::AddGlobal( const Variable & a_Global )
{
    m_Globals.push_back( a_Global );
}

//-----------------------------------------------------------------------------
void Pdb::AddArgumentRegister( const string & a_Reg, const string & a_Function )
{
    m_ArgumentRegisters.insert(a_Reg);

    if( a_Reg.find("ESP") != string::npos )
    {
        m_RegFunctionsMap["ESP"].push_back( a_Function );
    }
    else if( a_Reg.find("30006") != string::npos )
    {
        m_RegFunctionsMap["30006"].push_back( a_Function );
    }
}

//-----------------------------------------------------------------------------
Type * Pdb::GetTypePtrFromId( ULONG a_ID )
{
    auto it = m_TypeMap.find(a_ID);
    if( it != m_TypeMap.end() )
    {
        return &it->second;
    }

    return nullptr;
}

//-----------------------------------------------------------------------------
GUID Pdb::GetGuid()
{
    GUID guid = {0};
    if( m_DiaGlobalSymbol )
    {
        m_DiaGlobalSymbol->get_guid( &guid );
    }

    return guid;
}

//-----------------------------------------------------------------------------
void Pdb::PrintFunction( Function& a_Function )
{
    a_Function.Print();
}

//-----------------------------------------------------------------------------
void Pdb::Clear()
{
    m_Functions.clear();
    m_Types.clear();
    m_Globals.clear();
    m_TypeMap.clear();
    m_FunctionMap.clear();
    m_FileName = L"";
}

//-----------------------------------------------------------------------------
void Pdb::Reserve()
{
    const int size = 8*1024;
    m_Functions.reserve(size);
    m_Types.reserve(size);
    m_Globals.reserve(size);
}

//-----------------------------------------------------------------------------
void Pdb::Print() const
{
    for( auto & reg : m_ArgumentRegisters )
    {
        ORBIT_LOGV( reg );
    }

    ORBIT_LOG("\n\nEsp functions:");
    for (auto & reg : m_RegFunctionsMap)
    {
        ORBIT_LOGV(reg.first);
        for( auto & func : reg.second )
        {
            ORBIT_LOGV(func);
        }
    }
}

//-----------------------------------------------------------------------------
void Pdb::PrintGlobals() const
{
    for (const auto & var : m_Globals)
    {
        PRINT_VAR( var.ToString() );
    }
}

//-----------------------------------------------------------------------------
wstring Pdb::GetCachedName()
{
    string pdbName = ws2s( Path::GetFileName( m_FileName ) );
    tr2::sys::path fileName = GuidToString( m_ModuleInfo.PdbSig70 ) + "-" + ToHexString( m_ModuleInfo.PdbAge ) + "_" + pdbName;
    fileName.replace_extension(".bin");
    return fileName.wstring();
}

//-----------------------------------------------------------------------------
wstring Pdb::GetCachedKey()
{
    wstring cachedName = GetCachedName();
    wstring cachedKey = cachedName.substr( 0, cachedName.find_first_of('_') );
    return cachedKey;
}

//-----------------------------------------------------------------------------
bool Pdb::Load( const string & /*a_CachedPdb*/ )
{
    /*ifstream is( a_CachedPdb, ios::binary );
    if( !is.fail() )
    {
        cereal::BinaryInputArchive Ar( is );
        Ar(*this);
        m_LoadedFromCache = true;
        return true;
    }*/

    return false;
}

//-----------------------------------------------------------------------------
void Pdb::Update()  
{
    if( m_FinishedLoading )
    {
        m_LoadingCompleteCallback();
        m_FinishedLoading = false;
        Print();
        
        if( !m_LoadedFromCache && false )
        {
            Save();
        }
    }

    if( m_IsLoading )
    {
        SendStatusToUi();
    }
}

//-----------------------------------------------------------------------------
void Pdb::SendStatusToUi()
{
    wstring status = Format( L"status:Parsing %s\nFunctions: %i\nTypes: %i\nGlobals: %i\n"
                                , m_Name.c_str()
                                , m_Functions.size()
                                , m_Types.size()
                                , m_Globals.size() );

    if( m_IsPopulatingFunctionMap )
    {
        status += L"PopulatingFunctionMap\n";
    }
    if( m_IsPopulatingFunctionStringMap )
    {
        status += L"PopulatingFunctionStringMap\n";
    }

    int numPoints = 10;
    int period = 4000;
    int progress = (int)(float(((ULONG)m_LoadTimer->QueryMillis()%period))/(float)period * (float)numPoints);
    if( progress > numPoints ) progress = numPoints;
    for( int i = 0; i <= progress; ++i )
    {
        status += L".";
    }

    GTcpServer->SendToUiNow( status );
}

//-----------------------------------------------------------------------------
void ParseDll( const char* a_FileName );

//-----------------------------------------------------------------------------
bool PdbGetFileSize( const TCHAR* pFileName, DWORD& FileSize )
{
    HANDLE hFile = ::CreateFile( pFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );

    if( hFile == INVALID_HANDLE_VALUE )
    {
        ORBIT_ERROR;
        return false;
    }

    // Obtain the size of the file 
    FileSize = ::GetFileSize( hFile, NULL );

    if( FileSize == INVALID_FILE_SIZE )
    {
        ORBIT_ERROR;
        // and continue ... 
    }

    // Close the file 
    if( !::CloseHandle( hFile ) )
    {
        ORBIT_ERROR;
        // and continue ... 
    }

    // Complete 
    return ( FileSize != INVALID_FILE_SIZE );
}


//-----------------------------------------------------------------------------
bool GetFileParams(const TCHAR* pFileName, DWORD64& BaseAddr, DWORD& FileSize)
{
    // Check parameters 

    if( pFileName == 0 ) 
    {
        return false; 
    }


    // Determine the extension of the file 

    TCHAR szFileExt[_MAX_EXT] = {0}; 

    /*
    void __cdecl _tsplitpath (
    register const _TSCHAR *path,
    _TSCHAR *drive,
    _TSCHAR *dir,
    _TSCHAR *fname,
    _TSCHAR *ext
    )
    {
    */

    _tsplitpath_s( pFileName, NULL, 0, NULL, 0, NULL, 0, szFileExt, _MAX_EXT ); 


    // Is it .PDB file ? 

    if( _tcsicmp( szFileExt, _T(".PDB") ) == 0 ) 
    {
        // Yes, it is a .PDB file 

        // Determine its size, and use a dummy base address 

        BaseAddr = 0x10000000; // it can be any non-zero value, but if we load symbols 
        // from more than one file, memory regions specified 
        // for different files should not overlap 
        // (region is "base address + file size") 

        if( !PdbGetFileSize( pFileName, FileSize ) ) 
        {
            return false; 
        }

    }
    else 
    {
        // It is not a .PDB file 

        // Base address and file size can be 0 

        BaseAddr = 0; 
        FileSize = 0; 
    }


    // Complete 

    return true; 

}

//-----------------------------------------------------------------------------
void ShowSymbolInfo( IMAGEHLP_MODULE64 & ModuleInfo ) 
{
    switch( ModuleInfo.SymType ) 
    {
    case SymNone:       ORBIT_LOG("No symbols available for the module.\n"); break;
    case SymExport:     ORBIT_LOG("Loaded symbols: Exports\n");              break;
    case SymCoff:       ORBIT_LOG("Loaded symbols: COFF\n");                 break;
    case SymCv:         ORBIT_LOG("Loaded symbols: CodeView\n");             break;
    case SymSym:        ORBIT_LOG("Loaded symbols: SYM\n");                  break;
    case SymVirtual:    ORBIT_LOG("Loaded symbols: Virtual\n");              break;
    case SymPdb:        ORBIT_LOG("Loaded symbols: PDB\n");                  break;
    case SymDia:        ORBIT_LOG("Loaded symbols: DIA\n");                  break;
    case SymDeferred:   ORBIT_LOG("Loaded symbols: Deferred\n");             break;
    default:            ORBIT_LOG("Loaded symbols: Unknown format.\n");      break;
    }

    // Image name 
    if( wcslen( ModuleInfo.ImageName ) > 0 ) 
    {
        ORBIT_LOG( Format( TEXT("Image name: %s \n"), ModuleInfo.ImageName ) );
    }

    // Loaded image name 
    if( wcslen( ModuleInfo.LoadedImageName ) > 0 ) 
    {
        ORBIT_LOG( Format( TEXT("Loaded image name: %s \n"), ModuleInfo.LoadedImageName ) ); 
    }

    // Loaded PDB name 
    if( wcslen( ModuleInfo.LoadedPdbName ) > 0 ) 
    {
        ORBIT_LOG( Format( TEXT("PDB file name: %s \n"), ModuleInfo.LoadedPdbName ) ); 
    }

    // Is debug information unmatched ? 
    // (It can only happen if the debug information is contained 
    // in a separate file (.DBG or .PDB) 
    if( ModuleInfo.PdbUnmatched || ModuleInfo.DbgUnmatched ) 
    {
        ORBIT_LOG( Format( _T("Warning: Unmatched symbols. \n") ) ); 
    }

    // Line numbers available ? 
    ORBIT_LOG( Format( _T("Line numbers: %s \n"), ModuleInfo.LineNumbers ? _T("Available") : _T("Not available") ) ); 

    // Global symbols available ? 
    ORBIT_LOG( Format( _T("Global symbols: %s \n"), ModuleInfo.GlobalSymbols ? _T("Available") : _T("Not available" ) ) ); 

    // Type information available ? 
    ORBIT_LOG( Format( _T("Type information: %s \n"), ModuleInfo.TypeInfo ? _T("Available") : _T("Not available") ) ); 

    // Source indexing available ? 
    ORBIT_LOG( Format( _T("Source indexing: %s \n"), ModuleInfo.SourceIndexed ? _T("Yes") : _T("No") ) ); 

    // Public symbols available ?
    ORBIT_LOG( Format( _T("Public symbols: %s \n"), ModuleInfo.Publics ? _T("Available") : _T("Not available") ) ); 
}


//-----------------------------------------------------------------------------
bool Pdb::LoadPdb( const wchar_t* a_PdbName )
{
    SCOPE_TIMER_LOG( L"LOAD PDB" );

    m_IsLoading = true;
    m_LoadTimer->Start();

    string msg = "pdb:" + ws2s( a_PdbName );
    GTcpServer->SendToUiAsync( msg );

    string nameStr = ws2s( a_PdbName );

    if( ToLower( Path::GetExtension( a_PdbName ) ) == L".dll" )
    {
        SCOPE_TIMER_LOG( L"LoadDll Exports" );
        ParseDll( nameStr.c_str() );
    }
    else
    {
        SCOPE_TIMER_LOG( L"LoadPdbDia" );
        LoadPdbDia();
    }

    ShowSymbolInfo( m_ModuleInfo );
    ProcessData();
    GParams.AddToPdbHistory( ws2s(a_PdbName).c_str() );

    m_FinishedLoading = true;
    m_IsLoading = false;

    return true;
}

//-----------------------------------------------------------------------------
bool Pdb::LoadDataFromPdb()
{
    DiaManager diaManager;
    if( !diaManager.LoadDataFromPdb( m_FileName.c_str(), &m_DiaSession, &m_DiaGlobalSymbol ) )
    {
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
bool Pdb::LoadPdbDia()
{
    if( m_DiaGlobalSymbol )
    {
        Reserve();
        auto group = oqpi_tk::make_parallel_group<oqpi::task_type::waitable>( "Fork" );
        group->addTask( oqpi_tk::make_task_item( "DumpAllFunctions"  , DumpAllFunctions  , m_DiaGlobalSymbol ) );
        group->addTask( oqpi_tk::make_task_item( "DumpTypes"         , DumpTypes         , m_DiaGlobalSymbol ) );
        group->addTask( oqpi_tk::make_task_item( "HookDumpAllGlobals", OrbitDumpAllGlobals, m_DiaGlobalSymbol ) );
        oqpi_tk::schedule_task( oqpi::task_handle( group ) ).wait();

        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
void Pdb::LoadPdbAsync( const wchar_t* a_PdbName, function<void()> a_CompletionCallback )
{
    m_FileName = a_PdbName;
    m_Name = Path::GetFileName( m_FileName );

    m_LoadingCompleteCallback = a_CompletionCallback;
    m_LoadingThread = make_unique<thread>( &Pdb::LoadPdb, this, m_FileName.c_str() );
    m_LoadingThread->detach();
}

//-----------------------------------------------------------------------------
void Pdb::PopulateFunctionMap()
{
    m_IsPopulatingFunctionMap = true;
    
    SCOPE_TIMER_LOG( Format( L"Pdb::PopulateFunctionMap for %s", m_FileName.c_str() ) );

    for( Function & Function : m_Functions )
    {
        m_FunctionMap.insert( make_pair( Function.m_Address, &Function ) );
    }

    m_IsPopulatingFunctionMap = false;
}

//-----------------------------------------------------------------------------
void Pdb::PopulateStringFunctionMap()
{
    m_IsPopulatingFunctionStringMap = true;

    SCOPE_TIMER_LOG( Format( L"Pdb::PopulateStringFunctionMap for %s", m_FileName.c_str() ) );

    {
        SCOPE_TIMER_LOG( L"Reserving map" );
        m_StringFunctionMap.reserve( unsigned ( 1.5f * (float)m_Functions.size() ) );
    }

    {
        SCOPE_TIMER_LOG( L"Hash" );

        if( m_Functions.size() > 1000 )
        {
            oqpi_tk::parallel_for_each( "StringMap", m_Functions,[]( Function & a_Function )
            {
                a_Function.Hash();
            } );
        }
        else
        {
            for( Function & Function : m_Functions )
            {
                Function.Hash();
            }
        }
    }

    {
        SCOPE_TIMER_LOG( L"Map inserts" );

        for( Function & Function : m_Functions )
        {
            m_StringFunctionMap[Function.m_NameHash] = &Function;
        }
    }

    m_IsPopulatingFunctionStringMap = false;
}

//-----------------------------------------------------------------------------
void Pdb::ApplyPresets()
{
    SCOPE_TIMER_LOG( Format( L"Pdb::ApplyPresets - %s", m_Name.c_str() ) );

    if (Capture::GSessionPresets)
    {
        wstring pdbName = Path::GetFileName( m_Name );

        auto it = Capture::GSessionPresets->m_Modules.find(pdbName);
        if (it != Capture::GSessionPresets->m_Modules.end())
        {
            SessionModule & a_Module = it->second;

            for( uint64_t hash : a_Module.m_FunctionHashes )
            {
                PRINT_VAR(hash);
                auto fit = m_StringFunctionMap.find( hash );
                if( fit != m_StringFunctionMap.end() )
                {
                    Function * function = fit->second;
                    function->Select();
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------
Function* Pdb::GetFunctionFromExactAddress( DWORD64 a_Address )
{
    DWORD64 address = a_Address - (DWORD64)GetHModule();

    if( m_FunctionMap.find(address) != m_FunctionMap.end() )
    {
        return m_FunctionMap[address];
    }

    return nullptr;
}

//-----------------------------------------------------------------------------
Function* Pdb::GetFunctionFromProgramCounter( DWORD64 a_Address )
{
    DWORD64 address = a_Address - (DWORD64)GetHModule();

    auto & it = m_FunctionMap.upper_bound( address );
    if (!m_FunctionMap.empty() && it != m_FunctionMap.begin())
    {
        --it;
        Function* func = it->second;
        return func;
    }

    return nullptr;
}

//-----------------------------------------------------------------------------
IDiaSymbol* Pdb::SymbolFromAddress( DWORD64 a_Address )
{
    if( m_DiaSession )
    {
        IDiaSymbol* symbol;
        DWORD rva = DWORD(a_Address - (DWORD64)GetHModule());
        auto error = m_DiaSession->findSymbolByRVA( rva, SymTagFunction, &symbol );
        if( error == S_OK )
        {
            return symbol;
        }
        else
        {
            PrintLastError();
        }
    }

    return nullptr;
}

//-----------------------------------------------------------------------------
bool Pdb::LineInfoFromAddress( DWORD64 a_Address, LineInfo & o_LineInfo )
{
    if( !m_DiaSession )
    {
        return false;
    }

    IDiaEnumLineNumbers * lineNumbers = nullptr;
    DWORD rva = DWORD(a_Address - (DWORD64)GetHModule());
    wstring fileNameW;
    if( SUCCEEDED( m_DiaSession->findLinesByRVA( rva, 1, &lineNumbers ) ) )
    {
        IDiaLineNumber* pLineNumber;
        ULONG celt = 0;

        while( SUCCEEDED( lineNumbers->Next( 1, &pLineNumber, &celt ) ) && celt == 1 && fileNameW.size() == 0 )
        {
            IDiaSourceFile* sourceFile;
            if( SUCCEEDED( pLineNumber->get_sourceFile( &sourceFile ) ) )
            {
                BSTR fileName;
                if( SUCCEEDED( sourceFile->get_fileName( &fileName ) ) )
                {
                    fileNameW = wstring( fileName );
                    o_LineInfo.m_Address = a_Address;
                    o_LineInfo.m_File = fileName;
                    o_LineInfo.m_Line = 0;

                    SysFreeString( fileName );
                }

                DWORD lineNumber;
                if( SUCCEEDED( pLineNumber->get_lineNumber( &lineNumber ) ) )
                {
                    o_LineInfo.m_Line = lineNumber;
                }

                sourceFile->Release();
            }

            pLineNumber->Release();
        }

        lineNumbers->Release();
    }

    return fileNameW.size() > 0;
}

//-----------------------------------------------------------------------------
IDiaSymbol* Pdb::GetDiaSymbolFromId( ULONG a_Id )
{
    IDiaSymbol* symbol;
    if( m_DiaSession )
    {
        auto error = m_DiaSession->symbolById( a_Id, &symbol );
        if( error  == S_OK )
        {
            return symbol;
        }
        else
        {
            PrintLastError();
        }
    }

    return nullptr;
}

//-----------------------------------------------------------------------------
void Pdb::ProcessData()
{
    SCOPE_TIMER_LOG( Format( L"Pdb::ProcessData for %s", m_Name.c_str() ) );

    ScopeLock lock( Capture::GTargetProcess->GetDataMutex() );

    auto & functions = Capture::GTargetProcess->GetFunctions();
    auto & globals   = Capture::GTargetProcess->GetGlobals();

    functions.reserve( functions.size() + m_Functions.size() );

    for( Function & func : m_Functions )
    {
        func.m_Pdb = this;
        functions.push_back( &func );
        GOrbitUnreal.OnFunctionAdded( &func );
    }

    for( Type & type : m_Types )
    {
        type.m_Pdb = this;
        Capture::GTargetProcess->AddType( type );
        GOrbitUnreal.OnTypeAdded( &type );
    }

    for( Variable & var : m_Globals )
    {
        var.m_Pdb = this;
        globals.push_back( &var );
    }

    for( auto & it : m_TypeMap )
    {
        it.second.m_Pdb = this;
    }

    PopulateFunctionMap();
    PopulateStringFunctionMap();
    // TODO: parallelize: PopulateStringFunctionMap();
}

//-----------------------------------------------------------------------------
void Pdb::Save()
{
    wstring fullName = Path::GetCachePath() + GetCachedName();

    SCOPE_TIMER_LOG( Format( L"Saving %s", fullName.c_str() ) );

    //ofstream os( fullName, ios::binary );
    //cereal::BinaryOutputArchive Ar(os);
    //Ar( *this );
}

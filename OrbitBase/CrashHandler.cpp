//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "CrashHandler.h"
#include "ScopeTimer.h"
#include "OrbitDbgHelp.h"
#include "PrintVar.h"
#include "Path.h"

#if _WIN32||_WIN64
#include "client/windows/handler/exception_handler.h"
#else
#include "client/linux/handler/exception_handler.h"
#endif

#include <fstream>

using namespace std;

//-----------------------------------------------------------------------------
void SendDumpInternal( const wstring & a_Dir, const wstring & a_Id );
google_breakpad::ExceptionHandler* GHandler;

//-----------------------------------------------------------------------------
bool DmpFilter( void* context )
{
    return true;
}

//-----------------------------------------------------------------------------
bool DmpCallback( const google_breakpad::MinidumpDescriptor& descriptor, void* context, bool succeeded )
{
    PRINT_FUNC;

    wstring dir = Path::GetDumpPath().wstring();
    wstring msg = succeeded ? 
                  L"A crash dump was generated in " + dir :
                  L"Failed to generate crash dump";
    /*MessageBox(
        NULL,
        msg.c_str(),
        L"Orbit Crash Handler: ",
        MB_ICONEXCLAMATION | MB_OK
    );*/
    printf("%S", msg.c_str());
    return false;
}

//-----------------------------------------------------------------------------
bool OnDemandDmpCallback( const google_breakpad::MinidumpDescriptor& descriptor, 
    void* context, bool succeeded )
{
    //SendDumpInternal( dump_path, minidump_id );
    return false;
}

//-----------------------------------------------------------------------------
CrashHandler::CrashHandler()
{
    // Creates a new ExceptionHandler instance to handle writing minidumps.
    // Before writing a minidump, the optional filter callback will be called.
    // Its return value determines whether or not Breakpad should write a
    // minidump.  Minidump files will be written to dump_path, and the optional
    // callback is called after writing the dump file, as described above.
    // handler_types specifies the types of handlers that should be installed.

    assert( GHandler == nullptr );

    GHandler = new google_breakpad::ExceptionHandler(
        google_breakpad::MinidumpDescriptor(Path::GetDumpPath().c_str()),
        DmpFilter,
        DmpCallback,
        0, // context
        true, //google_breakpad::ExceptionHandler::HANDLER_ALL,
        //MiniDumpNormal,
        //L"",
        0 );
}

//-----------------------------------------------------------------------------
void CrashHandler::SendMiniDump()
{
    SCOPE_TIMER_LOG(L"CrashHandler::WriteDump");
    google_breakpad::ExceptionHandler::WriteMinidump( Path::GetDumpPath(), OnDemandDmpCallback, nullptr );
}

//-----------------------------------------------------------------------------
void SendDumpInternal( const wstring & a_Dir, const wstring & a_Id )
{
    wstring fileName = a_Dir + a_Id + L".dmp";
    ifstream file( ws2s(fileName).c_str(), ios::binary | ios::ate );
    streamsize size = file.tellg();
    file.seekg( 0, ios::beg );

    vector<char> buffer( (unsigned int)size );
    if( file.read( buffer.data(), size ) )
    {
        /*if( GTcpClient )
        {
            Message msg(Msg_MiniDump);
            msg.m_Header.m_GenericHeader.m_Address = GetCurrentProcessId();
            GTcpClient->Send( msg, buffer );
        }*/
    }

    file.close();
}

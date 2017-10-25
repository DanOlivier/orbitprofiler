//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "MiniDump.h"
#include "OrbitProcess.h"
#include "CoreApp.h"
#include "PrintVar.h"
#include "Path.h"

#include <memory>
#include <google_breakpad/processor/minidump.h>

using namespace std;
using namespace google_breakpad;
namespace fs = std::experimental::filesystem;

//-----------------------------------------------------------------------------
MiniDump::MiniDump( wstring a_FileName )
{
    m_MiniDump = new google_breakpad::Minidump( ws2s(a_FileName) );
    m_MiniDump->Read();
}

//-----------------------------------------------------------------------------
MiniDump::~MiniDump()
{
    delete m_MiniDump;
}

//-----------------------------------------------------------------------------
shared_ptr<Process> MiniDump::ToOrbitProcess()
{
    MinidumpModuleList* moduleList = m_MiniDump->GetModuleList();
    if( moduleList )
    {
        m_MiniDump->Print();
        shared_ptr<Process> process = make_shared<Process>();
        process->SetIsRemote(true);
        process->SetID( 0 );

        unsigned int numModules = moduleList->module_count();
        for( unsigned int i = 0; i < numModules; ++i )
        {
            const MinidumpModule* module = moduleList->GetModuleAtIndex(i);
            fs::path fullName = module->code_file();
            PRINT_VAR( module->base_address() );
            PRINT_VAR( fullName );
            PRINT_VAR( module->code_identifier() );
            PRINT_VAR( module->debug_file() );
            PRINT_VAR( module->debug_identifier() );

            shared_ptr<Module> mod = make_shared<Module>();
            
            mod->m_FullName = fullName.wstring();
            mod->m_Name = Path::GetFileName( fullName ).wstring();
            
            if( EndsWith( mod->m_Name, L".exe" ) )
            {
                process->m_Name = mod->m_Name;
            }

            mod->m_Directory = Path::GetDirectory( fullName ).wstring();
            mod->m_AddressStart = module->base_address();
            mod->m_AddressEnd =   module->base_address() + module->size();
            mod->m_DebugSignature = s2ws( module->debug_identifier() );

            process->AddModule(mod);
        }

        process->FindPdbs(GCoreApp->m_SymbolLocations);
        return process;
    }

    return nullptr;
}

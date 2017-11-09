//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "MiniDump.h"
#include "PTM/OrbitProcess.h"
#include "PrintVar.h"
#include "Path.h"

#include <memory>
#include <google_breakpad/processor/minidump.h>

using namespace std;
using namespace google_breakpad;
namespace fs = std::experimental::filesystem;

//-----------------------------------------------------------------------------
MiniDump::MiniDump( const fs::path& a_FileName )
{
    m_MiniDump.reset(new google_breakpad::Minidump( a_FileName ));
    m_MiniDump->Read();
}

//-----------------------------------------------------------------------------
MiniDump::~MiniDump()
{
    //delete m_MiniDump;
}

//-----------------------------------------------------------------------------
shared_ptr<Process> MiniDump::ToOrbitProcess(const std::vector<fs::path>& symbolLocations)
{
    /*MinidumpThreadList* threadList = m_MiniDump->GetThreadList();
    if(threadList)
    {
        
    }*/
    MinidumpModuleList* moduleList = m_MiniDump->GetModuleList();
    if( moduleList )
    {
        m_MiniDump->Print();
        shared_ptr<Process> process = make_shared<Process>(0);
        process->SetIsRemote(true);
        //process->SetID( 0 );

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
            
            mod->m_FullName = fullName;
            mod->m_Name = fullName.filename();
            
            if( module == moduleList->GetMainModule() )
            {
                process->m_Name = mod->m_Name;
            }

            mod->m_Directory = fullName.parent_path();
            mod->m_AddressStart = module->base_address();
            mod->m_AddressEnd =   module->base_address() + module->size();
            mod->m_DebugSignature = s2ws( module->debug_identifier() );

            process->AddModule(mod);
        }

        process->FindPdbs(symbolLocations);
        return process;
    }

    return nullptr;
}

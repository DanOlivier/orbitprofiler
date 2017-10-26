//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "ModuleManager.h"
#include "OrbitModule.h"
#include "Tcp.h"
#include "TcpServer.h"
#include "Capture.h"
#include "OrbitProcess.h"
#include "CoreApp.h"
#include "Path.h"
#include "Pdb.h"

using namespace std;

ModuleManager GModuleManager;

//-----------------------------------------------------------------------------
ModuleManager::ModuleManager()
{
    GPdbDbg = make_shared<Pdb>(L"");
}

//-----------------------------------------------------------------------------
ModuleManager::~ModuleManager()
{

}

//-----------------------------------------------------------------------------
void ModuleManager::Init()
{
    GTcpServer->SetCallback(Msg_SetData, [=](const Message & a_Msg){ this->OnReceiveMessage(a_Msg); });
}

//-----------------------------------------------------------------------------
void ModuleManager::OnReceiveMessage( const Message & a_Msg )
{
    if (a_Msg.GetType() == Msg_SetData)
    {
        const DataTransferHeader & header = a_Msg.GetHeader().m_DataTransferHeader;
        ULONG64 address = (ULONG64)header.m_Address - (ULONG64)GPdbDbg->GetHModule();
        DataTransferHeader::DataType dataType = header.m_Type;

        if( dataType == DataTransferHeader::Data )
        {
            // TODO: make access to watched vars thread safe
            for (shared_ptr<Variable> var : Capture::GTargetProcess->GetWatchedVariables())
            {
                if (var->m_Address == address)
                {
                    var->ReceiveValue(a_Msg);
                    break;
                }
            }
        }
        else if( dataType == DataTransferHeader::Code )
        {
            if( Function* func = Capture::GTargetProcess->GetFunctionFromAddress( header.m_Address, true ) )
            {
                GCoreApp->Disassemble( func, a_Msg.GetData(), a_Msg.m_Size );
            }
        }
    }
}

//-----------------------------------------------------------------------------
void ModuleManager::LoadPdbAsync( const shared_ptr<Module> & a_Module, function<void()> a_CompletionCallback )
{   
    if (!a_Module->m_Loaded)
    {
        bool loadExports = a_Module->IsDll() && !a_Module->m_FoundPdb;
        if( a_Module->m_FoundPdb || loadExports )
        {
            fs::path pdbName = loadExports ? a_Module->m_FullName : a_Module->m_PdbName;
            m_UserCompletionCallback = a_CompletionCallback;

            GPdbDbg = a_Module->m_Pdb;
            if( GPdbDbg )
            {
                GPdbDbg->SetMainModule((HMODULE)a_Module->m_AddressStart);
                GPdbDbg->LoadPdbAsync(pdbName.c_str(), [&]() { this->OnPdbLoaded(); });
            }
        }
    }
}

//-----------------------------------------------------------------------------
void ModuleManager::LoadPdbAsync(const vector<fs::path> a_Modules, function<void()> a_CompletionCallback)
{
    m_UserCompletionCallback = a_CompletionCallback;
    m_ModulesQueue = a_Modules;
    DequeueAndLoad();
}

//-----------------------------------------------------------------------------
void ModuleManager::DequeueAndLoad()
{
    shared_ptr<Module> module = nullptr;
    
    while( module == nullptr && !m_ModulesQueue.empty() )
    {
        fs::path pdbName = m_ModulesQueue.back();
        m_ModulesQueue.pop_back();
    
        module = Capture::GTargetProcess->FindModule( Path::GetFileName( pdbName ).wstring() );
        if( module )
        {
            GPdbDbg = module->m_Pdb;
            if( module->m_PdbName.empty() )
            {
                module->m_PdbName = module->m_FullName;
            }

            GPdbDbg->LoadPdbAsync( module->m_PdbName.c_str(), [&](){ this->OnPdbLoaded(); });
            return;
        }
    }

    // No module was found, call user callback
    m_UserCompletionCallback();
}

//-----------------------------------------------------------------------------
void ModuleManager::OnPdbLoaded()
{
    shared_ptr<Pdb> lastPdb = GPdbDbg;
    AddPdb( lastPdb );

    if( !m_ModulesQueue.empty() )
    {
        // Start loading next pdb on other thread
        DequeueAndLoad();
    }
    
    // Apply presets on last pdb
    lastPdb->ApplyPresets();

    if( m_ModulesQueue.empty() )
    {
        m_UserCompletionCallback();
    }
}

//-----------------------------------------------------------------------------
void ModuleManager::AddPdb( const shared_ptr<Pdb> & a_Pdb )
{ 
    Process::ModuleMap_t& modules = Capture::GTargetProcess->GetModules();

    auto it = modules.find( (DWORD64)a_Pdb->GetHModule() );
    if( it != modules.end() )
    {
        it->second->m_Loaded = true;
    }
}

//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------


#include "PluginManager.h"
#include "TcpServer.h"
#include "../OrbitPlugin/OrbitSDK.h"
#include "Path.h"

#include <dlfcn.h>

PluginManager GPluginManager;

using namespace std;
namespace fs = std::experimental::filesystem;    

//-----------------------------------------------------------------------------
void PluginManager::Initialize()
{
    fs::path dir = Path::GetPluginPath();
    vector< fs::path > plugins = Path::ListFiles( dir, L".dll" );

    for( fs::path & file : plugins )
    {
        HMODULE module = dlopen(file.c_str(), RTLD_NOW);
        using function = Orbit::Plugin*();
        function* func = (function*)dlsym(module, "CreateOrbitPlugin");
        if( func )
        {
            Orbit::Plugin* newPlugin = func();
            static int count = 0;
            newPlugin->SetPluginID(count++);
            m_Plugins.push_back( newPlugin );
        }
    }

    GTcpServer->SetCallback( Msg_UserData, [=]( const Message & a_Msg ) { 
        this->OnReceiveUserData(a_Msg);
    } );
    GTcpServer->SetCallback( Msg_OrbitData, [=]( const Message & a_Msg ){ 
        this->OnReceiveOrbitData(a_Msg);
    } );
}

//-----------------------------------------------------------------------------
void PluginManager::OnReceiveUserData( const Message & a_Msg )
{
    if( a_Msg.GetType() == Msg_UserData )
    {
        Orbit::UserData* userData = (Orbit::UserData*)a_Msg.m_Data;
        userData->m_Data = (char*)userData + sizeof(Orbit::UserData);

        for( Orbit::Plugin* plugin : m_Plugins )
        {
            plugin->ReceiveUserData( userData );
        }
    }
}

//-----------------------------------------------------------------------------
void PluginManager::OnReceiveOrbitData( const Message & a_Msg )
{
    if( a_Msg.GetType() == Msg_OrbitData )
    {
        Orbit::Data* orbitData = ( Orbit::Data* )a_Msg.m_Data;
        orbitData->m_Data = orbitData + sizeof( Orbit::Data );

        for( Orbit::Plugin* plugin : m_Plugins )
        {
            plugin->ReceiveOrbitData( orbitData );
        }
    }
}
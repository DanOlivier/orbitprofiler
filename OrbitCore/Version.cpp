#include "Version.h"
#include "Utils.h"

#include <curl/curl.h>
#include <websocketpp/base64/base64.hpp>

#include <thread>

using namespace std;

//-----------------------------------------------------------------------------
bool   OrbitVersion::s_NeedsUpdate;
string OrbitVersion::s_LatestVersion;

#define OrbitVersionStr "dev"

//-----------------------------------------------------------------------------
string OrbitVersion::GetVersion()
{
    return OrbitVersionStr;
}

//-----------------------------------------------------------------------------
bool OrbitVersion::IsDev()
{
    return GetVersion() == string("dev");
}

//-----------------------------------------------------------------------------
bool OrbitVersion::CheckLicense( const wstring & a_License )
{
    vector< wstring > tokens = Tokenize( a_License, L"\r\n" );
    if( tokens.size() == 2 )
    {
        string decodedStr = XorString( websocketpp::base64_decode( ws2s(tokens[1]) ) );

        if( decodedStr == ws2s( tokens[0] ) )
        {
            return true;
        }
    }

    return false;
}

//-----------------------------------------------------------------------------
static size_t WriteCallback( void *contents, size_t size, size_t nmemb, void *userp )
{
    ( ( string* )userp )->append( (char*)contents, size * nmemb );
    return size * nmemb;
}

//-----------------------------------------------------------------------------
void OrbitVersion::CheckForUpdate()
{
    if( !IsDev() )
    {
        thread t(CheckForUpdateThread);
        t.detach();
    }
}

//-----------------------------------------------------------------------------
void OrbitVersion::CheckForUpdateThread()
{
    CURL *curl;
    CURLcode res;
    string readBuffer;

    curl = curl_easy_init();
    if( curl )
    {
        curl_easy_setopt( curl, CURLOPT_URL, "http://www.telescopp.com/update" );
        curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, WriteCallback );
        curl_easy_setopt( curl, CURLOPT_WRITEDATA, &readBuffer );
        res = curl_easy_perform( curl );
        curl_easy_cleanup( curl );

        // Get latest version from html, this needs to match what is on the website...
        string searchStr = "Latest version of the Orbit Profiler is: ";
        size_t pos = readBuffer.find( searchStr );

        if( pos != string::npos )
        {
            string version = Replace( readBuffer.substr( pos, 60 ), searchStr, "" );
            vector<string> tokens = Tokenize( version );

            if( tokens.size() > 0 )
            {
                s_LatestVersion = tokens[0];
                s_NeedsUpdate = s_LatestVersion != GetVersion();
            }
        }
    }
}

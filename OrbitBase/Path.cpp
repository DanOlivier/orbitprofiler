//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Path.h"
#include "Utils.h"

#include <limits.h> // PATH_MAX
#include <unistd.h> // readlink

#if _WIN32||_WIN64
#include <direct.h>
#include <Shlobj.h>
#endif

using namespace std;
namespace fs = std::experimental::filesystem;

namespace Path {

fs::path m_BasePath;
bool    m_IsPackaged;

#define countof(arr) sizeof(arr) / sizeof(arr[0])
//-----------------------------------------------------------------------------
fs::path GetExecutableName()
{
#if _WIN32||_WIN64
    wstring exeFullName;
    WCHAR  cwBuffer[2048] = { 0 };
    LPWSTR pszBuffer = cwBuffer;
    DWORD  dwMaxChars = countof(cwBuffer);
    DWORD dwLength = ::GetModuleFileNameW(NULL, pszBuffer, dwMaxChars);
    exeFullName = wstring(pszBuffer);

    // Clean up "../" inside full path
    wchar_t buffer[MAX_PATH];
    GetFullPathName( exeFullName.c_str(), MAX_PATH, buffer, nullptr );
    exeFullName = buffer;

    replace(exeFullName.begin(), exeFullName.end(), '\\', '/');
    return exeFullName;
#else
    char buf2[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buf2, sizeof(buf2)-1);
    if (len == 0) {
        // Error
    }
    return fs::path(buf2);
#endif
}

//-----------------------------------------------------------------------------
fs::path GetExecutablePath()
{
    fs::path fullPath = GetExecutableName();
    return fullPath.parent_path();
}

//-----------------------------------------------------------------------------
fs::path GetBasePath()
{
    if( !m_BasePath.empty() )
        return m_BasePath;

    fs::path exePath = GetExecutablePath();
    fs::path basePath;
    for(const auto& p : exePath)
    {
        if(p == "bin")
            break;
        basePath /= p;
    }
    m_BasePath = basePath;
    m_IsPackaged = fs::is_directory(m_BasePath / "text" );

    return m_BasePath;
}
 
//-----------------------------------------------------------------------------
fs::path GetDllPath( bool a_Is64Bit )
{
    fs::path basePath = GetBasePath();
    if(!m_IsPackaged)
    {
        basePath /= "bin";
        basePath /= a_Is64Bit ? "x64" : "Win32";
        
#ifdef _DEBUG
        basePath /= "Debug";
#else
        basePath /= "Release";
#endif
    }
    return basePath / GetDllName( a_Is64Bit );
}

//-----------------------------------------------------------------------------
fs::path GetDllName( bool a_Is64Bit )
{
    return a_Is64Bit ? "Orbit64.dll" : "Orbit32.dll";
}

//-----------------------------------------------------------------------------
fs::path GetParamsFileName()
{
    fs::path paramsDir = GetAppDataPath() / "config/";
    error_code ec;
    fs::create_directory( paramsDir, ec );
    return paramsDir / "config.xml";
}

//-----------------------------------------------------------------------------
fs::path GetFileMappingFileName()
{
    fs::path paramsDir = GetAppDataPath() / "config/";
    error_code ec;
    fs::create_directory( paramsDir, ec );
    return paramsDir / "FileMapping.txt";
}

//-----------------------------------------------------------------------------
fs::path GetSymbolsFileName()
{
    fs::path paramsDir = GetAppDataPath() / "config/";
    error_code ec;
    fs::create_directory( paramsDir, ec );
    return paramsDir / "Symbols.txt";
}

//-----------------------------------------------------------------------------
fs::path GetLicenseName()
{
    fs::path appDataDir = GetAppDataPath();
    error_code ec;
    fs::create_directory( appDataDir, ec );
    return  appDataDir / "user.txt";
}

//-----------------------------------------------------------------------------
fs::path GetCachePath()
{
    fs::path cacheDir = GetAppDataPath() / "cache/";
    error_code ec;
    fs::create_directory( cacheDir, ec );
    return cacheDir;
}

//-----------------------------------------------------------------------------
fs::path GetPresetPath()
{
    fs::path presetDir = GetAppDataPath() / "presets/";
    error_code ec;
    fs::create_directory( presetDir, ec );
    return presetDir;
}

//-----------------------------------------------------------------------------
fs::path GetPluginPath()
{
    fs::path presetDir = GetAppDataPath() / "plugins/";
    error_code ec;
    fs::create_directory( presetDir, ec );
    return presetDir;
}

//-----------------------------------------------------------------------------
fs::path GetCapturePath()
{
    fs::path captureDir = GetAppDataPath() / "output/";
    error_code ec;
    fs::create_directory( captureDir, ec );
    return captureDir;
}

//-----------------------------------------------------------------------------
fs::path GetDumpPath()
{
    fs::path dumpsDir = GetAppDataPath() / "dumps/";
    error_code ec;
    fs::create_directory( dumpsDir, ec );
    return dumpsDir;
}

//-----------------------------------------------------------------------------
fs::path GetTmpPath()
{
    fs::path tmpDir = GetAppDataPath() / "temp/";
    error_code ec;
    fs::create_directory( tmpDir, ec );
    return tmpDir;
}

//-----------------------------------------------------------------------------
fs::path StripExtension( const fs::path & a_FullName )
{
    return a_FullName.parent_path() / a_FullName.stem();
}

//-----------------------------------------------------------------------------
fs::path GetAppDataPath()
{
    fs::path appData = GetEnvVar( "APPDATA" ); // XXX: fallback?
    fs::path path = appData / "OrbitProfiler";
    error_code ec;
    fs::create_directory( path, ec );
    return path;
}

//-----------------------------------------------------------------------------
bool IsSourceFile( const fs::path& a_File )
{
    fs::path ext = a_File.extension();
    return ext == ".c" || ext == ".cpp" || ext == ".h" || ext == ".hpp" || ext == ".inl" || ext == ".cxx";
}

//-----------------------------------------------------------------------------
vector<fs::path> ListFiles( const fs::path & a_Dir, function< bool(const fs::path &)> a_Filter )
{
    vector<fs::path> files;

    for( auto it = fs::recursive_directory_iterator( a_Dir );
        it != fs::recursive_directory_iterator(); ++it )
    {
        const auto& file = it->path();

        if( !is_directory( file ) && a_Filter( file ) )
        {
            files.push_back( file );
        }
    }

    return files;
}

//-----------------------------------------------------------------------------
vector<fs::path> ListFiles( const fs::path & a_Dir, const fs::path & a_Filter )
{
    return ListFiles( a_Dir, [&]( const fs::path & a_Name ){ 
        return Contains( a_Name, a_Filter ); 
    } );
}

}

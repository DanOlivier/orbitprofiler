//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>
#if _WIN32||_WIN64
#include <filesystem>
#else
#include <experimental/filesystem>
#endif
#include <functional>

namespace Path
{
    //using path = std::experimental::filesystem::v1::path;
    namespace fs = std::experimental::filesystem;
    
    fs::path GetExecutableName();
    fs::path GetExecutablePath();
    fs::path GetBasePath();
    fs::path GetOrbitAppPdb( bool a_Is64Bit );
    fs::path GetDllPath( bool a_Is64Bit );
    fs::path GetDllName( bool a_Is64Bit );
    fs::path GetParamsFileName();
    fs::path GetFileMappingFileName();
    fs::path GetSymbolsFileName();
    fs::path GetLicenseName();
    fs::path GetCachePath();
    fs::path GetPresetPath();
    fs::path GetPluginPath();
    fs::path GetCapturePath();
    fs::path GetDumpPath();
    fs::path GetTmpPath();
    fs::path GetFileName( const fs::path& a_FullName );
    fs::path GetFileNameNoExt( const fs::path& a_FullName );
    fs::path StripExtension( const fs::path& a_FullName );
    fs::path GetExtension( const fs::path& a_FullName );
    fs::path GetDirectory( const fs::path& a_FullName );
    fs::path GetProgramFilesPath();
    fs::path GetAppDataPath();
    fs::path GetMainDrive();

    bool FileExists( const fs::path& a_File );
    bool DirExists( const fs::path& a_Dir );
    bool IsSourceFile( const fs::path& a_File );
    //bool IsPackaged() { return m_IsPackaged; }

    std::vector<fs::path> ListFiles( const fs::path& a_Dir, std::function< bool(const fs::path&)> a_Filter = [](const fs::path&){ return true; });
    std::vector<fs::path> ListFiles( const fs::path& a_Dir, const fs::path& a_Filter );
}

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
    
    static fs::path GetExecutableName();
    static fs::path GetExecutablePath();
    static fs::path GetBasePath();
    static fs::path GetOrbitAppPdb( bool a_Is64Bit );
    static fs::path GetDllPath( bool a_Is64Bit );
    static fs::path GetDllName( bool a_Is64Bit );
    static fs::path GetParamsFileName();
    static fs::path GetFileMappingFileName();
    static fs::path GetSymbolsFileName();
    static fs::path GetLicenseName();
    static fs::path GetCachePath();
    static fs::path GetPresetPath();
    static fs::path GetPluginPath();
    static fs::path GetCapturePath();
    static fs::path GetDumpPath();
    static fs::path GetTmpPath();
    static fs::path GetFileName( const fs::path& a_FullName );
    static fs::path GetFileNameNoExt( const fs::path& a_FullName );
    static fs::path StripExtension( const fs::path& a_FullName );
    static fs::path GetExtension( const fs::path& a_FullName );
    static fs::path GetDirectory( const fs::path& a_FullName );
    static fs::path GetProgramFilesPath();
    static fs::path GetAppDataPath();
    static fs::path GetMainDrive();

    static bool FileExists( const fs::path& a_File );
    static bool DirExists( const fs::path& a_Dir );
    static bool IsSourceFile( const fs::path& a_File );
    //static bool IsPackaged() { return m_IsPackaged; }

    static std::vector<fs::path> ListFiles( const fs::path& a_Dir, std::function< bool(const fs::path&)> a_Filter = [](const fs::path&){ return true; });
    static std::vector<fs::path> ListFiles( const fs::path& a_Dir, const fs::path& a_Filter );
}

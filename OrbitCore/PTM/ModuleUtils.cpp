//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "PTM/ModuleUtils.h"
#include "PTM/OrbitModule.h"

#include "PrintVar.h"
#include "Path.h"
#include "Log.h"

#include <magic.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <set>

using namespace std;
namespace fs = std::experimental::filesystem;

namespace ModuleUtils
{

class MagicDB
{
public:
    magic_t cookie = 0;
    MagicDB() {
        cookie = ::magic_open(MAGIC_NO_CHECK_ELF);
        magic_load(cookie, NULL);
    }
    ~MagicDB() { magic_close(cookie); }
};


//-----------------------------------------------------------------------------
// faulty read_symlink when dealing with pseudo-files in /proc (truncated files)
fs::path my_read_symlink(const fs::path& p, error_code& ec) 
{
    // struct ::stat st;
    // if (::lstat(p.c_str(), &st)) 
    // {
    //     ec.assign(errno, generic_category());
    //     return {};
    // }
    string buf(PATH_MAX, '\0');
    ssize_t len = ::readlink(p.c_str(), &buf.front(), buf.size());
    if (len == -1) 
    {
        ec.assign(errno, generic_category());
        return {};
    }
    ec.clear();
    return fs::path{buf.data(), buf.data() + len};
}

//-----------------------------------------------------------------------------
ModuleMap_t ListModules( DWORD pid )
{
    SCOPE_TIMER_LOG( L"ModuleUtils::ListModules" );

    static MagicDB magicDB;

    // Alternatively, we could cat /proc/$pid/maps, and filter out all but the 
    // executable segments (and filter out [vdso] and [vsyscall])
    // cat /proc/8606/maps | grep 'r\-xp' | tr -s ' ' | cut -d' ' -f6
    fs::path map_files = Format("/proc/%ld/map_files", pid);
    ModuleMap_t o_ModuleMap;
    
    struct AddressRange { long long start, end; };
    map<fs::path, AddressRange> moduleAddresses;
    static set<fs::path> inexistentPaths;
    
    fs::file_status s = status(map_files);
    printf("%o\n",s.permissions());

    if(0 != access(map_files.c_str(), R_OK))
    {
        printf("Error: don't have access to %s\n", map_files.c_str());
        return o_ModuleMap;
    }
    for(auto& p: fs::directory_iterator(map_files))
    {
        long long start, end;
        if(2 != sscanf(p.path().filename().string().c_str(), "%llx-%llx", &start, &end))
        {
            // XXX: develop better error handling
            throw std::runtime_error(Format("unable to parse: %s").c_str());
        }

        error_code ec;
        fs::path linkp = my_read_symlink(p, ec);
        if(ec)
        {
            printf("Warning: Error reading link: %s\n", linkp.string().c_str());
            continue;
        }
        if(linkp.empty()) continue;
        if(!fs::exists(linkp) && inexistentPaths.find(linkp) == inexistentPaths.end())
        {
            inexistentPaths.insert(linkp);
            printf("Warning: %s doesn't exist!\n", linkp.string().c_str());
            continue;
        }
        /*printf("%llx-%llx -> %s\n", 
            //p.path().filename().string().c_str(), 
            start, end,
            linkp.string().c_str());*/
        
        // This will include all mapped segments in the final range
        // whereas perhaps only the executable (code) segment is required
        const auto& key = moduleAddresses.find(linkp);
        if(key != moduleAddresses.end())
        {
            auto& range = key->second;
            range.start = min(start, range.start);
            range.end = max(start, range.start);
        }
        else
            moduleAddresses[linkp] = AddressRange{start, end};
    }

    for(auto& m: moduleAddresses)
    {
        auto& fullpath = m.first;
        auto& range = m.second;

        // Filter-out anything that isn't recognized as an ELF
        const string filetype = magic_file(magicDB.cookie, fullpath.string().c_str());
        const string ELF = "ELF";
        bool isELF = (filetype.compare(0, ELF.length(), ELF) == 0);
        if(!isELF)
        {
            string msg = Format("%llx-%llx -> %s", 
                range.start, range.end,
                fullpath.string().c_str());
            if(!isELF) msg += Format(" (%s)", filetype.c_str());
            msg += "\n";
            //printf("%s", msg.c_str());
            continue;
        }
        shared_ptr<Module> module = make_shared<Module>();
        module->m_Name = fullpath.filename();
        module->m_FullName = fullpath;
        module->m_Directory = fullpath.parent_path();
        module->m_AddressStart = range.start;
        module->m_AddressEnd = range.end;

        if( module->m_AddressStart == 0 )
        {
            throw std::runtime_error(Format("Unexpected: modules %s starts at address 0", 
                fullpath.string().c_str()).c_str());
        }
        o_ModuleMap[module->m_AddressStart] = module;
    }

    return o_ModuleMap;
}

}
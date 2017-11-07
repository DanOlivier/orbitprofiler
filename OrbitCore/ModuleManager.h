//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <memory>
#include <vector>
#include <functional>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

class Pdb;
struct Module;
class Message;

class ModuleManager
{
public:
    ModuleManager();
    ~ModuleManager();

    void Init();
    void OnReceiveMessage( const Message & a_Msg );
    void LoadPdbAsync(const std::shared_ptr<Module> & a_Module, std::function<void()> a_CompletionCallback);
    void LoadPdbAsync(const std::vector<fs::path> a_Modules, std::function<void()> a_CompletionCallback);
    
protected:
    void DequeueAndLoad();
    void OnPdbLoaded();
    void ApplyPresets( std::shared_ptr<Pdb> & a_Pdb );
    void AddPdb( const std::shared_ptr<Pdb> & a_Pdb );

protected:
    std::function<void()> m_UserCompletionCallback;
    std::vector<fs::path> m_ModulesQueue;
};
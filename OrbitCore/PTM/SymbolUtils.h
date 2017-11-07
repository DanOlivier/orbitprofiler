//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "BaseTypes.h"

#include <map>
#include <memory>

class Module;

//-----------------------------------------------------------------------------
namespace SymUtils
{
    typedef std::map< DWORD64, std::shared_ptr<Module> > ModuleMap_t;
    ModuleMap_t ListModules( DWORD pid );
};

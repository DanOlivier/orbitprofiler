//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <ostream>

namespace llvm { namespace pdb {
    class PDBSymbol;
    class IPDBSession;
}}

namespace OrbitDia
{
    void DiaDump( std::unique_ptr<llvm::pdb::PDBSymbol> a_Symbol );
    void DiaDump( unsigned long a_SymbolId );
    void DiaDump( std::unique_ptr<llvm::pdb::PDBSymbol> Symbol, std::ostream &OS, int Indent );
}
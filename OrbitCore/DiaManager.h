//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>

namespace llvm { namespace pdb {
    class PDBSymbol;
    class IPDBSession;
}}

class DiaManager
{
public:
    DiaManager();
    ~DiaManager();

    void Init();
    void DeInit();
    bool InitDataSource();
    bool LoadDataFromPdb( const fs::path& a_FileName, 
        std::unique_ptr<llvm::pdb::IPDBSession>& a_Session, 
        std::unique_ptr<llvm::pdb::PDBSymbol>& a_GlobalSymbol );

    static void InitMsDiaDll();

private:
    //IDiaDataSource* m_DiaDataSource;
};
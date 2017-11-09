#pragma once

#include <llvm/DebugInfo/CodeView/CodeView.h>
#include "BaseTypes.h"

#include <cstring>
#include <vector>

//-----------------------------------------------------------------------------
struct Argument
{
    Argument() { std::memset(this, 0, sizeof(*this)); }
    DWORD      m_Index;
    llvm::codeview::CallingConvention  m_Reg;
    DWORD      m_Offset;
    DWORD      m_NumBytes;
};

//-----------------------------------------------------------------------------
struct FunctionArgInfo
{
    FunctionArgInfo() : m_NumStackBytes(0), m_ArgDataSize(0) {}
    int m_NumStackBytes;
    int m_ArgDataSize;
    std::vector< Argument > m_Args;
};
//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------


#include "CallStackDataView.h"
#include "Callstack.h"
#include "Capture.h"
#include "PTM/OrbitProcess.h"
#include "SamplingProfiler.h"
#include "Pdb.h"
#include "App.h"

using namespace std;

//----------------------------------------------------------------------------
CallStackDataView::CallStackDataView() : m_CallStack( nullptr )
{
    
}

//-----------------------------------------------------------------------------
void CallStackDataView::SetAsMainInstance()
{
    GOrbitApp->RegisterCallStackDataView(this);
}

//-----------------------------------------------------------------------------
int CallStackDataView::GetNumElements()
{
    return (int)m_Indices.size();
}

//-----------------------------------------------------------------------------
void CallStackDataView::OnDataChanged()
{
    size_t numFunctions = m_CallStack ? m_CallStack->m_Depth : 0;
    m_Indices.resize(numFunctions);
    for( int i = 0; i < numFunctions; ++i )
    {
        m_Indices[i] = i;
    }
}

//-----------------------------------------------------------------------------
wstring CallStackDataView::GetValue(int a_Row, int a_Column)
{
    if (a_Row >= GetNumElements())
    {
        return L"";
    }

    Function & function = GetFunction(a_Row);

    wstring value;

    switch (s_HeaderMap[a_Column])
    {
    case Function::INDEX:
        value = Format(L"%d", a_Row); break;
    case Function::SELECTED:
        value = function.IsSelected() ? L"X" : L"-"; break;
    case Function::NAME:
        value = function.PrettyName(); break;
    case Function::ADDRESS:
        value = Format(L"0x%llx", function.GetVirtualAddress()); break;
    case Function::FILE:
        value = function.m_File; break;
    case Function::MODULE:
        value = function.GetModuleName().wstring(); break;
    case Function::LINE:
        value = Format(L"%i", function.m_Line); break;
    case Function::SIZE:
        value = Format(L"%lu", function.m_Size); break;
    case Function::CALL_CONV:
        value = Function::GetCallingConventionString((uint8_t)function.m_CallConv); break;
    default: break;
    }

    return value;
}

//-----------------------------------------------------------------------------
void CallStackDataView::OnFilter( const wstring & a_Filter )
{
    if( !m_CallStack )
        return;
    
    vector<int> indices;
    vector< wstring > tokens = Tokenize( ToLower( a_Filter ) );
     
    for( int i = 0; i < (int)m_CallStack->m_Depth; ++i )
    {
        const Function & function = GetFunction(i);
        wstring name = ToLower( function.m_PrettyName );
        bool match = true;

        for( wstring & filterToken : tokens )
        {
            if( !( name.find( filterToken ) != wstring::npos/* ||
                   file.find( filterToken ) != string::npos*/ ) )
            {
                match = false;
                break;
            }
        }

        if( match )
        {
            indices.push_back(i);
        }
    }

    m_Indices = indices;
}

//-----------------------------------------------------------------------------
Function & CallStackDataView::GetFunction( unsigned int a_Row )
{
    static Function dummy;

    if( m_CallStack )
    {
        if( (int)a_Row < m_CallStack->m_Depth )
        {
            ScopeLock lock(GCapture->m_TargetProcess->GetDataMutex());

            DWORD64 addr = m_CallStack->m_Data[a_Row];
            Function * func = GCapture->m_TargetProcess->GetFunctionFromAddress(addr, false);
            
            if( func )
            {
                return *func;
            }
            else if( GCapture->m_SamplingProfiler )
            {
                dummy.m_PrettyName = GCapture->m_SamplingProfiler->GetSymbolFromAddress( addr );
                dummy.m_Address = addr;
                return dummy;
            }
        }
    }

    dummy.m_PrettyName = L"";
    return dummy;
}
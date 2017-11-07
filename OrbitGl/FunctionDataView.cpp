//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------


#include "FunctionDataView.h"
#include "OrbitType.h"
#include "PTM/OrbitProcess.h"
#include "Capture.h"
#include "Log.h"
#include "App.h"
#include "Pdb.h"
#include "RuleEditor.h"

using namespace std;

//-----------------------------------------------------------------------------
FunctionsDataView::FunctionsDataView()
{
    GOrbitApp->RegisterFunctionsDataView(this);
    m_SortingToggles.resize(Function::NUM_EXPOSED_MEMBERS, false);
    m_SortingToggles[Function::SELECTED] = true;
}

//-----------------------------------------------------------------------------
vector<int>   FunctionsDataView::s_HeaderMap;
vector<float> FunctionsDataView::s_HeaderRatios;

//-----------------------------------------------------------------------------
const vector<wstring>& FunctionsDataView::GetColumnHeaders()
{
    static vector<wstring> Columns;

    if( s_HeaderMap.size() == 0 )
    {
        Columns.push_back(L"Hooked");   s_HeaderMap.push_back(Function::SELECTED);  s_HeaderRatios.push_back(0);
        Columns.push_back(L"Index");    s_HeaderMap.push_back(Function::INDEX);     s_HeaderRatios.push_back(0);
        Columns.push_back(L"Function"); s_HeaderMap.push_back(Function::NAME);      s_HeaderRatios.push_back(0.5f);
        Columns.push_back(L"Size");     s_HeaderMap.push_back(Function::SIZE);      s_HeaderRatios.push_back(0);
        Columns.push_back(L"File");     s_HeaderMap.push_back(Function::FILE);      s_HeaderRatios.push_back(0);
        Columns.push_back(L"Line");     s_HeaderMap.push_back(Function::LINE);      s_HeaderRatios.push_back(0);
        Columns.push_back(L"Module");   s_HeaderMap.push_back(Function::MODULE);    s_HeaderRatios.push_back(0);
        Columns.push_back(L"Address");  s_HeaderMap.push_back(Function::ADDRESS);   s_HeaderRatios.push_back(0);
        Columns.push_back(L"Conv");     s_HeaderMap.push_back(Function::CALL_CONV); s_HeaderRatios.push_back(0);
    }

    return Columns;
}

//-----------------------------------------------------------------------------
const vector<float>& FunctionsDataView::GetColumnHeadersRatios()
{
    return s_HeaderRatios;
}

//-----------------------------------------------------------------------------
wstring FunctionsDataView::GetValue( int a_Row, int a_Column )
{
    ScopeLock lock( GCapture->m_TargetProcess->GetDataMutex() );

    if( a_Row >= GetNumElements() )
    {
        return L"";
    }

    Function & function = GetFunction( a_Row );

    wstring value;

    switch ( s_HeaderMap[a_Column] )
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
        value = function.m_Pdb ? function.m_Pdb->GetName().wstring() : L""; break;
    case Function::LINE:
        value = Format( L"%i", function.m_Line ); break;
    case Function::SIZE:
        value = Format( L"%lu", function.m_Size ); break;
    case Function::CALL_CONV:
        value = Function::GetCallingConventionString((uint8_t)function.m_CallConv); break;
    default: break;
    }

    return value;
}

//-----------------------------------------------------------------------------
#define ORBIT_FUNC_SORT(Member)\
    [&](int a, int b) { \
        return OrbitUtils::Compare(functions[a]->Member, functions[b]->Member, ascending); \
    }

//-----------------------------------------------------------------------------
void FunctionsDataView::OnSort( int a_Column, bool a_Toggle )
{
    if (!SortAllowed())
    {
        return;
    }

    const vector<Function*> & functions = GCapture->m_TargetProcess->GetFunctions();
    auto MemberID = Function::MemberID( s_HeaderMap[a_Column] );

    if (a_Toggle)
    {
        m_SortingToggles[MemberID] = !m_SortingToggles[MemberID];
    }

    bool ascending = m_SortingToggles[MemberID];
    function<bool(int a, int b)> sorter = nullptr;

    switch (MemberID)
    {
    case Function::NAME:     sorter = ORBIT_FUNC_SORT( m_PrettyName );     break;
    case Function::ADDRESS:  sorter = ORBIT_FUNC_SORT( m_Address );        break;
    case Function::MODULE:   sorter = ORBIT_FUNC_SORT( m_Pdb->GetName() ); break;
    case Function::FILE:     sorter = ORBIT_FUNC_SORT( m_File );           break;
    case Function::LINE:     sorter = ORBIT_FUNC_SORT( m_Line );           break;
    case Function::SIZE:     sorter = ORBIT_FUNC_SORT( m_Size );           break;
    case Function::SELECTED: sorter = ORBIT_FUNC_SORT( IsSelected() );       break;
    case Function::CALL_CONV:sorter = ORBIT_FUNC_SORT( m_CallConv );       break;
    case Function::INDEX:
    case Function::NUM_EXPOSED_MEMBERS:
        break;
    }

    if( sorter ) 
    {
        sort(m_Indices.begin(), m_Indices.end(), sorter);
    }

    m_LastSortedColumn = a_Column;
}

//-----------------------------------------------------------------------------
enum TypesContextMenu
{
    FUN_SELECT,
    FUN_UNSELECT,
    FUNC_MENU_VIEW,
    FUNC_GO_TO_DISASSEMBLY,
    FUNC_CREATE_RULE,
    FUNC_SET_AS_FRAME
};

//-----------------------------------------------------------------------------
const vector<wstring>& FunctionsDataView::GetContextMenu(int a_Index)
{
    static vector<wstring> Menu = { L"Hook", L"Unhook", L"Visualize", L"Go To Disassembly", L"Create Rule"/*, "Set as Main Frame"*/ };
    return Menu;
}

//-----------------------------------------------------------------------------
void FunctionsDataView::OnContextMenu( int a_MenuIndex, vector<int> & a_ItemIndices )
{
    switch (a_MenuIndex)
    {
        case FUN_SELECT:
            for (int i : a_ItemIndices)
            {
                Function & func = GetFunction(i);
                func.Select();
            }
            break;
        case FUN_UNSELECT:
            for( int i : a_ItemIndices )
            {
                Function & func = GetFunction( i );
                func.UnSelect();
            }
            break;
        case FUNC_MENU_VIEW: 
            for( int i : a_ItemIndices )
            {
                GetFunction(i).Print();
            }

            GOrbitApp->SendToUiNow(L"output");
            break;
        case FUNC_GO_TO_DISASSEMBLY:
            for( int i : a_ItemIndices )
            {
                Function & func = GetFunction( i );
                func.GetDisassembly();
            }
            break;
        case FUNC_CREATE_RULE:
            for( int i : a_ItemIndices )
            {
                Function & func = GetFunction( i );
                GOrbitApp->LaunchRuleEditor(&func);
                break;
            }
            break;
        case FUNC_SET_AS_FRAME:
            for( int i : a_ItemIndices )
            {
                GetFunction( i ).SetAsMainFrameFunction();
                break;
            }
            break;
        default: break;
    }
}

//-----------------------------------------------------------------------------
void FunctionsDataView::OnFilter( const wstring & a_Filter )
{
    m_FilterTokens = Tokenize( ToLower( a_Filter ) );

    ParallelFilter();
    
    if( m_LastSortedColumn != -1 )
    {
        OnSort(m_LastSortedColumn, false);
    }
}

//-----------------------------------------------------------------------------
void FunctionsDataView::ParallelFilter()
{
    vector<Function*> & functions = GCapture->m_TargetProcess->GetFunctions();
    const auto prio = oqpi::task_priority::normal;
    auto numWorkers = oqpi_tk::scheduler().workersCount( prio );
    //int numWorkers = oqpi::thread::hardware_concurrency();
    vector< vector<int> > indicesArray;
    indicesArray.resize( numWorkers );

    oqpi_tk::parallel_for( "FunctionsDataViewParallelFor", (int)functions.size(), [&]( int32_t a_BlockIndex, int32_t a_ElementIndex )
    {
        vector<int> & result = indicesArray[a_BlockIndex];
        const wstring & name = functions[a_ElementIndex]->Lower();

        for( wstring & filterToken : m_FilterTokens )
        {
            if( name.find( filterToken ) == wstring::npos )
            {
                return;
            }
        }
        
        result.push_back( a_ElementIndex );
    } );
    
    set< int > indicesSet;
    for( vector<int> & results : indicesArray )
    {
        for( int index : results )
        {
            indicesSet.insert( index );
        }
    }

    m_Indices.clear();
    for( int i : indicesSet )
    {
        m_Indices.push_back( i );
    }
}

//-----------------------------------------------------------------------------
void FunctionsDataView::OnDataChanged()
{
    ScopeLock lock( GCapture->m_TargetProcess->GetDataMutex() );

    size_t numFunctions = GCapture->m_TargetProcess->GetFunctions().size();
    m_Indices.resize(numFunctions);
    for (int i = 0; i < numFunctions; ++i)
    {
        m_Indices[i] = i;
    }

    if( m_LastSortedColumn != -1 )
    {
        OnSort( m_LastSortedColumn, false );
    }
}

//-----------------------------------------------------------------------------
Function & FunctionsDataView::GetFunction( unsigned int a_Row )
{
    vector<Function*> & functions = GCapture->m_TargetProcess->GetFunctions();
    return *functions[m_Indices[a_Row]];
}

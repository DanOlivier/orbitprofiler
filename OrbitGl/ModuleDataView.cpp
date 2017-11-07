//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------


#include "ModuleDataView.h"
#include "PTM/OrbitModule.h"
#include "PTM/OrbitProcess.h"
#include "App.h"

using namespace std;

//-----------------------------------------------------------------------------
ModulesDataView::ModulesDataView()
{
    m_SortingToggles.resize(MDV_NumColumns, false);

    GOrbitApp->RegisterModulesDataView(this);
}

//-----------------------------------------------------------------------------
vector<float> ModulesDataView::s_HeaderRatios;

//-----------------------------------------------------------------------------
const vector<wstring>& ModulesDataView::GetColumnHeaders()
{
    static vector<wstring> Columns;
    if( Columns.size() == 0 )
    { 
        Columns.push_back(L"Index");         s_HeaderRatios.push_back(0);
        Columns.push_back(L"Name");          s_HeaderRatios.push_back(0.2f);
        Columns.push_back(L"Path");          s_HeaderRatios.push_back(0.3f);
        Columns.push_back(L"Address Range"); s_HeaderRatios.push_back(0.15f);
        Columns.push_back(L"Debug info");    s_HeaderRatios.push_back(0);
        Columns.push_back(L"Pdb Size");      s_HeaderRatios.push_back(0);
        Columns.push_back(L"Loaded");        s_HeaderRatios.push_back(0);
    }
    return Columns;
}

//-----------------------------------------------------------------------------
const vector<float>& ModulesDataView::GetColumnHeadersRatios()
{
    return s_HeaderRatios;
}

//-----------------------------------------------------------------------------
wstring ModulesDataView::GetValue( int row, int col )
{
    const shared_ptr<Module> & module = GetModule(row);
    wstring value;
    
    switch (col)
    {
    case MDV_Index:
        value = to_wstring((long)row); break;
    case MDV_ModuleName:
        value = module->m_Name.wstring(); break;
    case MDV_Path:
        value = module->m_FullName.wstring(); break;
    case MDV_AddressRange:
        value = module->m_AddressRange; break;
    case MDV_HasPdb:
        value = module->m_FoundPdb ? L"*" : L""; break;
    case MDV_PdbSize:
        value = module->m_FoundPdb ? GetPrettySize( module->m_PdbSize ) : L""; break;
    case MDV_Loaded:
        value = module->m_Loaded ? L"*" : L""; break;
    default:
        break;
    }

    return value;
}

//-----------------------------------------------------------------------------
#define ORBIT_PROC_SORT(Member) \
    [&](int a, int b) { \
        return OrbitUtils::Compare(m_Modules[a]->Member, m_Modules[b]->Member, ascending); \
    }

//-----------------------------------------------------------------------------
void ModulesDataView::OnSort(int a_Column, bool a_Toggle)
{
    MdvColumn mdvColumn = MdvColumn(a_Column);

    if (a_Toggle)
    {
        m_SortingToggles[mdvColumn] = !m_SortingToggles[mdvColumn];
    }

    bool ascending = m_SortingToggles[mdvColumn];
    function<bool(int a, int b)> sorter = nullptr;

    switch (mdvColumn)
    {
    case MDV_ModuleName:    sorter = ORBIT_PROC_SORT(m_Name);         break;
    case MDV_Path:          sorter = ORBIT_PROC_SORT(m_FullName);     break;
    case MDV_AddressRange:  sorter = ORBIT_PROC_SORT(m_AddressStart); break;
    //case MDV_HasPdb:        sorter = ORBIT_PROC_SORT(m_FoundPdb);     break;
    //case MDV_PdbSize:       sorter = ORBIT_PROC_SORT(m_PdbSize);      break;
    case MDV_Loaded:        sorter = ORBIT_PROC_SORT(m_Loaded);       break;
    case MDV_Index:
    case MDV_NumColumns:
        break;
    }

    if (sorter)
    {
        sort(m_Indices.begin(), m_Indices.end(), sorter);
    }

    m_LastSortedColumn = a_Column;
}

//-----------------------------------------------------------------------------
enum ModulesContextMenuIDs
{
    MODULES_LOAD,
    FIND_PDB
};

//-----------------------------------------------------------------------------
const vector<wstring>& ModulesDataView::GetContextMenu( int a_Index )
{
    static vector<wstring> Menu = { L"Load PDB" };
    static vector<wstring> dllMenu = { L"Load dll exports", L"Find pdb" };
    static vector<wstring> EmptyMenu;

    shared_ptr<Module> module = GetModule( a_Index );
    if( !module->m_Loaded )
    {
        if( module->m_FoundPdb )
        {
            return Menu;
        }
        else if( module->IsDll() )
        {
            return dllMenu;
        }
        else
        {
            return EmptyMenu;
        }
    }

    return EmptyMenu;
}

//-----------------------------------------------------------------------------
void ModulesDataView::OnContextMenu( int a_MenuIndex, vector<int> & a_ItemIndices )
{
    switch (a_MenuIndex)
    {
    case MODULES_LOAD: 
    {
        for( int index : a_ItemIndices )
        {
            const shared_ptr<Module> & module = GetModule(index);

            if( module->m_FoundPdb || module->IsDll() )
            {
                Process::ModuleMap_t& processModules = m_Process->GetModules();
                auto it = processModules.find( module->m_AddressStart );
                if( it != processModules.end() )
                {
                    shared_ptr<Module> & module = it->second;

                    if( !module->m_Loaded )
                    {
                        GOrbitApp->EnqueueModuleToLoad( module );
                    }
                }
            }
        }

        GOrbitApp->LoadModules();
        break;
    }
    case FIND_PDB:
    {
        wstring FileName = GOrbitApp->FindFile(L"Find Pdb File", L"", L"*.pdb" );
    }
        break;
    default: break;
    }
}

//-----------------------------------------------------------------------------
void ModulesDataView::OnTimer()
{
}

//-----------------------------------------------------------------------------
void ModulesDataView::OnFilter(const wstring & a_Filter)
{
    vector<int> indices;
    vector< wstring > tokens = Tokenize( ToLower( a_Filter ) );

    for (int i = 0; i < (int)m_Modules.size(); ++i)
    {
        shared_ptr<Module> & module = m_Modules[i];
        wstring name = ToLower( module->GetPrettyName() );

        bool match = true;

        for( wstring & filterToken : tokens )
        {
            if ( name.find(filterToken) == string::npos )
            {
                match = false;
                break;
            }
        }

        if (match)
        {
            indices.push_back(i);
        }
    }

    m_Indices = indices;

    if (m_LastSortedColumn != -1)
    {
        OnSort(m_LastSortedColumn, false);
    }
}

//-----------------------------------------------------------------------------
void ModulesDataView::SetProcess( shared_ptr<Process> a_Process )
{
    m_Modules.clear();
    m_Process = a_Process;
    
    for( auto & it : a_Process->GetModules() )
    {
        it.second->GetPrettyName();
        m_Modules.push_back( it.second );
    }
    
    int numModules = (int)m_Modules.size();
    m_Indices.resize(numModules);
    for (int i = 0; i < numModules; ++i)
    {
        m_Indices[i] = i;
    }

    OnSort(MDV_PdbSize, false);
}

//-----------------------------------------------------------------------------
const shared_ptr<Module> & ModulesDataView::GetModule( unsigned int a_Row ) const
{
    return m_Modules[m_Indices[a_Row]];
}

//-----------------------------------------------------------------------------
bool ModulesDataView::GetDisplayColor( int a_Row, int a_Column, unsigned char& r, unsigned char& g, unsigned char& b )
{
    if ( GetModule(a_Row)->m_Loaded )
    {
        static unsigned char R = 42;
        static unsigned char G = 218;
        static unsigned char B = 130;
        r = R;
        g = G;
        b = B;
        return true;
    }
    else if (GetModule(a_Row)->m_FoundPdb)
    {
        static unsigned char R = 42;
        static unsigned char G = 130;
        static unsigned char B = 218;
        r = R;
        g = G;
        b = B;
        return true;
    }

    return false;
}

//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------


#include "ThreadDataViewGl.h"
#include "ThreadView.h"
#include "Capture.h"
#include "PTM/OrbitProcess.h"

using namespace std;

//-----------------------------------------------------------------------------
ThreadDataViewGl::ThreadDataViewGl()
{

}

//-----------------------------------------------------------------------------
const vector<wstring>& ThreadDataViewGl::GetColumnHeaders()
{
    static vector< wstring > columns = { L"ThreadId", L"History", L"Usage" };
    return columns;
}

//-----------------------------------------------------------------------------
void ThreadDataViewGl::OnSort( int a_Column, bool a_Toggle )
{
    switch ( (ColumnType)a_Column )
    {
    case ColumnType::THREAD_ID:
        GCapture->m_TargetProcess->SortThreadsById();
        break;
    case ColumnType::HISTORY:
    case ColumnType::USAGE:
        GCapture->m_TargetProcess->SortThreadsByUsage();
        break;
    }
}

//-----------------------------------------------------------------------------
void ThreadDataViewGl::SetGlPanel( class GlPanel* a_GlPanel )
{
    if( a_GlPanel->GetType() == GlPanel::THREADS )
    {
        m_ThreadView = static_cast<ThreadView*>( a_GlPanel );
    }
}

//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "OrbitType.h"
#include "DataViewModel.h"

//-----------------------------------------------------------------------------
class LiveFunctionsDataView : public DataViewModel
{
public:
    LiveFunctionsDataView();

    virtual const std::vector<std::wstring>& GetColumnHeaders() override;
    virtual const std::vector<float>& GetColumnHeadersRatios() override;
    virtual const std::vector<std::wstring>& GetContextMenu( int a_Index ) override;
    virtual std::wstring GetValue( int a_Row, int a_Column ) override;

    void OnFilter( const std::wstring & a_Filter ) override;
    void OnSort( int a_Column, bool a_Toggle = true ) override;
    void OnContextMenu( int a_Index, std::vector<int> & a_ItemIndices ) override;
    void OnDataChanged() override;
    virtual void OnTimer() override;

protected:
    Function & GetFunction( unsigned int a_Row ) const;

    static std::vector<int>   s_HeaderMap;
    static std::vector<float> s_HeaderRatios;
    std::vector<Function*>    m_Functions;
};


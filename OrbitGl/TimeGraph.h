//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once


#include "TextBox.h"
#include "BlockChain.h"
#include "ContextSwitch.h"
#include "EventTracer.h"
#include "TimeGraphLayout.h"
#include "Geometry.h"
#include "Batcher.h"
#include "TextRenderer.h"
#include "MemoryTracker.h"

#include <unordered_map>

class TimeGraph
{
public:
    TimeGraph(GlCanvas*, TextRenderer*, PickingManager*);

    void Draw( bool a_Picking = false );
    void DrawMainFrame( TextBox & a_Box );
    void DrawIdentifiers();
    void DrawEvents( bool a_Picking = false );
    void DrawTime();
    void UpdateThreadIds();
    void GetBounds( Vec2 & a_Min, Vec2 & a_Max );
    
    void DrawLineBuffer( bool a_Picking );
    void DrawBoxBuffer( bool a_Picking );
    void DrawBuffered( bool a_Picking );
    void DrawText();
    void NeedsUpdate();
    void UpdatePrimitives( bool a_Picking );
    void UpdateEvents();
    void SelectEvents( float a_WorldStart, float a_WorldEnd, ThreadID a_TID );

    void ProcessTimer( Timer & a_Timer );
    void UpdateThreadDepth( int a_ThreadId, int a_Depth );
    void UpdateMaxTimeStamp( IntervalType a_Time );
    void AddContextSwitch();
    
    int GetThreadDepth( int a_ThreadId ) const;
    int GetThreadDepthTotal() const;
    float GetThreadTotalHeight();
    float GetTextBoxHeight() const { return m_Layout.m_TextBoxHeight; }
    float GetWorldFromRawTimeStamp( IntervalType a_Time ) const;
    float GetWorldFromUs( double a_Micros ) const;
    IntervalType GetRawTimeStampFromWorld( float a_WorldX );
    IntervalType GetRawTimeStampFromUs( double a_MicroSeconds ) const;
    void GetWorldMinMax( float & a_Min, float & a_Max ) const;
    bool UpdateSessionMinMaxCounter();

    static Color GetThreadColor( int a_ThreadId );
    
    void Clear();
    void ZoomAll();
    void ZoomTime( float a_ZoomValue, double a_MouseRatio );
    void SetMinMax( double a_MinEpochTime, double a_MaxEpochTime );
    void PanTime( int a_InitialX, int a_CurrentX, int a_Width, double a_InitialTime );
    double GetTime( double a_Ratio );
    double GetTimeIntervalMicro( double a_Ratio );
    void Select( const Vec2 & a_WorldStart, const Vec2 a_WorldStop );
    double GetSessionTimeSpanUs();
    double GetCurrentTimeSpanUs();
    void NeedsRedraw(){ m_NeedsRedraw = true; }

    bool IsVisible( const Timer & a_Timer );
    int GetNumDrawnTextBoxes(){ return m_NumDrawnTextBoxes; }
    void AddTextBox( const TextBox& a_TextBox );
    void AddContextSwitch( const ContextSwitch & a_CS );
    //void SetPickingManager( class PickingManager* a_Manager ){ m_PickingManager = a_Manager; }
    //void SetCanvas( GlCanvas* a_Canvas );
    
private:
    TextRenderer                    m_TextRendererStatic;
    TextRenderer*                   m_TextRenderer = nullptr;
    GlCanvas*                       m_Canvas = nullptr;
    TextBox                         m_SceneBox;
public:
    BlockChain<TextBox, 65536>      m_TextBoxes;
private:
    int                             m_NumDrawnTextBoxes = 0;
    
    double                          m_RefEpochTimeUs = 0;
public:
    double                          m_MinEpochTimeUs = 0;
    double                          m_MaxEpochTimeUs = 0;
private:
    IntervalType                    m_SessionMinCounter;// = _I64_MAX;
    IntervalType                    m_SessionMaxCounter;// = _I64_MIN;
    std::map< ThreadID, int >       m_ThreadDepths;
    std::map< ThreadID, uint32_t >  m_EventCount;
    double                          m_TimeWindowUs = 0;
    float                           m_WorldStartX = 0;
    float                           m_WorldWidth = 0;

    unsigned int                    m_MainFrameCounter = 0;
    unsigned char                   m_TrackAlpha = 255;

public:
    TimeGraphLayout                 m_Layout;
private:
    std::map< ThreadID, class EventTrack* > m_EventTracks;

    std::map< DWORD/*ThreadId*/, std::map< long long, ContextSwitch > > m_ContextSwitchesMap;
    std::map< DWORD/*CoreId*/, std::map< long long, ContextSwitch > >   m_CoreUtilizationMap;

    std::map< ThreadID, uint32_t >  m_ThreadCountMap;

    std::vector< CallstackEvent >   m_SelectedCallstackEvents;
    bool                            m_NeedsUpdatePrimitives = false;
public:
    bool                            m_DrawText = true;
    bool                            m_NeedsRedraw = false;
private:
    std::vector<TextBox*>           m_VisibleTextBoxes;
public:
    Batcher                         m_Batcher;
private:
    PickingManager*                 m_PickingManager;
    Mutex                           m_Mutex;
    Timer                           m_LastThreadReorder;
public:
    MemoryTracker                   m_MemTracker;
};

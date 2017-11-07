//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "GlCanvas.h"
#include "GlSlider.h"

struct ContextSwitch;

class CaptureWindow : public GlCanvas
{
public:
    CaptureWindow();
    virtual ~CaptureWindow();
    
    void DoZoom();
    void ZoomAll();
    
    void UpdateWheelMomentum( float a_DeltaTime ) override;
    void MouseMoved(int a_X, int a_Y, bool a_Left, bool a_Right, bool a_Middle) override;
    void LeftDown(int a_X, int a_Y) override;
    void LeftUp() override;
    void Pick( int a_X, int a_Y );
    void Pick( PickingID a_ID, int a_X, int a_Y  );
    void Hover( int a_X, int a_Y );
    void FindCode( DWORD64 address );
    void RightDown(int a_X, int a_Y) override;
    void RightUp() override;
    void MiddleDown( int a_X, int a_Y ) override;
    void MiddleUp( int a_X, int a_Y ) override;
    void MouseWheelMoved( int a_X, int a_Y, int a_Delta, bool a_Ctrl ) override;
    void KeyPressed( unsigned int a_KeyCode, bool a_Ctrl, bool a_Shift, bool a_Alt ) override;
    void OnTimer() override;
    void Draw() override;
    void DrawScreenSpace() override;
    void DrawStatus();
    void RenderUI() override;
    void RenderText() override;
    void PreRender() override;
    void PostRender() override;
    void Resize( int a_Width, int a_Height ) override;
    void RenderHelpUi();
    void RenderMemTracker();
    void RenderBar();
    void RenderTimeBar();
    void OnTimerAdded( Timer & a_Timer );
    void OnContextSwitchAdded( const ContextSwitch & a_CS );
    void ResetHoverTimer();
    void SelectTextBox( class TextBox* a_TextBox );
    void OnDrag( float a_Ratio );
    void NeedsUpdate();
    void ToggleSampling();
    void ClearCaptureData();
    void OnCaptureStarted();
    float GetTopBarTextY();

public:
    TimeGraph       m_TimeGraph;
private:
    OutputWindow    m_StatsWindow;
    Timer           m_HoverTimer;
    std::wstring    m_ToolTip;
    int             m_HoverDelayMs = 300;
    bool            m_IsHovering = false;
    bool            m_CanHover = false;
    bool            m_DrawHelp = false;
    bool            m_DrawMemTracker = false;
    bool            m_FirstHelpDraw = true;
    bool            m_DrawStats = false;
    GlSlider        m_Slider;
    int             m_ProcessX = 0;
};


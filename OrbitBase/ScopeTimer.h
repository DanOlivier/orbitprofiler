//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "Profiling.h"
#include <string>

#define SCOPE_TIMER_LOG( msg ) LocalScopeTimer timer(msg)

#ifndef thread_local
# if __STDC_VERSION__ >= 201112 && !defined __STDC_NO_THREADS__
#  define thread_local _Thread_local
# elif defined _WIN32 && ( \
       defined _MSC_VER || \
       defined __ICL || \
       defined __DMC__ || \
       defined __BORLANDC__ )
#  define thread_local __declspec(thread) 
/* note that ICC (linux) and Clang are covered by __GNUC__ */
# elif defined __GNUC__ || \
       defined __SUNPRO_C || \
       defined __xlC__
#  define thread_local __thread
# else
#  error "Cannot define thread_local"
# endif
#endif

extern thread_local int CurrentDepth;

//-----------------------------------------------------------------------------
class Timer
{
public:
    Timer() : m_TID(0)
            , m_Depth(0)
            , m_SessionID(-1)
            , m_Type(NONE)
            , m_Processor(-1)
            , m_CallstackHash(0)
            , m_FunctionAddress(0)
    {
        m_UserData[0] = 0;
        m_UserData[1] = 0;
    }

    void Start();
    void Stop();
    void Reset(){ Stop(); Start(); }

    inline double ElapsedMillis() const;
    inline double ElapsedSeconds() const;

    inline double QueryMillis()  { m_PerfCounter.stop(); return ElapsedMillis(); }
    inline double QuerySeconds() { m_PerfCounter.stop(); return ElapsedSeconds(); }

    static inline int GetCurrentDepthTLS() { return CurrentDepth; }
    static inline void ClearThreadDepthTLS() { CurrentDepth = 0; }

    static const int Version = 0;

    enum Type : uint8_t
    {
        NONE,
        CORE_ACTIVITY,
        THREAD_ACTIVITY,
        HIGHLIGHT,
        UNREAL_OBJECT,
        ZONE,
        ALLOC,
        FREE
    };

    Type GetType() const { return m_Type; }
    void SetType( Type a_Type ){ m_Type = a_Type; }
    bool IsType( Type a_Type ) const { return m_Type == a_Type; }

public:

    // Needs to have to exact same layout in win32/x64, debug/release
    int         m_TID;
    int8_t      m_Depth;
    int8_t      m_SessionID;
    Type        m_Type;
    int8_t      m_Processor;
    DWORD64     m_CallstackHash;
    DWORD64     m_FunctionAddress;
    DWORD64     m_UserData[2];
    PerfCounter m_PerfCounter;
};

//-----------------------------------------------------------------------------
class SimpleTimer
{
public:
    SimpleTimer() : m_IsRunning(false){}

    void Start() { m_PerfCounter.start(); m_IsRunning = true; }
    void Stop()  { m_PerfCounter.stop(); m_IsRunning = false; }

    inline bool IsRunning() const { return m_IsRunning; }
    inline double ElapsedMillis();
    inline double ElapsedSeconds();

    inline double QueryMillis()  { if( m_IsRunning ) { m_PerfCounter.stop(); } return ElapsedMillis(); }
    inline double QuerySeconds() { if( m_IsRunning ) { m_PerfCounter.stop(); } return ElapsedSeconds(); }

public:
    PerfCounter m_PerfCounter;
    bool            m_IsRunning;
};

//-----------------------------------------------------------------------------
class ScopeTimer
{
public:
    ScopeTimer(){}
    ScopeTimer( const char* a_Name );
    ~ScopeTimer();

protected:
    Timer m_Timer;
};

//-----------------------------------------------------------------------------
class LocalScopeTimer
{
public:
    LocalScopeTimer();
    LocalScopeTimer( const std::wstring & a_Message );
    LocalScopeTimer( double* a_Millis );
    ~LocalScopeTimer();
protected:
    Timer        m_Timer;
    double*      m_Millis;
    std::wstring m_Msg;
};

//-----------------------------------------------------------------------------
class ConditionalScopeTimer
{
public:
    ConditionalScopeTimer() : m_Active(false) {}
    ~ConditionalScopeTimer();
    void Start( const char* a_Name );

protected:
    enum { NameSize = 64 };

    Timer m_Timer;
    bool      m_Active;
    char      m_Name[NameSize];
};

//-----------------------------------------------------------------------------
inline double Timer::ElapsedMillis() const
{
    IntervalType elapsedMicros = PerfCounter::get_microseconds(m_PerfCounter.get_start(), m_PerfCounter.get_end());
    return (double)elapsedMicros * 0.001;
}

//-----------------------------------------------------------------------------
inline double Timer::ElapsedSeconds() const
{
    IntervalType elapsedMicros = PerfCounter::get_microseconds(m_PerfCounter.get_start(), m_PerfCounter.get_end());
    return (double)elapsedMicros * 0.000001;
}

//-----------------------------------------------------------------------------
inline double SimpleTimer::ElapsedMillis()
{
    IntervalType elapsedMicros = PerfCounter::get_microseconds(m_PerfCounter.get_start(), m_PerfCounter.get_end());
    return (double)elapsedMicros * 0.001;
}

//-----------------------------------------------------------------------------
inline double SimpleTimer::ElapsedSeconds()
{
    IntervalType elapsedMicros = PerfCounter::get_microseconds(m_PerfCounter.get_start(), m_PerfCounter.get_end());
    return (double)elapsedMicros * 0.000001;
}


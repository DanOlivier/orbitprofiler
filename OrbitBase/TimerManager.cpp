//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "TimerManager.h"
#include "Threading.h"
#include "Message.h"
#include "TcpForward.h"
//#include "Params.h"

#if _WIN32||_WIN64
#include <direct.h>
#endif

using namespace std;

//-----------------------------------------------------------------------------
TimerManager::TimerManager()
    : m_IsRecording(false)
    , m_ExitRequested(false)
    , m_FlushRequested(false)
    , m_LockFreeQueue(65534)
    , m_NumQueuedEntries(0)
    , m_NumQueuedMessages(0)
    , m_NumQueuedTimers(0)
    , m_NumTimersFromPreviousSession(0)
    , m_NumFlushedTimers(0)
{
}

//-----------------------------------------------------------------------------
TimerManager::~TimerManager()
{
}

//-----------------------------------------------------------------------------
void TimerManager::StartRecording()
{
    if( m_IsRecording )
    {
        return;
    }
    
    m_IsRecording = true;
}

//-----------------------------------------------------------------------------
void TimerManager::StopRecording()
{
    m_IsRecording = false;
    FlushQueue();
}

//-----------------------------------------------------------------------------
void TimerManager::FlushQueue()
{
    m_FlushRequested = true;

    const size_t numTimers = 4096;
    Timer Timers[numTimers];
    m_NumFlushedTimers = 0;

    while (!m_ExitRequested)
    {
        size_t numDequeued = m_LockFreeQueue.try_dequeue_bulk(Timers, numTimers);

        if (numDequeued == 0)
            break;

        m_NumQueuedEntries -= (int)numDequeued;
        m_NumFlushedTimers += (int)numDequeued;
    }

    m_FlushRequested = false;
    m_ConditionVariable.signal();
}

//-----------------------------------------------------------------------------
void TimerManager::Stop()
{
    m_IsRecording = false;
    m_ExitRequested = true;
    m_ConditionVariable.signal();

    if (m_ConsumerThread)
    {
        m_ConsumerThread->join();
    }
}

//-----------------------------------------------------------------------------
void TimerManager::Add( const Timer& a_Timer )
{
    if( m_IsRecording )
    {
        m_LockFreeQueue.enqueue(a_Timer);
        m_ConditionVariable.signal();
        ++m_NumQueuedEntries;
        ++m_NumQueuedTimers;
    }
}

//-----------------------------------------------------------------------------
void TimerManager::Add( const Message& a_Message )
{
    if( m_IsRecording )
    {
        m_LockFreeMessageQueue.enqueue(a_Message);
        m_ConditionVariable.signal();
        ++m_NumQueuedEntries;
        ++m_NumQueuedMessages;
    }
}

//-----------------------------------------------------------------------------
void TimerManager::Add( const ContextSwitch & a_CS )
{
    m_ContextSwitchAddedCallback( a_CS );
}
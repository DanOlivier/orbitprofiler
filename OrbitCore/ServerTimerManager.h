#pragma once
#include "TimerManager.h"

class ServerTimerManager : public TimerManager
{
public:
    ServerTimerManager() : TimerManager() {}

    //-----------------------------------------------------------------------------
    void StartRecording() override
    {
        if (m_IsRecording)
        {
            return;
        }

        //CreateDataBase();
        if (!m_ConsumerThread)
        {
            m_ConsumerThread = new std::thread([&]() { ConsumeTimers(); });
        }

        m_IsRecording = true;
    }

    //-----------------------------------------------------------------------------
    void ConsumeTimers()
    {
        SetThreadName(pthread_self(), "OrbitConsumeTimers");
        //SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

        Timer Timer;

        while (!m_ExitRequested)
        {
            m_ConditionVariable.wait();

            while (!m_ExitRequested && !m_FlushRequested && m_LockFreeQueue.try_dequeue(Timer))
            {
                --m_NumQueuedEntries;
                --m_NumQueuedTimers;

                if (Timer.m_SessionID == Message::GSessionID)
                {
                    for (TimerAddedCallback & Callback : m_TimerAddedCallbacks)
                    {
                        Callback(Timer);
                    }
                }
                else
                {
                    ++m_NumTimersFromPreviousSession;
                }
            }
        }
    }
};

extern ServerTimerManager* GTimerManager;


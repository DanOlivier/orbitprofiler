#pragma once

#include "TimerManager.h"

class ClientTimerManager : public TimerManager
{
public:
    ClientTimerManager() : TimerManager()
    {
        m_ConsumerThread = new std::thread([&]() { SendTimers(); });
    }

    //-----------------------------------------------------------------------------
    void StartClient()
    {
        m_IsRecording = true;
    }

    //-----------------------------------------------------------------------------
    void StopClient()
    {
        m_IsRecording = false;
        FlushQueue();

        if( GTcpClient )
        {
            GTcpClient->FlushSendQueue();
        }
    }

    //-----------------------------------------------------------------------------
    void FlushQueue() override
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

            int numEntries = m_NumFlushedTimers;
            GTcpClient->Send( Msg_NumFlushedEntries, numEntries );
        }

        m_FlushRequested = false;
        m_ConditionVariable.signal();
    }

    //-----------------------------------------------------------------------------
    void SendTimers()
    {
        SetThreadName(pthread_self(), "OrbitSendTimers");

        const size_t numTimers = 4096;
        Timer Timers[numTimers];

        while (!m_ExitRequested)
        {
            Message Msg(Msg_Timer);

            // Wait for non-empty queue
            while (m_NumQueuedEntries <= 0 && !m_ExitRequested)
            {
                m_ConditionVariable.wait();
            }

            size_t numDequeued = m_LockFreeQueue.try_dequeue_bulk(Timers, numTimers);
            m_NumQueuedEntries -= (int)numDequeued;
            m_NumQueuedTimers -= (int)numDequeued;
            Msg.m_Size = (int)numDequeued * sizeof(Timer);

            GTcpClient->Send(Msg, (void*)Timers);

            int numEntries = m_NumQueuedEntries;
            GTcpClient->Send(Msg_NumQueuedEntries, numEntries);

            while (m_LockFreeMessageQueue.try_dequeue(Msg) && !m_ExitRequested)
            {
                --m_NumQueuedEntries;
                --m_NumQueuedMessages;
                GTcpClient->Send(Msg);
            }
        }
    }
};

extern ClientTimerManager* GTimerManager;

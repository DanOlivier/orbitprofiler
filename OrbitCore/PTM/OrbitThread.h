//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

//#include "ScopeTimer.h"
#include "RingBuffer.h"

#include <memory>

//#include <proc/readproc.h>
struct proc_t;
extern "C" void freeproc(proc_t* p);

struct ProcDeleter {
    void operator()(proc_t* b) { freeproc(b); }
};
typedef std::unique_ptr<proc_t, ProcDeleter> ProcHandle_t;

typedef unsigned long ThreadID;

//-----------------------------------------------------------------------------
class Thread
{
public:
    Thread(ThreadID a_ID, ProcHandle_t handle) :
        m_TID(a_ID), 
        m_Handle(std::move(handle))
    {
        m_Usage.Fill(0.f);
    }

    float GetUsage();
    void  UpdateUsage();

//private:
    ThreadID m_TID = 0;
    ProcHandle_t m_Handle;
    RingBuffer<float, 32> m_Usage;
};


//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "OrbitThread.h"
#include "Utils.h"

#include <proc/readproc.h>
#include <proc/sysinfo.h> // uptime

//-----------------------------------------------------------------------------
void Thread::UpdateUsage()
{
    m_Usage.Add( GetUsage() );
}

//-----------------------------------------------------------------------------
float Thread::GetUsage()
{
    static double uptime_sav;
    double uptime_cur;
    uptime(&uptime_cur, NULL);
    float ellapsedTime = uptime_cur - uptime_sav;
    if (ellapsedTime < 0.01) ellapsedTime = 0.005;
    uptime_sav = uptime_cur;

    long Hertz = sysconf(_SC_CLK_TCK);

    unsigned long long total_time = m_Handle->utime + m_Handle->stime;
    //total_time += (m_Handle->cutime + m_Handle->cstime);
    unsigned int seconds = uptime_cur - (double(m_Handle->start_time) / Hertz);
    double cpuUsage = 100 * ((double(total_time) / Hertz) / seconds);
    return cpuUsage;
}

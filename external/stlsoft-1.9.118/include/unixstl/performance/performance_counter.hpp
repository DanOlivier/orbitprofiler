#pragma once

# include <sys/time.h>
# include <stdint.h>

namespace unixstl
{

class performance_counter
{
public:
    typedef struct timeval      epoch_type;
    typedef int64_t             interval_type;
    typedef performance_counter class_type;

public:
    void    start();
    void    stop();
    void    restart();
public:
    static epoch_type       get_epoch();

    static interval_type    get_seconds(epoch_type start, epoch_type end);
    static interval_type    get_milliseconds(epoch_type start, epoch_type end);
    static interval_type    get_microseconds(epoch_type start, epoch_type end);

    interval_type   get_period_count() const;
    interval_type   get_seconds() const;
    interval_type   get_milliseconds() const;
    interval_type   get_microseconds() const;

    interval_type   stop_get_period_count_and_restart();
    interval_type   stop_get_seconds_and_restart();
    interval_type   stop_get_milliseconds_and_restart();
    interval_type   stop_get_microseconds_and_restart();

    epoch_type  get_start() const; //{ return m_start; }   // start of measurement period
    epoch_type  get_end() const; //  { return m_end; }     // End of measurement period
    void set_start( epoch_type a_Start ); // { m_start = a_Start; }
    void set_end( epoch_type a_End ); // { m_end = a_End; }

    static interval_type get_period_count_from_microseconds( interval_type micros );

private:
    static void measure_(epoch_type &epoch);
private:
    epoch_type  m_start;
    epoch_type  m_end;
};

inline /* static */ void performance_counter::measure_(performance_counter::epoch_type &epoch)
{
    ::gettimeofday(&epoch, nullptr);
}

inline void performance_counter::start()
{
    measure_(m_start);

    m_end = m_start;
}

inline void performance_counter::stop()
{
    measure_(m_end);
}

inline void performance_counter::restart()
{
    measure_(m_start);
    m_end = m_start;
}

inline /* static */ performance_counter::epoch_type performance_counter::get_epoch()
{
    epoch_type epoch;

    measure_(epoch);

    return epoch;
}

inline /* static */ performance_counter::interval_type performance_counter::get_seconds(performance_counter::epoch_type start, performance_counter::epoch_type end)
{
    //UNIXSTL_MESSAGE_ASSERT("end before start: stop() must be called after start()", start.tv_sec <= end.tv_sec);

    long    secs    =   end.tv_sec - start.tv_sec;
    long    usecs   =   end.tv_usec - start.tv_usec;

    //UNIXSTL_ASSERT(usecs >= 0 || secs > 0);

    return secs + usecs / (1000 * 1000);
}

inline /* static */ performance_counter::interval_type performance_counter::get_milliseconds(performance_counter::epoch_type start, performance_counter::epoch_type end)
{
    //UNIXSTL_MESSAGE_ASSERT("end before start: stop() must be called after start()", start.tv_sec <= end.tv_sec);

    long    secs    =   end.tv_sec - start.tv_sec;
    long    usecs   =   end.tv_usec - start.tv_usec;

    //UNIXSTL_ASSERT(usecs >= 0 || secs > 0);

    return secs * 1000 + usecs / 1000;
}

inline /* static */ performance_counter::interval_type performance_counter::get_microseconds(performance_counter::epoch_type start, performance_counter::epoch_type end)
{
    //UNIXSTL_MESSAGE_ASSERT("end before start: stop() must be called after start()", start.tv_sec <= end.tv_sec);

    long    secs    =   end.tv_sec - start.tv_sec;
    long    usecs   =   end.tv_usec - start.tv_usec;

    //UNIXSTL_ASSERT(usecs >= 0 || secs > 0);

    return secs * (1000 * 1000) + usecs;
}

inline performance_counter::interval_type performance_counter::get_period_count() const
{
    return get_microseconds();
}

inline performance_counter::interval_type performance_counter::get_seconds() const
{
    //UNIXSTL_MESSAGE_ASSERT("end before start: stop() must be called after start()", m_start.tv_sec <= m_end.tv_sec);

    long    secs    =   m_end.tv_sec - m_start.tv_sec;
    long    usecs   =   m_end.tv_usec - m_start.tv_usec;

    //UNIXSTL_ASSERT(usecs >= 0 || secs > 0);

    return secs + usecs / (1000 * 1000);
}

inline performance_counter::interval_type performance_counter::get_milliseconds() const
{
    //UNIXSTL_MESSAGE_ASSERT("end before start: stop() must be called after start()", m_start.tv_sec <= m_end.tv_sec);

    long    secs    =   m_end.tv_sec - m_start.tv_sec;
    long    usecs   =   m_end.tv_usec - m_start.tv_usec;

    //UNIXSTL_ASSERT(usecs >= 0 || secs > 0);

    return secs * 1000 + usecs / 1000;
}

inline performance_counter::interval_type performance_counter::get_microseconds() const
{
    //UNIXSTL_MESSAGE_ASSERT("end before start: stop() must be called after start()", m_start.tv_sec <= m_end.tv_sec);

    long    secs    =   m_end.tv_sec - m_start.tv_sec;
    long    usecs   =   m_end.tv_usec - m_start.tv_usec;

    //UNIXSTL_ASSERT(usecs >= 0 || secs > 0);

    return secs * (1000 * 1000) + usecs;
}

}

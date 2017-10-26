#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdint.h>

namespace winstl
{
    class performance_counter
    {
    public:
        typedef int64_t                  epoch_type;
        typedef int64_t                  interval_type;
        typedef performance_counter         class_type;
    public:
        static void class_init()
        {
            class_type  instance;

            instance.start();
        }
        static void class_uninit()
        {}


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

        inline epoch_type  get_start() const { return m_start; }   // start of measurement period
        inline epoch_type  get_end() const { return m_end; }     // End of measurement period
        inline void set_start(epoch_type a_Start) { m_start = a_Start; }
        inline void set_end(epoch_type a_End) { m_end = a_End; }

        inline static interval_type get_period_count_from_microseconds(interval_type micros)
        {
            return frequency_()* micros / interval_type(1000000);
        }

    private:
        typedef void(*measure_fn_type)(epoch_type&);

        static interval_type    frequency_();
        static interval_type    query_frequency_();
        static void             qpc_(epoch_type &epoch);
        static void             gtc_(epoch_type &epoch);
        static measure_fn_type  get_measure_fn_();
        static void             measure_(epoch_type &epoch);
    private:
        epoch_type  m_start;    // start of measurement period
        epoch_type  m_end;      // End of measurement period
    };

    inline /* static */ performance_counter::interval_type performance_counter::query_frequency_()
    {
        interval_type   frequency;

        // If no high-performance counter is available ...
        if (!::QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&frequency)) ||
            frequency == 0)
        {
            // ... then set the divisor to be the frequency for GetTickCount(), which is
            // 1000 since it returns intervals in milliseconds.
            frequency = 1000;
        }

        return frequency;
    }

    inline /* static */ performance_counter::interval_type performance_counter::frequency_()
    {
        static interval_type    s_frequency = query_frequency_();
        //WINSTL_ASSERT(0 != s_frequency);
        return s_frequency;
    }

    inline /* static */ void performance_counter::qpc_(epoch_type &epoch)
    {
        static_cast<void>(::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&epoch)));
    }

    inline /* static */ void performance_counter::gtc_(epoch_type &epoch)
    {
        epoch = static_cast<int32_t>(::GetTickCount());
    }

    inline /* static */ performance_counter::measure_fn_type performance_counter::get_measure_fn_()
    {
        measure_fn_type fn;
        epoch_type      frequency;

        if (QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&frequency)))
        {
            fn = qpc_;
        }
        else
        {
            fn = gtc_;
        }

        return fn;
    }

    inline /* static */ void performance_counter::measure_(epoch_type &epoch)
    {
        static measure_fn_type  fn = get_measure_fn_();
        fn(epoch);
    }

    // Operations
    inline void performance_counter::start()
    {
        measure_(m_start);
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
        epoch_type  epoch;
        measure_(epoch);
        return epoch;
    }

    inline /* static */ performance_counter::interval_type performance_counter::get_seconds(performance_counter::epoch_type start, performance_counter::epoch_type end)
    {
        interval_type   period_count = static_cast<interval_type>(end - start);
        return period_count / frequency_();
    }

    inline /* static */ performance_counter::interval_type performance_counter::get_milliseconds(performance_counter::epoch_type start, performance_counter::epoch_type end)
    {
        interval_type   result;
        interval_type   count = static_cast<interval_type>(end - start);

//#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
        if (count < interval_type(0x20C49BA5E353F7))
//#else /* ? STLSOFT_CF_64BIT_INT_SUPPORT */
//        if (count < interval_type(0x20C49B, 0xA5E353F7))
//#endif /* !STLSOFT_CF_64BIT_INT_SUPPORT */
        {
            result = (count * interval_type(1000)) / frequency_();
        }
        else
        {
            result = (count / frequency_()) * interval_type(1000);
        }

        return result;
    }

    inline /* static */ performance_counter::interval_type performance_counter::get_microseconds(performance_counter::epoch_type start, performance_counter::epoch_type end)
    {
        interval_type   result;
        interval_type   count = static_cast<interval_type>(end - start);

//#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
        if (count < interval_type(0x8637BD05AF6))
//#else /* ? STLSOFT_CF_64BIT_INT_SUPPORT */
//        if (count < interval_type(0x863, 0x7BD05AF6))
//#endif /* !STLSOFT_CF_64BIT_INT_SUPPORT */
        {
            result = (count * interval_type(1000000)) / frequency_();
        }
        else
        {
            result = (count / frequency_()) * interval_type(1000000);
        }

        return result;
    }

    inline performance_counter::interval_type performance_counter::get_period_count() const
    {
        return static_cast<interval_type>(m_end - m_start);
    }

    inline performance_counter::interval_type performance_counter::get_seconds() const
    {
        return get_period_count() / frequency_();
    }

    inline performance_counter::interval_type performance_counter::get_milliseconds() const
    {
        interval_type   result;
        interval_type   count = get_period_count();

//#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
        if (count < interval_type(0x20C49BA5E353F7))
//#else /* ? STLSOFT_CF_64BIT_INT_SUPPORT */
//        if (count < interval_type(0x20C49B, 0xA5E353F7))
//#endif /* !STLSOFT_CF_64BIT_INT_SUPPORT */
        {
            result = (count * interval_type(1000)) / frequency_();
        }
        else
        {
            result = (count / frequency_()) * interval_type(1000);
        }

        return result;
    }

    inline performance_counter::interval_type performance_counter::get_microseconds() const
    {
        interval_type   result;
        interval_type   count = get_period_count();

//#ifdef STLSOFT_CF_64BIT_INT_SUPPORT
        if (count < interval_type(0x8637BD05AF6))
//#else /* ? STLSOFT_CF_64BIT_INT_SUPPORT */
//        if (count < interval_type(0x863, 0x7BD05AF6))
//#endif /* !STLSOFT_CF_64BIT_INT_SUPPORT */
        {
            result = (count * interval_type(1000000)) / frequency_();
        }
        else
        {
            result = (count / frequency_()) * interval_type(1000000);
        }

        return result;
    }


    inline performance_counter::interval_type performance_counter::stop_get_period_count_and_restart()
    {
        stop();
        interval_type   interval = get_period_count();
        m_start = m_end;
        return interval;
    }

    inline performance_counter::interval_type performance_counter::stop_get_seconds_and_restart()
    {
        stop();
        interval_type   interval = get_seconds();
        m_start = m_end;
        return interval;
    }

    inline performance_counter::interval_type performance_counter::stop_get_milliseconds_and_restart()
    {
        stop();
        interval_type   interval = get_milliseconds();
        m_start = m_end;
        return interval;
    }

    inline performance_counter::interval_type performance_counter::stop_get_microseconds_and_restart()
    {
        stop();
        interval_type   interval = get_microseconds();
        m_start = m_end;
        return interval;
    }
}

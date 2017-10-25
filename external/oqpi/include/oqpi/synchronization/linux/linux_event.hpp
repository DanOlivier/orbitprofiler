#pragma once

#include "oqpi/platform.hpp"

#include <condition_variable>
#include <mutex>
#include <iostream>

namespace oqpi {

    //----------------------------------------------------------------------------------------------
    // Forward declaration of this platform semaphore implementation
    using event_manual_reset_policy_impl = struct linux_event_manual_reset_policy;
    //----------------------------------------------------------------------------------------------
    template<typename _ResetPolicy> class linux_event;
    //----------------------------------------------------------------------------------------------
    template<typename _ResetPolicy>
    using event_impl = class linux_event<_ResetPolicy>;
    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------
    template<typename _ResetPolicy>
    class linux_event
        : public _ResetPolicy
    {
        mutable std::mutex m;
        mutable std::condition_variable cv;
        bool is_ready = false;
        
    protected:
        explicit linux_event(const std::string &)
        {
        }

        ~linux_event()
        {
        }

    protected:
        // User interface
        void notify()
        {
            std::lock_guard<std::mutex> lk(m);            
            is_ready = true;
            cv.notify_one();
        }

        void wait() const
        {
            std::unique_lock<std::mutex> lk(m);
            while (!is_ready)
            {
                cv.wait(lk, [this]{return is_ready;});
                if (!is_ready)
                    std::cout << "Spurious wake up!\n";
            }
            //t.join();
        }

        void reset()
        {
            _ResetPolicy::reset(m, is_ready);
        }

        template<typename _Rep, typename _Period>
        bool waitFor(const std::chrono::duration<_Rep, _Period>& relTime) const
        {
            std::lock_guard<std::mutex> lk(m);
            while (!is_ready)
            {
                cv.wait_for(lk, relTime);
                if (!is_ready)
                    std::cout << "Spurious wake up!\n";
            }
            //t.join();
        }

    private:
        // Not copyable
        linux_event(const linux_event &)                = delete;
        linux_event& operator =(const linux_event &)    = delete;
    };

    //----------------------------------------------------------------------------------------------
    struct linux_event_manual_reset_policy
    {
        static bool is_manual_reset_enabled()
        {
            return true;
        }

        void reset(std::mutex& m, bool& is_ready)
        {
            std::lock_guard<std::mutex> lk(m);
            is_ready = false;
        }
    };
    //----------------------------------------------------------------------------------------------

} /*oqpi*/

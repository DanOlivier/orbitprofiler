#pragma once

#include "oqpi/platform.hpp"

#include <condition_variable>
#include <mutex>
#include <iostream>

namespace oqpi {

    //----------------------------------------------------------------------------------------------
    // Forward declaration of this platform semaphore implementation
    using semaphore_impl = class linux_semaphore;
    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------
    class linux_semaphore
    {
        mutable std::mutex m;
        mutable std::condition_variable cv;

        mutable long count_ = 0;
        long    maxCount_;

    protected:
        explicit linux_semaphore(int32_t initCount, int32_t maxCount, const std::string &)
            : count_(initCount)
            , maxCount_(maxCount)
        {
        }

        ~linux_semaphore()
        {
        }

    protected:
        // User interface
        void notify(int32_t count)
        {
            std::lock_guard<std::mutex> lk(m);
            count_ += count;
            cv.notify_one();
        }

        bool tryWait()
        {
            std::lock_guard<std::mutex> lk(m);
            if(count_) {
                --count_;
                return true;
            }
            return false;
        }

        void wait() const
        {
            std::unique_lock<std::mutex> lk(m);
            cv.wait(lk, [this]{ return count_ > 0; });
            --count_;
        }

        template<typename _Rep, typename _Period>
        bool waitFor(const std::chrono::duration<_Rep, _Period>& relTime)
        {
            std::lock_guard<std::mutex> lk(m);
            auto finished = cv.wait_for(lk, relTime, [this]{ return count_ > 0; });
            if (finished)
                --count_;

            return finished;
        }

    private:
        // Not copyable
        linux_semaphore(const linux_semaphore &)                = delete;
        linux_semaphore& operator =(const linux_semaphore &)    = delete;
    };
    //----------------------------------------------------------------------------------------------

} /*oqpi*/

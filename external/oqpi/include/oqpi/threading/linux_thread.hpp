#pragma once

#include <chrono>
#include "oqpi/platform.hpp"
#include "oqpi/error_handling.hpp"

#include <thread>
#include <sched.h>

namespace oqpi {

    using thread_impl = class linux_thread;
    class linux_thread
    {
        std::thread t;
    protected:
        //------------------------------------------------------------------------------------------
        static uint32_t hardware_concurrency()
        {
            return std::thread::hardware_concurrency();;
        }

    protected:
        //------------------------------------------------------------------------------------------
        // Default constructible
        linux_thread() noexcept {}

        //------------------------------------------------------------------------------------------
        ~linux_thread() {}

    protected:
        //------------------------------------------------------------------------------------------
        template<typename _Launcher>
        bool create(const thread_attributes &attributes, void *pData)
        {
            // Thread function
            const auto lpStartAddress = [](void* pData) -> int
            {
                auto upLauncher = std::unique_ptr<_Launcher>(static_cast<_Launcher*>(pData));
                if (oqpi_ensure(upLauncher != nullptr))
                {
                    (*upLauncher)();
                }
                return 0;
            };
            
            t = std::thread(lpStartAddress, pData);

            //setPriority(attributes.priority_);
            //setCoreAffinityMask(attributes.coreAffinityMask_);

            return true;
        }

    protected:
        //------------------------------------------------------------------------------------------
        // Platform specific types
        using native_handle_type    = std::thread::native_handle_type;
        using id                    = std::thread::id;

    protected:
        //------------------------------------------------------------------------------------------
        id getId() const { return t.get_id(); }
        native_handle_type getNativeHandle() const { return const_cast<std::thread&>(t).native_handle(); }
        bool joinable() const { return t.joinable(); }

        void join() { t.join(); }
        void detach() { t.detach(); }

        //------------------------------------------------------------------------------------------
        core_affinity getCoreAffinityMask() const;
        void setCoreAffinityMask(core_affinity affinityMask);

        thread_priority getPriority() const;
        void setPriority(thread_priority priority);

        //------------------------------------------------------------------------------------------
        void setName(const char *name)
        {

        }

    public:
        //------------------------------------------------------------------------------------------
        static void set_core_affinity_mask(native_handle_type handle, core_affinity affinityMask);
        static core_affinity get_core_affinity_mask(native_handle_type handle);

        static void set_priority(native_handle_type handle, thread_priority priority);
        static thread_priority get_priority(native_handle_type handle);

        //------------------------------------------------------------------------------------------
        static void set_name(id threadId, const char *name)
        {

        }

        //------------------------------------------------------------------------------------------
        static id get_current_thread_id() {
            auto id = std::this_thread::get_id();
            return id;
        }
        //------------------------------------------------------------------------------------------
    };


    namespace this_thread {

        //------------------------------------------------------------------------------------------
        inline void set_name(const char *name)
        {
            linux_thread::set_name(std::this_thread::get_id(), name);
        }
        //------------------------------------------------------------------------------------------
        template<typename _Rep, typename _Period>
        inline void sleep_for(const std::chrono::duration<_Rep, _Period>& relTime)
        {
            std::this_thread::sleep_for(relTime);
        }
        //------------------------------------------------------------------------------------------
        inline uint32_t get_current_core()
        {
            // Retrieves the number of the processor the current thread was running on during
            // the call to this function.
            return sched_getcpu();
        }
        //------------------------------------------------------------------------------------------
        inline void yield() noexcept
        {
            // Causes the calling thread to yield execution to another thread that is ready to run
            // on the current processor. The operating system selects the next thread to be executed.
            std::this_thread::yield();
        }
        //------------------------------------------------------------------------------------------
        inline void set_priority(thread_priority threadPriority)
        {
            pthread_t h = pthread_self();
            linux_thread::set_priority(h, threadPriority);
        }
        //------------------------------------------------------------------------------------------
        inline void set_affinity_mask(core_affinity coreAffinityMask)
        {
            pthread_t h = pthread_self();
            linux_thread::set_core_affinity_mask(h, coreAffinityMask);
        }
        //------------------------------------------------------------------------------------------
        inline auto get_id()
        {
            return std::this_thread::get_id();
        }
    }
}

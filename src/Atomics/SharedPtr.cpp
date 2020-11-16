#include "gtest/gtest.h"
#include <atomic>
#include <memory>
#include <thread>
#include <condition_variable>
#include "../GlobalVars.h"

TEST(Atomics, shared_ptr)
{
    std::shared_ptr<int> sptr = std::make_shared<int>(2011);

    // When you copy a std::shared_ptr
    // in a thread, all is fine.
    for (int i = 0; i < 10; ++i)
    {
        std::thread([sptr]()
            {
                std::shared_ptr<int> localPtr(sptr);
                localPtr = std::make_shared<int>(2014);
            }).detach();
    }
    EXPECT_EQ(*sptr, 2011);


    // The lambda-function binds the std::shared_ptr ptr by reference. This means, the
    // assignment may become a concurrent readingand writing of the underlying resource;
    // therefore, the program has undefined behaviour.
    /*
    for (int i = 0; i < 10; ++i)
    {
        std::thread([&sptr]() { sptr = std::make_shared<int>(2014); }).detach();
    }
    */
}

TEST(Atomics, DataRaceSharedPtr)
{
    std::shared_ptr<int> sptr = std::make_shared<int>(2011);

    // data-race for shared_ptr, leads to undefined behavior
    /*
    for (int i = 0; i < 10; i++)
    {
        std::thread([&sptr, i]
            {
                auto localPtr = std::make_shared<int>(i);
                std::atomic_store(&sptr, localPtr);

            }).detach();
    }
    */

    EXPECT_EQ(true, true);
}

TEST(Atomics, DataRaceSharedPtrResolved)
{
    std::shared_ptr<int> sptr = std::make_shared<int>(2011);

    bool isdataReady = false;

    for (int i = 0; i < 10; i++)
    {
        std::thread([&sptr, i, &isdataReady]
            {
                auto localPtr = std::make_shared<int>(i);
                std::atomic_store(&sptr, localPtr);

                // notify main thread that data is ready
                // at the last step of for-cycle
                if (i == 9)
                {
                    {
                        std::lock_guard<std::mutex> lck(g_mutex::sharedPtr);
                        isdataReady = true;
                    }
                    g_condition_variable::sharedPtr.notify_one();
                }

            }).detach();
    }

    // check if data is already modified by other threads
    std::unique_lock<std::mutex> lck(g_mutex::sharedPtr);
    g_condition_variable::sharedPtr.wait(lck, [&isdataReady] { return isdataReady; });

    // any of threads can run last, so any of possible i can be set

    EXPECT_EQ(*sptr, 9);
}
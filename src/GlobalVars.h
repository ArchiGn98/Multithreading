#pragma once
#include <mutex>
#include <condition_variable>

namespace g_mutex
{
	extern std::mutex spinlock;
	extern std::mutex atomicConditionVariable;
}

namespace g_condition_variable
{
	extern std::condition_variable atomicConditionVariable;
}

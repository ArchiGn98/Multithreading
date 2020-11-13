#include "GlobalVars.h"

namespace g_mutex
{
	std::mutex spinlock;
	std::mutex atomicConditionVariable;
}

namespace g_condition_variable
{
	std::condition_variable atomicConditionVariable;
}
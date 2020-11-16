#include "GlobalVars.h"

namespace g_mutex
{
	std::mutex spinlock;
	std::mutex atomicConditionVariable;
	std::mutex sharedPtr;
}

namespace g_condition_variable
{
	std::condition_variable atomicConditionVariable;
	std::condition_variable sharedPtr;
}
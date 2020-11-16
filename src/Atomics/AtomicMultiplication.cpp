#include "gtest/gtest.h"
#include <atomic>

template <typename T>
T FetchMultAtomic(std::atomic<T>& shared, T mult)
{
	T OldValue = shared.load();
	while (!shared.compare_exchange_strong(OldValue, OldValue * mult));
	return OldValue;
}

TEST(Atomics, FetchMultAtomic)
{
	std::atomic<int> myInt{ 5 };
	FetchMultAtomic(myInt, 5);
	EXPECT_EQ(myInt, 25);
}
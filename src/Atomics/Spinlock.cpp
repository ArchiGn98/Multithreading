#include "gtest/gtest.h"
#include "../GlobalVars.h"

#include <atomic>
#include <thread>
#include <mutex>
#include <vector>

/*
					Possible Scenarios of thread1 and thread2 and SpinlockAtomicFlag

	1. Thread t gets the lock because the lock invocation was successful.The lock invocation is
	successful if the initial value of the flag in line 11 is false.In this case thread t sets it in an
	atomic operation to true.The value true is the value of the while loop returns to thread t2 if
	it tries to get the lock.So thread t2 is caught in the rat race.Thread t2 cannot set the value of
	the flag to false so that t2 must wait until thread t1 executes the unlock methodand sets the
	flag to false (lines 14 - 16).
	2. Thread t doesn’t get the lock.So we are in scenario 1 with swapped roles.
*/

class SpinlockAtomicFlag
{
	// set to false
	std::atomic_flag flag = ATOMIC_FLAG_INIT;
public:
	void lock()
	{
		// set value back to true and return previous state
		while ( flag.test_and_set() );
	}

	void unlock()
	{
		// set value to false
		flag.clear();
	}
};

SpinlockAtomicFlag spin;


// using our custom lock cpu load reaches to high level if sleap is active
void mergeVectorsSpinlock(std::vector<int>& vec1, const std::vector<int>& vec2)
{
	spin.lock();
	for (const auto& el : vec2)
	{
		vec1.push_back(el);
	}
	//std::this_thread::sleep_for(std::chrono::milliseconds(20000));
	spin.unlock();
}

// using mutex cpu high-load is not observed if sleap is active
void mergeVectorsMutex(std::vector<int>& vec1, const std::vector<int>& vec2)
{
	g_mutex::spinlock.lock();
	for (const auto& el : vec2)
	{
		vec1.push_back(el);
	}
	//std::this_thread::sleep_for(std::chrono::milliseconds(20000));
	g_mutex::spinlock.unlock();
}

void mergeVectors(std::vector<int>& vec1, const std::vector<int>& vec2)
{
	for (const auto& el : vec2)
	{
		vec1.push_back(el);
	}
}

TEST(Atomics, MergeVectorsSpinlockAtomicFlag)
{
	std::vector<int> vec1 { 1, 2, 3, 4 };
	std::vector<int> vec2 { 5, 6, 7, 8 };
	std::vector<int> vec3 { 9, 10, 11, 12 };

	// thread 1 will be first one
	std::vector<int> expected1 { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 }; 

	// thread 2 will be first one
	std::vector<int> expected2 { 1, 2, 3, 4, 9, 10, 11, 12, 5, 6, 7, 8 };

	std::thread t1(mergeVectorsSpinlock, std::ref(vec1), vec2);
	std::thread t2(mergeVectorsSpinlock, std::ref(vec1), vec3);

	t1.join();
	t2.join();

	bool isEqual = (vec1 == expected1) | (vec1 == expected2);

	EXPECT_EQ(true, isEqual);
}

TEST(Atomics, MergeVectorsMutex)
{
	std::vector<int> vec1{ 1, 2, 3, 4 };
	std::vector<int> vec2{ 5, 6, 7, 8 };
	std::vector<int> vec3{ 9, 10, 11, 12 };
	
	// thread 1 will be first one
	std::vector<int> expected1{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };

	// thread 2 will be first one
	std::vector<int> expected2{ 1, 2, 3, 4, 9, 10, 11, 12, 5, 6, 7, 8 };

	std::thread t1(mergeVectorsMutex, std::ref(vec1), vec2);
	std::thread t2(mergeVectorsMutex, std::ref(vec1), vec3);

	t1.join();
	t2.join();

	bool isEqual = (vec1 == expected1) | (vec1 == expected2);

	EXPECT_EQ(true, isEqual);
}

TEST(Atomics, MergeVectors)
{
	std::vector<int> vec1{ 1, 2, 3, 4 };
	std::vector<int> vec2{ 5, 6, 7, 8 };
	std::vector<int> vec3{ 9, 10, 11, 12 };
	
	// thread 1 will be first one
	std::vector<int> expected1{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };

	// thread 2 will be first one
	std::vector<int> expected2{ 1, 2, 3, 4, 9, 10, 11, 12, 5, 6, 7, 8 };

	std::thread t1(mergeVectors, std::ref(vec1), vec2);
	std::thread t2(mergeVectors, std::ref(vec1), vec3);

	t1.join();
	t2.join();
	
	// shared value vec1 is not locked => undefined behavior because of data-raise
	//bool isEqual = (vec1 == expected1) | (vec1 == expected2);
	//EXPECT_EQ(vec1, expected);
}
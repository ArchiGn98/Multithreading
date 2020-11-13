#include "../GlobalVars.h"
#include "gtest/gtest.h"

#include <condition_variable>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

/* 
Prepare and do some work using conditional_variable

• Thread t1
	– prepares the work package mySharedWork = {1, 0, 3}
	– set the non-atomic boolean dataReady to true
	– send its notification condVar.notify_one
• Thread t2
	– waits for the notification condVar.wait(lck, []{ return dataReady; }) while holding
	the lock lck
	– continues its work mySharedWork[1] = 2 after getting the notification
*/
#define _condition_var g_condition_variable::atomicConditionVariable
#define _mutex g_mutex::atomicConditionVariable

void waitingForWork(std::vector<int>& mySharedWork, bool& isdataReady)
{
	// waiting for data until isdataReady == true
	std::unique_lock<std::mutex> lck(_mutex);
	_condition_var.wait(lck, [&isdataReady] { return isdataReady; });

	// do some work
	mySharedWork[1] = 2;
}

void setDataReady(std::vector<int>& mySharedWork, bool& isdataReady)
{
	// Prepare data
	mySharedWork = { 1 , 0, 3 };
	{
		// only one thread can modify "isdataReady"
		std::lock_guard<std::mutex> lck(_mutex);
		isdataReady = true;
	}
	
	// notify other thread that data is prepared
	_condition_var.notify_one();
}

TEST(Atomics, ConditionVariable)
{
	bool isDataReady{ false };
	std::vector<int> data;
	std::vector<int> expected{ 1, 2, 3 };

	std::thread t1(waitingForWork, std::ref(data), std::ref(isDataReady));
	std::thread t2(setDataReady, std::ref(data), std::ref(isDataReady));

	t1.join();
	t2.join();

	EXPECT_EQ(data, expected);
}

/*
Prepare and do some work using std::atomic<bool>

What guarantees that the thread t1 executes mySharedWork[1] = 2
after thread t2 had executed mySharedWork = {1, 0, 3} ?
Now it gets more formal.
	• mySharedWork = {1, 0, 3} happens-before dataReady = true
	• while (!dataReady.load()) happens-before mySharedWork[1] = 2;
	• dataReady = true synchronizes-with while (!dataReady.load())
	• Because synchronizes-with establishes a happens-before relation and happens-before is transitive, 
	  it follows: mySharedWork = {1, 0, 3} happens-before mySharedWork[1] = 2
*/
void waitingForWorkAtomic(std::vector<int>& mySharedWork, std::atomic<bool>& dataReady)
{
	// waiting for data until dataReady == true
	// load() => atomically obtains the value of the atomic object
	while (!dataReady.load())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}

	// do some work
	mySharedWork[1] = 2;
}

void setDataReadyAtomic(std::vector<int>& mySharedWork, std::atomic<bool>& dataReady)
{
	// Prepare data
	mySharedWork = { 1 , 0, 3 };

	// notify other thread that data is prepared
	dataReady = true;
}

TEST(Atomics, ConditionVariableAtomic)
{
	std::atomic<bool> dataReady{ false };
	std::vector<int> data;
	std::vector<int> expected{ 1, 2, 3 };

	std::thread t1(waitingForWorkAtomic, std::ref(data), std::ref(dataReady));
	std::thread t2(setDataReadyAtomic, std::ref(data), std::ref(dataReady));

	t1.join();
	t2.join();

	EXPECT_EQ(data, expected);
}

/*
Difference between std::atomic<bool> and std::condition_variable synchronisation:

The condition variable notifies the waiting thread (push principle) while the atomic boolean
repeatedly asks for the value (pull principle).
*/
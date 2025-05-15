#pragma once	
#include "pch.h"
#include <iostream>

#include <thread>
#include <mutex>
#include <atomic>
#include <windows.h>

mutex m;


#pragma region spinlockExample

class SpinLock1
{
public:

	void lock() {

		bool expected = false;
		bool desired = true;

		while (_locked.compare_exchange_strong(expected, desired) == false) {
			expected = false;
		}
	}

	void unlock() {
		_locked.store(false);
	}

private:
	atomic<bool> _locked = false;
};

class Spinlock2 {


public:
	void lock() {
		while (flag.test_and_set(std::memory_order_acquire)) {
			// busy wait
		}
	}

	void unlock() {
		flag.clear(std::memory_order_release);
	}

private:
	std::atomic_flag flag = ATOMIC_FLAG_INIT;
};

SpinLock1 spinLock;
int32 sum = 0;

void Add() {
	for (int i = 0; i < 1000000; ++i) {
		lock_guard<SpinLock1> lock(spinLock);
		sum++;
	}
}

void Sub() {
	for (int i = 0; i < 1000000; ++i) {
		lock_guard<SpinLock1> lock(spinLock);
		sum--;
	}
}
#pragma endregion

#pragma region eventExample
queue<int32> q;
HANDLE handle;

void Producer() {

	while (true) {

		{
			unique_lock<mutex> lock(m);
			q.push(1);
		}

		::SetEvent(handle);

		this_thread::sleep_for(chrono::milliseconds(100));
	}
}

void Consumer() {

	while (true) {

		::WaitForSingleObject(handle, INFINITE);
		{
			unique_lock<mutex> lock(m);
			if (!q.empty()) {
				q.pop();
			}

		}
	}
}

#pragma endregion


#pragma region thread local storage Example	

thread_local int32 LThreadId = 0;

void ThreadLocalMain(int32 id)
{
	LThreadId = id;
	cout << "Thread ID: " << LThreadId << endl;
}

void ThreadLocalFunc() {
	vector<thread> threads;


	for (int i = 0; i < 10; ++i) {
		threads.push_back(thread(ThreadLocalMain, i));
	}

	for (thread& t : threads) {
		t.join();
	}

}


#pragma endregion
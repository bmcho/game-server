#include "pch.h"
#include <iostream>

#include <thread>
#include <mutex>
#include <atomic>


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

int32 sum = 0;
mutex m;
SpinLock1 spinLock;


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

int main()
{
	thread t1(Add);
	thread t2(Sub);

	t1.join();
	t2.join();


	cout << "sum = " << sum << endl;

}

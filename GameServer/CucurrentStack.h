#pragma once

#include <mutex>

template<typename T>
class LockStack {

public:
	LockStack() {};
	LockStack(const LockStack&) = delete;
	LockStack& operator=(const LockStack&) = delete;

	void Push(T value) {
		lock_guard<mutex> lock(_mutex);
		_stack.push(std::move(value));
		_condVar.notify_one();
	}

	bool TryPop(T& value) {

		lock_guard<mutex> lock(_mutex);
		if (_stack.empty) {
			return false;
		}

		value = std::move(_stack.top());
		_stack.pop();
		return true;
	}

	void WaitPop(T& value) {

		unique_lock<mutex> lock(_mutex);
		_condVar.wait(lock, [] {return _stack.empty() == false;});

		value = std::move(_stack.top());
		_stack.pop();
	}

private:
	stack<T> _stack;
	mutex _mutex;
	condition_variable _condVar;
};


template<typename T>
class LockFreeStack {

	struct Node {

		Node(const T& value) : data(value), next(nullptr) {
		}


		T data;
		Node* next;
	};

public:

	void Push(const T& value) {
		Node* node = new Node(value);
		node->next = _head;
		while (_head.compare_exchange_weak(node->next, node) == false) {
		}
	}

	bool TryPop(T& value) {

		++_popCount;

		Node* oldHead = _head;

		while (oldHead != nullptr && _head.compare_exchange_weak(oldHead, oldHead->next) == false) {

		}

		if (oldHead == nullptr) {
			--_popCount;
			return false;
		}

		value = std::move(oldHead->data);
		TryDelete(oldHead);
		return true;
	}

	void TryDelete(Node* oldHead) {

		if (_Popcount == 1) {

			Node* node = _pendingList.exchange(nullptr);

			if (--_popCount == 0) {
				DeleteNodes(node);
			}
			else {
				ChainPendingNodeList(node);
			}

			delete oldHead;
		}
		else {
			ChainPaendingNodeList(oldHead);
			--_popCount;
		}
	}

	static void DeleteNodes(Node* node) {
		while (node != nullptr) {
			Node* next = node->next;
			delete node;
			node = next;
		}
	}

	void ChainPaendingNodeList(Node* first, Node* last) {

		last->next = _pendingList;

		while (_pendingList.compare_exchange_weak(last->next, first) == false) {
		}
	}

	void ChainPendingNodeList(Node* node) {
		Node* last = node;
		while (last->data != nullptr) {
			last = last->next;
		}
		ChainPaendingNodeList(node, last);
	}

	void ChainPendingNode(Node* node) {
		ChainPaendingNodeList(node, node);
	}

private:
	atomic<Node*> _head;
	atomic<uint32> _popCount = 0;
	atomic<Node*> _pendingList;
};



template<typename T>
class LockFreeStackForSharedPtr {

	struct Node {

		Node(const T& value) : data(make_shared<T>(value)), next(nullptr) {
		}


		shared_ptr<T> data;
		shared_ptr<Node> next;
	};

public:

	void Push(const T& value) {

		shared_ptr<Node> node = make_shared<Node>(value);
		node->next = std::atomic_load(&_head);

		while (std::atomic_compare_exchange_weak(&_head, node->next, node) == false) {
		}

	}

	shared_ptr<T> TryPop() {

		shared_ptr<Node> oldHead = std::atomic_load(&_head);

		while(oldHead != nullptr && std::atomic_compare_exchange_weak(&_head, oldHead, oldHead->next) == false) {
		}

		if (oldHead == nullptr) {
			return shared_ptr<T>();
		}

		return oldHead->data;

	}


private:
	shared_ptr<Node*> _head;
};

/*
 * xqueue.h
 * desc: blocking queue shared by multi-thread
 */

#ifndef _XQUEUE_H
#define _XQUEUE_H

#include <deque>
#include "xlock.h"

// 
template<typename T>
class BlockingQueue {
public:
	BlockingQueue() : mutex_(), notEmpty_(mutex_), queue_() {}
	~BlockingQueue() {}
	void put(const T& x) {
		MutexLockGuard lock(mutex_);
		queue_.push_back(x);
		notEmpty_.notify(); // TODO: move outside of lock
	}
	T take() {
		MutexLockGuard lock(mutex_);
		while(queue_.empty()) { notEmpty_.wait(); } // must be "while"
		T front(queue_.front()); // 
		queue_.pop_front();
		return front;
	}
	int size() const { MutexLockGuard lock(mutex_); return queue_.size(); }
private:
	mutable MutexLock mutex_;
	Condition notEmpty_;
	std::deque<T> queue_;
private:
	BlockingQueue(const BlockingQueue&);
	BlockingQueue& operator=(const BlockingQueue&);
};

#endif // #ifndef _XQUEUE_H

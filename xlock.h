/*
 * xlock.h
 * desc: wapper of mutex and cond_var
 */

#ifndef _XLOCK_H
#define _XLOCK_H

#include <pthread.h>

// 
class MutexLock {
public:
	MutexLock() { pthread_mutex_init(&mutex_, NULL); }
	~MutexLock() { pthread_mutex_destroy(&mutex_); }
	void lock() { pthread_mutex_lock(&mutex_); }
	void unlock() { pthread_mutex_unlock(&mutex_); }
	pthread_mutex_t* handle() { return &mutex_; }
private:
	pthread_mutex_t mutex_;
private:
	MutexLock(const MutexLock&);
	MutexLock& operator=(const MutexLock&);
};

// 
class Condition {
public:
	explicit Condition(MutexLock& mutex) : mutex_(mutex) { pthread_cond_init(&pcond_, NULL); }
	~Condition() { pthread_cond_destroy(&pcond_); }
	void wait() { pthread_cond_wait(&pcond_, mutex_.handle()); }
	void notify() { pthread_cond_signal(&pcond_); }
	void notifyAll() { pthread_cond_broadcast(&pcond_); }
private:
	MutexLock& mutex_;
	pthread_cond_t pcond_;
private:
	Condition(const Condition&);
	Condition& operator=(const Condition&);
};

// 
class MutexLockGuard {
public:
	explicit MutexLockGuard(MutexLock& mutex) : mutex_(mutex) { mutex_.lock(); }
	~MutexLockGuard() { mutex_.unlock(); }
private:
	MutexLock& mutex_;
private:
	MutexLockGuard(const MutexLockGuard&);
	MutexLockGuard& operator=(const MutexLockGuard&);
};

#define MutexLockGuard(x) static_assert(false, "missing mutex guard var name")

#endif // #ifndef _XLOCK_H

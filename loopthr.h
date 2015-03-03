/*
 * loopthr.h
 * desc: 
 */

#ifndef _LOOPTHR_H
#define _LOOPTHR_H

// 
#ifdef WIN32
#define  THREAD_ID  CWinThread* 
#define  THREAD_FUNC_TYPE  UINT 
#define  THREAD_ARG  LPVOID 
#define  THREAD_PREFIX  __cdecl 
typedef  UINT (*THREAD_FUNC_ROUTINE)(LPVOID);
#else
#include <pthread.h>
#define  THREAD_ID  pthread_t 
#define  THREAD_FUNC_TYPE  void* 
#define  THREAD_ARG  void* 
#define  THREAD_PREFIX 
typedef  void* (*THREAD_FUNC_ROUTINE)(void*);
#endif

typedef  void (*LOOP_FUNC)();

// 
class CThread {
public:
	CThread();
	virtual ~CThread();

public:
	void start();
	void stop();
	void restart();
	bool running() const;

protected:
	virtual void run() = 0;
	static void msleep(unsigned int msecs);

private:
	static THREAD_FUNC_TYPE THREAD_PREFIX routine(THREAD_ARG arg);

private:
	THREAD_ID tid;
	bool runflag;

private:
	CThread(const CThread&);
	CThread& operator=(const CThread&);
};


// 
class CLoop {
public:
	CLoop(LOOP_FUNC f = NULL, int d = 1000);
	~CLoop();

public:
	void start(LOOP_FUNC f = NULL, int d = -1);
	void stop();
	void restart();
	bool running() const;
	int count();

private:
	static THREAD_FUNC_TYPE THREAD_PREFIX routine(THREAD_ARG arg);
	static void msleep(unsigned int msecs);

private:
	THREAD_ID tid;
	LOOP_FUNC func;
	int delay;
	bool runflag;
	int runcnt;

private:
	CLoop(const CLoop&);
	CLoop& operator=(const CLoop&);
};

#endif // #ifndef _LOOPTHR_H

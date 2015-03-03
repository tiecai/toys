/*
 * loopthr.cpp
 * desc: 
 */

#include "stdafx.h"
#include "loopthr.h"

#ifdef __unix
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#endif


// 
CThread::CThread()
	: tid(0), runflag(false)
{
}

// 
CThread::~CThread() {
	stop();
}

// 
void CThread::start() {
	if(runflag) {
		return;
	} else {
		runflag = true;
	}

#ifdef WIN32
	tid = AfxBeginThread(CThread::routine, this);
#else
	pthread_attr_t tmpattr;
	pthread_attr_init(&tmpattr);
	pthread_attr_setdetachstate(&tmpattr, PTHREAD_CREATE_DETACHED);
	pthread_create(&tid, &tmpattr, CThread::routine, this);
	pthread_attr_destroy(&tmpattr);
#endif
}

// 
void CThread::stop() {
	if(!runflag) {
		return;
	} else {
		runflag = false;
	}

#ifdef WIN32
	TerminateThread(tid->m_hThread, 0);
#else
	pthread_cancel(tid);
#endif
}

// 
void CThread::restart() {
	stop();
	start();
}

// 
bool CThread::running() const {
	return runflag;
}


// 
void CThread::msleep(unsigned int msecs) {
#ifdef WIN32
	Sleep(msecs);
#else
	if(0 == msecs) {
		sleep(0);
	} else {
		struct timeval tt;
		tt.tv_sec = msecs / 1000;
		tt.tv_usec = (msecs % 1000) * 1000;
		select(0, 0, 0, 0, &tt);
	}
#endif
}

// 
THREAD_FUNC_TYPE THREAD_PREFIX CThread::routine(THREAD_ARG arg) {
	((CThread*) arg)->run();

	((CThread*) arg)->runflag = false;
	return 0;
}



// 
CLoop::CLoop(LOOP_FUNC f, int d)
	: tid(0), func(f), delay(d), runflag(false), runcnt(0)
{
}

// 
CLoop::~CLoop() {
	stop();
}

// 
void CLoop::start(LOOP_FUNC f, int d) {
	if(runflag) {
		return;
	} else {
		runflag = true;
	}

	if(NULL == f) {
		if(NULL == func) {
			runflag = false;
			return;
		}
	} else {
		func = f;
	}
	if(d != -1) {
		delay = d;
	}
	runcnt = 0;

#ifdef WIN32
	tid = AfxBeginThread(CLoop::routine, this);
#else
	pthread_attr_t tmpattr;
	pthread_attr_init(&tmpattr);
	pthread_attr_setdetachstate(&tmpattr, PTHREAD_CREATE_DETACHED);
	pthread_create(&tid, &tmpattr, CLoop::routine, this);
	pthread_attr_destroy(&tmpattr);
#endif
}

// 
void CLoop::stop() {
	if(!runflag) {
		return;
	} else {
		runflag = false;
	}

#ifdef WIN32
	TerminateThread(tid->m_hThread, 0);
#else
	pthread_cancel(tid);
#endif
}

// 
void CLoop::restart() {
	stop();
	start();
}

// 
bool CLoop::running() const {
	return runflag;
}

// 
int CLoop::count() {
	return ++runcnt;
}

// 
void CLoop::msleep(unsigned int msecs) {
#ifdef WIN32
	Sleep(msecs);
#else
	if(0 == msecs) {
		sleep(0);
	} else {
		struct timeval tt;
		tt.tv_sec = msecs / 1000;
		tt.tv_usec = (msecs % 1000) * 1000;
		select(0, 0, 0, 0, &tt);
	}
#endif
}

// 
THREAD_FUNC_TYPE THREAD_PREFIX CLoop::routine(THREAD_ARG arg) {
#ifdef __unix
	int oldtype, oldstate;
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
#endif

	while(1) {
		msleep(((CLoop*) arg)->delay);
		((CLoop*) arg)->runcnt = 0;
		((CLoop*) arg)->func();
	}

	((CLoop*) arg)->runflag = false;
	return 0;
}


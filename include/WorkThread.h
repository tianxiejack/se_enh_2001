/*
 * WorkThread.h
 *
 *  Created on: Sep 21, 2018
 *      Author: ubuntu
 */

#ifndef WORKTHREAD_H_
#define WORKTHREAD_H_
#include<pthread.h>
#include<cstdio>
typedef int            int32;
typedef unsigned int   uint32;
typedef short           int16;
typedef unsigned short  uint16;
typedef char            int8;
typedef unsigned char   uint8;


class WorkThread {
public:
	WorkThread();
	virtual ~WorkThread();
private:
	pthread_t m_threadId;
protected:
	bool m_bRun;
public:
	int RunThread(void* (*ThreadFunc)(void*), void* pArg);
	int IsRunning();
	int SetThreadExit();
	int WaitThreadExit();
};

#endif /* WORKTHREAD_H_ */

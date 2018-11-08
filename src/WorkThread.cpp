/*
 * WorkThread.cpp
 *
 *  Created on: Sep 21, 2018
 *      Author: ubuntu
 */

#include "WorkThread.h"

WorkThread::WorkThread():m_threadId(0), m_bRun(false) {
	// TODO Auto-generated constructor stub
}

WorkThread::~WorkThread() {

	 SetThreadExit();
}

int WorkThread::RunThread(void* (*ThreadFunc)(void*), void* pArg)
{
	int nRet = -1;

	m_bRun = true;
	nRet = pthread_create(&m_threadId, NULL, ThreadFunc, pArg);

	return nRet;
}
int WorkThread::IsRunning()
{
	return m_bRun;
}

int WorkThread::SetThreadExit()
{
	m_bRun = false;

	return 0;
}

int WorkThread::WaitThreadExit()
{
	if (m_threadId)
	{
		pthread_join(m_threadId, NULL);
	}
	return 0;
}

/*
 * sceneProcess.cpp
 *
 *  Created on: 2019年1月4日
 *      Author: jet
 */
#include "sceneProcess.hpp"
#include <stdio.h>

void CSceneProcess::start()
{
	memset(m_cnt,0,sizeof(m_cnt));
	calcErr.x = calcErr.y = 0;
}

void CSceneProcess::detect(const Mat& frame, int chId)
{	
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	unsigned int us = now.tv_sec*1000000+now.tv_nsec/1000;
	*(unsigned int*)frame.data = us;
	bool bInit = false;
	if(m_cnt[chId] == 0)
		m_obj.InitSceneLock(frame);
	else
		m_obj.CalSceneLock(frame);

	if(!m_cnt[chId])
		m_cnt[chId]++;

}

void CSceneProcess::optFlowDetect(const Mat& frame, int chId,cv::Rect &getBound)
{	
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	unsigned int us = now.tv_sec*1000000+now.tv_nsec/1000;
	*(unsigned int*)frame.data = us;
	bool bInit = true;
	if(m_cnt[chId] == 0)
		m_obj.optFlowInitSceneLock(frame);
	else
		bInit = m_obj.optFlowCalcSceneLock(frame,getBound);

	if(!m_cnt[chId])
		m_cnt[chId]++;

	if(!bInit)
	{
		printf("optFlowDetect : lost !!! \ n");
		m_cnt[chId] = 0;
	}
}

void CSceneProcess::optFlowGetResult(cv::Point2f & result)
{	
	calcErr.x = m_obj.m_filterVel.x;
	calcErr.y = m_obj.m_filterVel.y;

	result.x = calcErr.x;
	result.y = calcErr.y;
	
	return ;
}


void CSceneProcess::getResult(cv::Point2f & result)
{	
	calcErr.x += m_obj.m_filterVel.x;
	calcErr.y += m_obj.m_filterVel.y;

	result.x = calcErr.x;
	result.y = calcErr.y;
	
	return ;
}





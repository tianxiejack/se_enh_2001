;/*
 * sceneProcess.cpp
 *
 *  Created on: 2019年1月4日
 *      Author: jet
 */
#include "sceneProcess.hpp"
#include <stdio.h>

bool sceneLost;

CSceneProcess::CSceneProcess():m_lostHandleFlag(false)
{
	MedianFlowTracker::Params parameters;
	parameters.pointsInGrid = 25;
	m_mediaFlowObj = MedianFlowTracker::CreateMedianFlowTrk();
}

CSceneProcess::~CSceneProcess()
{
}

void CSceneProcess::start()
{
	memset(m_cnt,0,sizeof(m_cnt));
	calcErr.x = calcErr.y = 0;
}

void CSceneProcess::detect(const Mat& frame, int chId)
{	
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	unsigned long long us = now.tv_sec*1000000+now.tv_nsec/1000;
	*(unsigned long long*)frame.data = us;
	bool bInit = false;
	if(m_cnt[chId] == 0)
		m_sceneProcObj.InitSceneLock(frame);
	else
		m_sceneProcObj.CalSceneLock(frame);

	if(!m_cnt[chId])
		m_cnt[chId]++;

}

bool CSceneProcess::sceneLockInit(const Mat& frame , Rect2d &getBound)
{	
	bool ret = false;
	ret = m_mediaFlowObj->init( frame, getBound );
	InitRect = getBound;
	return ret;
}

bool CSceneProcess::sceneLockProcess(const Mat& frame , Rect2d &getBound)
{	
	bool ret = false;
	ret = m_mediaFlowObj->update( frame, getBound );

	if(getBound.x < 0 || getBound.x + getBound.width > 1920 || getBound.y < 0 || getBound.y + getBound.height > 1080 )
			m_lostHandleFlag = true;

	if(m_lostHandleFlag)
	{
		secInitRect = getBound;
		//x
		if(getBound.x < 0){
			secInitRect.x = 0;
			secInitRect.width = getBound.x + getBound.width;
		}
		else if(getBound.x + getBound.width > 1920)
			secInitRect.width = 1920 - getBound.x;
		else{
			if( getBound.width <= InitRect.width ){
				if(getBound.x + getBound.width <= InitRect.width){
					secInitRect.x = 0;
					secInitRect.width = getBound.x + getBound.width;
				}
				else if(getBound.x  > 1920 - InitRect.width)
					secInitRect.width = 1920 - secInitRect.x; 				
			}
		}

		//y
		if(getBound.y < 0){
			secInitRect.y = 0;
			secInitRect.height = getBound.y + getBound.height;
		}
		else if(getBound.y + getBound.height > 1080)
			secInitRect.height = 1080 - getBound.x;
		else{
			if( getBound.height <= InitRect.height ){
				if(getBound.y + getBound.height <= InitRect.height){
					secInitRect.y = 0;
					secInitRect.height = getBound.y + getBound.height;
				}
				else if(getBound.y  > 1080 - InitRect.height)
					secInitRect.height = 1080 - secInitRect.y; 
			}
		}

		if( secInitRect.width == InitRect.width && secInitRect.height == InitRect.height )
			m_lostHandleFlag = false;
		
		m_mediaFlowObj->init( frame , secInitRect );			
	}
	return ret;
}

void CSceneProcess::optFlowGetResult(cv::Point2f & result)
{	
	calcErr.x = m_sceneProcObj.m_filterVel.x;
	calcErr.y = m_sceneProcObj.m_filterVel.y;

	result.x = calcErr.x;
	result.y = calcErr.y;
	
	return ;
}


void CSceneProcess::getResult(cv::Point2f & result)
{	
	calcErr.x += m_sceneProcObj.m_filterVel.x;
	calcErr.y += m_sceneProcObj.m_filterVel.y;

	result.x = calcErr.x;
	result.y = calcErr.y;
	
	return ;
}





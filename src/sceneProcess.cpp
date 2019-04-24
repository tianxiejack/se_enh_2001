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
	parameters.pointsInGrid = 20;
	parameters.bZoomHalf = true;
	m_mediaFlowObj = MedianFlowTracker::CreateMedianFlowTrk(parameters);
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

	if(getBound.x < 0 || getBound.x + getBound.width > frame.cols || getBound.y < 0 || getBound.y + getBound.height > frame.rows )
			m_lostHandleFlag = true;

	if(m_lostHandleFlag)
	{
		secInitRect = getBound;
		//x
		if(getBound.x < 0){
			secInitRect.x = 0;
			secInitRect.width = getBound.x + getBound.width;
		}
		else if(getBound.x + getBound.width > frame.cols)
			secInitRect.width = frame.cols - getBound.x;
		else{
			if( getBound.width <= InitRect.width ){
				if(getBound.x + getBound.width <= InitRect.width){
					secInitRect.x = 0;
					secInitRect.width = getBound.x + getBound.width;
				}
				else if(getBound.x  > frame.cols - InitRect.width)
					secInitRect.width = frame.cols - secInitRect.x;
			}
		}

		//y
		if(getBound.y < 0){
			secInitRect.y = 0;
			secInitRect.height = getBound.y + getBound.height;
		}
		else if(getBound.y + getBound.height > frame.rows)
			secInitRect.height = frame.rows - getBound.x;
		else{
			if( getBound.height <= InitRect.height ){
				if(getBound.y + getBound.height <= InitRect.height){
					secInitRect.y = 0;
					secInitRect.height = getBound.y + getBound.height;
				}
				else if(getBound.y  > frame.rows - InitRect.height)
					secInitRect.height = frame.rows - secInitRect.y;
			}
		}

		if( secInitRect.width == InitRect.width && secInitRect.height == InitRect.height )
			m_lostHandleFlag = false;
		
		m_mediaFlowObj->init( frame , secInitRect );	

		if( secInitRect.width < InitRect.width ){
			if( secInitRect.x < 0.001 )
				getBound.x = secInitRect.width - InitRect.width;
			else
				getBound.width = InitRect.width;
		}
		if( secInitRect.height < InitRect.height ){
			if( secInitRect.y < 0.001 )
				getBound.y = secInitRect.height - InitRect.height;
			else
				getBound.height = InitRect.height;
		}	
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





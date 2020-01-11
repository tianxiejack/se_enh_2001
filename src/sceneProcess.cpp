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
}

CSceneProcess::~CSceneProcess()
{
}

void CSceneProcess::start()
{
}

void CSceneProcess::detect(const Mat& frame, int chId)
{}


bool CSceneProcess::sceneLockInit(const Mat& frame , Rect2d &getBound)
{}

bool CSceneProcess::sceneLockProcess(const Mat& frame , Rect2d &getBound)
{}

void CSceneProcess::optFlowGetResult(cv::Point2f & result)
{		
	return ;
}


void CSceneProcess::getResult(cv::Point2f & result)
{		
	return ;
}





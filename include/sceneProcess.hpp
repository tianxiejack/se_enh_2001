/*
 * sceneProcess.hpp
 *
 *  Created on: 2019年1月4日
 *      Author: jet
 */

#ifndef SCENEPROCESS_HPP_
#define SCENEPROCESS_HPP_

#include "sceneProc.hpp"
#include "configtable.h"
#include "MedianFlowTrk.hpp"

using namespace OptFlowTrk;

class CSceneProcess
{
public :
	CSceneProcess();
	virtual	~CSceneProcess();
	
	SceneProc m_sceneProcObj;
	int m_curChId;
	cv::Size m_imgSize[MAX_CHAN];
	unsigned long m_cnt[MAX_CHAN];

	void detect(const Mat& frame, int chId);
	void getResult(cv::Point2f & result);
	void start();

	//void optFlowDetect(const Mat& frame, int chId);
	void optFlowGetResult(cv::Point2f & result);
	
	bool sceneLockInit(const Mat& frame,Rect2d &getBound);
	bool sceneLockProcess(const Mat& frame , Rect2d &getBound);

	cv::Ptr<MedianFlowTracker> m_mediaFlowObj;
private:
	cv::Point2f calcErr;
	Rect2d InitRect,secInitRect;
	bool m_lostHandleFlag;
};



#endif /* SCENEPROCESS_HPP_ */

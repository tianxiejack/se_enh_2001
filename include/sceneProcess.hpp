/*
 * sceneProcess.hpp
 *
 *  Created on: 2019年1月4日
 *      Author: jet
 */

#ifndef SCENEPROCESS_HPP_
#define SCENEPROCESS_HPP_

#include "sceneProc.hpp"
#include "configable.h"

class CSceneProcess
{
public :
	SceneProc m_obj;
	int m_curChId;
	cv::Size m_imgSize[MAX_CHAN];
	unsigned long m_cnt[MAX_CHAN];

	void detect(const Mat& frame, int chId);
	void getResult(cv::Point2f & result);
	void start();

	void optFlowDetect(const Mat& frame, int chId);
	void optFlowGetResult(cv::Point2f & result);


private:
	cv::Point2f calcErr;
};



#endif /* SCENEPROCESS_HPP_ */

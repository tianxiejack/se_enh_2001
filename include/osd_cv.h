/*
 * osd_cv.h
 *
 *  Created on: 2017年5月12日
 *      Author: s123
 */

#ifndef OSD_CV_H_
#define OSD_CV_H_
#include "osa.h"
#include "osa_thr.h"
#include "osa_buf.h"
#include <osa_sem.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/legacy/compat.hpp>
using namespace cv;

typedef struct
{
	int x;
	int y;

}Osd_cvPoint;


typedef struct  _line_param_fb{
	UInt32 enableWin;
	UInt32 objType;        //GRPX_OBJ_ID
	UInt32 frcolor;
	UInt32 bgcolor;

	Int16 x;		        //center position
	Int16 y;		        //center position
	Int16 res0;

	Int16 width;            //graphic width
	Int16 height;           //graphic height
	Int16 res1;

	Int16 linePixels;		//line width
	Int16 lineGapWidth;     //must be <= width
	Int16 lineGapHeight;    //must be <= height
}Line_Param_fb;


void Drawcvcross(Mat frame,Line_Param_fb *lineparm);
 CvScalar  GetcvColour(int colour);
 void DrawcvDashcross(Mat frame,Line_Param_fb *lineparm,int linelength,int dashlength);
 void drawdashlinepri(Mat frame,int startx,int starty,int endx,int endy,int linelength,int dashlength,int colour);
 void DrawcvLine(Mat frame,Osd_cvPoint *start,Osd_cvPoint *end,int frcolor,int linew);
 void Drawcvcrossaim(Mat frame,Line_Param_fb *lineparm);
 void drawcvrect(Mat frame,int x,int y,int width,int height,int frcolor);

#endif /* OSD_CV_H_ */

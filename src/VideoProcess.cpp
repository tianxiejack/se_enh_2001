

#include <glut.h>
#include "VideoProcess.hpp"
#include "vmath.h"
#include "arm_neon.h"

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "app_ctrl.h"
#include "ipc_custom_head.hpp"
#include "osd_cv.h"

typedef Rect_<double> Rect2d;

using namespace vmath;
extern CMD_EXT *msgextInCtrl;
extern CMD_Mtd_Frame Mtd_Frame;
int CVideoProcess::m_mouseEvent = 0;
int CVideoProcess::m_mousex = 0;
int CVideoProcess::m_mousey = 0;
CVideoProcess * CVideoProcess::pThis = NULL;
bool CVideoProcess::m_bTrack = false;
bool CVideoProcess::m_bMtd = false;
bool CVideoProcess::m_bMoveDetect = false;
bool CVideoProcess::m_bSceneTrack = false;
bool CVideoProcess::m_bPatterDetect = false;
int CVideoProcess::m_iTrackStat = 0;
int CVideoProcess::m_iTrackLostCnt = 0;
int CVideoProcess::m_iSceneTrackStat = 0;
int CVideoProcess::m_iSceneTrackLostCnt = 0;
bool CVideoProcess::motionlessflag = false;
int64 CVideoProcess::tstart = 0;
static int count=0;
int ScalerLarge,ScalerMid,ScalerSmall;

int CVideoProcess::MAIN_threadCreate(void)
{
	int iRet = OSA_SOK;
	iRet = OSA_semCreate(&mainProcThrObj.procNotifySem ,1,0) ;
	OSA_assert(iRet == OSA_SOK);


	mainProcThrObj.exitProcThread = false;

	mainProcThrObj.initFlag = true;

	mainProcThrObj.pParent = (void*)this;

	iRet = OSA_thrCreate(&mainProcThrObj.thrHandleProc, mainProcTsk, 0, 0, &mainProcThrObj);

	return iRet;
}


static void copyMat2Mat(cv::Mat src, cv::Mat dst, cv::Point pt)
{
	int i, j;
	uchar *psrc, *pdst;
#pragma UNROLL(4)
	for(i=0; i<src.rows; i++)
	{
		psrc = (uchar*)src.ptr<cv::Vec4b>(i,0);
		pdst =  (uchar*)dst.ptr<cv::Vec4b>(pt.y+i, pt.x);
		memcpy(pdst, psrc, src.cols*sizeof(cv::Vec4b));
	}
}

TARGETBOX mBox[MAX_TARGET_NUMBER];
//static int trktime = 0;


void extractUYVY2Gray(Mat src, Mat dst)
{
	int ImgHeight, ImgWidth,ImgStride;

	ImgWidth = src.cols;
	ImgHeight = src.rows;
	ImgStride = ImgWidth*2;
	uint8_t  *  pDst8_t;
	uint8_t *  pSrc8_t;

	pSrc8_t = (uint8_t*)(src.data);
	pDst8_t = (uint8_t*)(dst.data);
//#pragma UNROLL 4
//#pragma omp parallel for
	for(int y = 0; y < ImgHeight*ImgWidth; y++)
	{
		pDst8_t[y] = pSrc8_t[y*2+1];
	}
}

char trkINFODisplay[256];

void translateTransform(cv::Mat const& src, cv::Mat& dst, int dx, int dy)
{
	if(dx >= src.cols - 10)
		return ;
	dst = cv::Mat::zeros(src.rows,src.cols,CV_8UC1);
	printf("%s:dx=%d\n",__func__, dx);
	for (int j=0 ; j<src.rows-dy; j++)
	{
		memcpy(dst.ptr<uchar>(j+dy,dx),src.ptr<uchar>(j,0), src.cols -dx);
		for (int i=0 ; i<src.cols -dx ; i++)
		{
			dst.at<uchar>(j+dy,i+dx) = (int)src.at<uchar>(j,i);
		}
	}

}

void CVideoProcess::main_proc_func()
{
	OSA_printf("%s: Main Proc Tsk Is Entering...\n",__func__);
	unsigned int framecount=0;
	
	float speedx,speedy;
	float optValue;
	UTC_Rect AcqRect;
	unsigned int t1 ;
	static bool Movedetect=false;
	Point pt1,pt2,erspt1,erspt2,erspt3,erspt4;
	static UTC_ACQ_param acqRect;
	CMD_EXT tmpCmd={0};
	double value;
	int tmpVal;
	bool retFlag;
	
	std::vector<cv::Point2f> sceDetectResult;
	cv::Point2f tmpSceDetectResult;
#if 1
	
	static int timeFlag = 2;
	static int speedcount = 0;

	time_t timep;  
	struct tm *p;  	
	char file[128];
	const int MvDetectAcqRectWidth  =  80;
	const int MvDetectAcqRectHeight =  80;
	cv::Point2f tmpPoint;

	static int movex = 0;
	static int movey = 0;
	static int sceneJudge;
	cv::Rect getbound;
	Rect2d getSceneRect,SceneRectInit;
	static int channelId;
	unsigned int patternTime,pTime;
	unsigned int timePoint;
#endif
	while(mainProcThrObj.exitProcThread ==  false)
	{
		OSA_semWait(&mainProcThrObj.procNotifySem, OSA_TIMEOUT_FOREVER);

		Mat frame = mainFrame[mainProcThrObj.pp^1];
		bool bTrack = mainProcThrObj.cxt[mainProcThrObj.pp^1].bTrack;
		bool bMtd = mainProcThrObj.cxt[mainProcThrObj.pp^1].bMtd;
		bool bMoveDetect = mainProcThrObj.cxt[mainProcThrObj.pp^1].bMoveDetect;
		bool bPatternDetect = mainProcThrObj.cxt[mainProcThrObj.pp^1].bPatternDetect;
		int chId = mainProcThrObj.cxt[mainProcThrObj.pp^1].chId;
		int iTrackStat = mainProcThrObj.cxt[mainProcThrObj.pp^1].iTrackStat;

		bool bSceneTrack = mainProcThrObj.cxt[mainProcThrObj.pp^1].bSceneTrack;
		int iSceneTrackStat = mainProcThrObj.cxt[mainProcThrObj.pp^1].iSceneTrackStat;
			
		int channel = frame.channels();
		Mat frame_gray;

		cv::Mat	salientMap, sobelMap;

		mainProcThrObj.pp ^=1;
		if(!OnPreProcess(chId, frame))
			continue;

		if(!m_bMoveDetect){
			motionlessflag = false;
			sceneJudge = 0;
			sceDetectResult.clear();
		}

		if(!m_bSceneTrack)
		{
			channelId = 0;
			getSceneRectBK.width = 0;
		}
		

		if(!m_bTrack && !m_bMtd &&!m_bMoveDetect&&!m_bSceneTrack&&!m_bPatterDetect){
			OnProcess(chId, frame);
			continue;
		}
		

		if(chId != m_curChId)
			continue;
	
		frame_gray = Mat(frame.rows, frame.cols, CV_8UC1);

		if(channel == 2)
		{
			if((chId == video_gaoqing0)||(chId == video_gaoqing)||(chId == video_gaoqing2)||(chId == video_gaoqing3))
				extractYUYV2Gray2(frame, frame_gray);
			else if(chId == video_pal)
				extractUYVY2Gray(frame, frame_gray);
		}
		else
		{
			memcpy(frame_gray.data, frame.data, frame.cols * frame.rows*channel*sizeof(unsigned char));
		}

		if(bTrack)
		{
		#if __TRACK__
			iTrackStat = ReAcqTarget();
			if(Movedetect&&(iTrackStat==0))
			{
			}
			int64 trktime = 0;
			if(algOsdRect == true)
				trktime = getTickCount();//OSA_getCurTimeInMsec();
			if(m_iTrackStat==2)
			{
				//m_searchmod=1;
			}
			else
			{
				m_searchmod=0;
			}
			m_iTrackStat = process_track(iTrackStat, frame_gray, frame_gray, m_rcTrack);

			putText(m_display.m_imgOsd[msgextInCtrl->SensorStat],trkINFODisplay,
				Point( 10, 25),
				FONT_HERSHEY_TRIPLEX,0.8,
				cvScalar(0,0,0,0), 1
			);
#if 0
			sprintf(trkINFODisplay, "trkStatus:%u,trkErrorX=%f,trkErrorY=%f",
				iTrackStat,m_rcTrack.x,m_rcTrack.y);
			putText(m_display.m_imgOsd[msgextInCtrl->SensorStat],trkINFODisplay,
				Point( 10, 25),
				FONT_HERSHEY_TRIPLEX,0.8,
				cvScalar(255,255,0,255), 1
			);
#endif
			

			UtcGetSceneMV(m_track, &speedx, &speedy);
			UtcGetOptValue(m_track, &optValue);
			
			if(m_iTrackStat == 2)
				m_iTrackLostCnt++;
			else
				m_iTrackLostCnt = 0;

			if(m_iTrackStat == 2)
			{								
				if(m_iTrackLostCnt > 3)
				{					
				}
				else
				{
					m_iTrackStat = 1;
				}
			}

			if(m_display.disptimeEnable == 1)
			{
				putText(m_display.m_imgOsd[1],m_strDisplay,
						Point( m_display.m_imgOsd[1].cols-450, 30),
						FONT_HERSHEY_TRIPLEX,0.8,
						cvScalar(0,0,0,0), 1
						);
				sprintf(m_strDisplay, "TrkStat=%d speedxy: (%0.2f,%0.2f)", m_iTrackStat, speedx,speedy);

				putText(m_display.m_imgOsd[1],m_strDisplay,
						Point( m_display.m_imgOsd[1].cols-450, 30),
						FONT_HERSHEY_TRIPLEX,0.8,
						cvScalar(255,255,0,255), 1
						);
			}
			
			//printf("********m_iTrackStat=%d\n",m_iTrackStat);
			//OSA_printf("ALL-Trk: time = %d ms \n", OSA_getCurTimeInMsec() - trktime);
			if(algOsdRect == true){
				float time = ( (getTickCount() - trktime)/getTickFrequency())*1000;;//OSA_getCurTimeInMsec() - trktime;
				static float totaltime = 0;
				static int count11 = 1;
				totaltime += time;
				if((count11++)%100 == 0)
				{
					OSA_printf("ALL-TRK: time = %f ms \n", totaltime/100 );
					count11 = 1;
					totaltime = 0.0;
				}
			}
		#endif
		}
		else if(bMtd)
		{
			tstart = getTickCount();

			Rect roi;

			if(tvzoomStat)
				{
					roi.x=frame_gray.cols/4;
					roi.y=frame_gray.rows/4;
					roi.width=frame_gray.cols/2;
					roi.height=frame_gray.rows/2;
					//OSA_printf("roiXY(%d,%d),WH(%d,%d)\n",roi.x,roi.y,roi.width,roi.height);
				}
			else
				{
					roi.x=0;
					roi.y=0;
					roi.width=frame_gray.cols;
					roi.height=frame_gray.rows;
				}
		#if __MMT__
			//m_MMTDObj.MMTDProcess(frame_gray, m_tgtBox, m_display.m_imgOsd[1], 0);
			m_MMTDObj.MMTDProcessRect(frame_gray, m_tgtBox, roi, m_display.m_imgOsd[1], 0);
			for(int i=0;i<MAX_TARGET_NUMBER;i++)
			{
				m_mtd[chId]->tg[i].cur_x=m_tgtBox[i].Box.x+m_tgtBox[i].Box.width/2;
				m_mtd[chId]->tg[i].cur_y=m_tgtBox[i].Box.y+m_tgtBox[i].Box.height/2;
				m_mtd[chId]->tg[i].valid=m_tgtBox[i].valid;
				//OSA_printf("ALL-MTD: time  ID %d  valid=%d x=%d y=%d\n",i,m_tgtBox[i].valid,m_tgtBox[i].Box.x,m_tgtBox[i].Box.y);
			}
		#endif	
		
		}
		else if (bMoveDetect)
		{
		#if __MOVE_DETECT__
			if(m_pMovDetector != NULL)
			{	
				if(!motionlessflag)
				{ 		
					#if 1
						if( 0 == sceneJudge )
							t1 = OSA_getCurTimeInMsec();
						m_sceneObj.detect(frame_gray, chId);	
						m_sceneObj.getResult(tmpSceDetectResult);
						sceDetectResult.push_back(tmpSceDetectResult);
						if(sceDetectResult.size() >= 5)
						{
						
							if( abs(sceDetectResult[0].x -  sceDetectResult[1].x ) < 1
								&& abs(sceDetectResult[1].x -  sceDetectResult[2].x ) < 1
								&& abs(sceDetectResult[2].x -  sceDetectResult[3].x ) < 1
								&& abs(sceDetectResult[3].x -  sceDetectResult[4].x ) < 1
								&& abs(sceDetectResult[0].y -  sceDetectResult[1].y ) < 1
								&& abs(sceDetectResult[1].y -  sceDetectResult[2].y ) < 1
								&& abs(sceDetectResult[2].y -  sceDetectResult[3].y ) < 1
								&& abs(sceDetectResult[3].y -  sceDetectResult[4].y ) < 1 )
							{
								motionlessflag = true;
								sceDetectResult.clear();
							}
							sceDetectResult.erase(sceDetectResult.begin(),sceDetectResult.begin()+1);
						}
						if( sceneJudge > 50 )
							motionlessflag = true;
					#else
						if( 0 == sceneJudge )
							t1 = OSA_getCurTimeInMsec();
						if( OSA_getCurTimeInMsec() - t1 > 1000)
							motionlessflag = true;
					#endif
					sceneJudge++;
				}
				 if(motionlessflag)
				{
					m_pMovDetector->setFrame(frame_gray,msgextInCtrl->SensorStat,Mtd_Frame.detectSpeed, Mtd_Frame.tmpMinPixel, Mtd_Frame.tmpMaxPixel, Mtd_Frame.sensitivityThreshold);
				}
			}
		#endif
		}
		else if( bSceneTrack )
		{	
			if(channelId < 10)
				channelId++;

			if( 1 == channelId )
			{	
				SceneRectInit.x = vcapWH[chId][0]/4;
				SceneRectInit.y = vcapWH[chId][1]/4;
				SceneRectInit.width = vcapWH[chId][0]*1/2;
				SceneRectInit.height = vcapWH[chId][1]*1/2;
				m_sceneObj.sceneLockInit( frame_gray , SceneRectInit);
			}
			else
			{
				unsigned long int t1 = cv::getTickCount();
				retFlag = m_sceneObj.sceneLockProcess( frame_gray, getSceneRect );
				unsigned long int t2 = cv::getTickCount();
				printf("  cost time = %d \n",(t2 - t1)/1000000);
				if(!retFlag)
					printf(" warning : scene Lost !!!\n");
					
				if( retFlag )
				{
					getSceneRectBK = getSceneRect;
				}
	
				tmpPoint.x = (float)(getSceneRectBK.x + getSceneRectBK.width/2 - msgextInCtrl->opticAxisPosX[msgextInCtrl->SensorStat]); 
				tmpPoint.y = (float)(getSceneRectBK.y + getSceneRectBK.height/2 - msgextInCtrl->opticAxisPosY[msgextInCtrl->SensorStat]);	

				
				
				//send IPC
				if( getSceneRectBK.width && getSceneRectBK.height && getSceneRectBK.x && getSceneRectBK.y )
				{
					SENDST scenetrk;
					scenetrk.cmd_ID = sceneTrk;
					tmpVal = msgextInCtrl->SceneAvtTrkStat;
					memcpy(&scenetrk.param[0] ,&tmpVal, 4);
					memcpy(&scenetrk.param[4] ,&tmpPoint.x , 4);
					memcpy(&scenetrk.param[8] ,&tmpPoint.y , 4);
					ipc_sendmsg(&scenetrk, IPC_FRIMG_MSG);
				}	
			}	
		}

		if(bPatternDetect)
		{
			
			#if __PATTERN_DETECT__
				detectornew->detectasync(frame);
			#endif
		}

		
		OnProcess(chId, frame);
		framecount++;

	/************************* while ********************************/
	}
	OSA_printf("%s: Main Proc Tsk Is Exit...\n",__func__);
}

int CVideoProcess::MAIN_threadDestroy(void)
{
	int iRet = OSA_SOK;

	mainProcThrObj.exitProcThread = true;
	OSA_semSignal(&mainProcThrObj.procNotifySem);

	iRet = OSA_thrDelete(&mainProcThrObj.thrHandleProc);

	mainProcThrObj.initFlag = false;
	OSA_semDelete(&mainProcThrObj.procNotifySem);

	return iRet;
}

CVideoProcess::CVideoProcess()
	:m_track(NULL),m_curChId(MAIN_CHID),m_curSubChId(-1),adaptiveThred(40)		
{
	pThis = this;
	memset(m_mtd, 0, sizeof(m_mtd));
	memset(&mainProcThrObj, 0, sizeof(MAIN_ProcThrObj));
	
	detState = TRUE;
	trackEnd = FALSE;
	trackStart = TRUE;
	nextDetect = FALSE;
	lastFrameBox=0;
	moveStat = FALSE;
	m_acqRectW			= 	60;
	m_acqRectH			= 	60;

	m_intervalFrame 			= 0;
	m_intervalFrame_change 	= 0;
	m_bakChId = m_curChId;
	trackchange		=0;
	m_searchmod		=0;
	tvzoomStat		=0;
	wFileFlag			=0;
	preAcpSR	={0};
	algOsdRect = false;

	mptz_click = mptz_originX = mptz_originY = 0;
	
#if __MOVE_DETECT__
	m_pMovDetector	=NULL;
#endif

#if __MMT__
	memset(m_tgtBox, 0, sizeof(TARGETBOX)*MAX_TARGET_NUMBER);
#endif


	m_click = m_draw = m_tempX = m_tempY = 0;
	memset(m_rectn, 0, sizeof(m_rectn));
	memset(mRect, 0, sizeof(mRect));
	setrigon_flag = 0;

	pol_draw = pol_tempX = pol_tempY = 0;
	memset(pol_rectn, 0, sizeof(pol_rectn));
	memset(polRect, 0, sizeof(polRect));
	memset(polWarnRect, 0, sizeof(polWarnRect));
	memset(polWarnRectBak, 0, sizeof(polWarnRectBak));
	memset(polwarn_count, 0, sizeof(polwarn_count));
	memset(polwarn_count_bak, 0, sizeof(polwarn_count_bak));
	setrigon_polygon = 0;


}

CVideoProcess::~CVideoProcess()
{
	pThis = NULL;
}

int CVideoProcess::creat()
{
	int i = 0;
	#if __TRACK__
		trackinfo_obj=(Track_InfoObj *)malloc(sizeof(Track_InfoObj));
	#endif
	m_display.create();

	MultiCh.m_user = this;
	MultiCh.m_usrFunc = callback_process;
	MultiCh.creat();

	MAIN_threadCreate();
	OSA_mutexCreate(&m_mutex);
	OSA_mutexCreate(&m_algboxLock);
	OSA_mutexCreate(&m_trackboxLock);
	OnCreate();

	
#if __MOVE_DETECT__
	if(m_pMovDetector == NULL)
		m_pMovDetector = MvDetector_Create();
	OSA_assert(m_pMovDetector != NULL);
#endif


	model.push_back(string("config/yolo_v3_tiny.yml"));
	modelsize.push_back(Size(1920,1080));

#if __PATTERN_DETECT__
	detectornew = DetectorFactory::getinstance()->createDetector(DetectorFactory::DEEPLEARNDETECTOR);

	detectornew->setparam(Detector::MAXTRACKNUM,20);
	detectornew->init(model,modelsize);
	detectornew->dynamicsetparam(Detector::DETECTSCALE100x,100);
	detectornew->dynamicsetparam(Detector::DETECTMOD,1);
	detectornew->dynamicsetparam(Detector::DETECTFREQUENCY,1);
	detectornew->dynamicsetparam(Detector::DETECTNOTRACK,0);
	detectornew->getversion();
	detectornew->setasyncdetect(detectcall,trackcall);
#endif
	return 0;
}


int CVideoProcess::destroy()
{
	stop();
	OSA_mutexDelete(&m_mutex);
	OSA_mutexDelete(&m_algboxLock);
	OSA_mutexDelete(&m_trackboxLock);
	MAIN_threadDestroy();

	MultiCh.destroy();
	m_display.destroy();

	OnDestroy();

#if __MOVE_DETECT__
	DeInitMvDetect();
#endif

	return 0;
}


void CVideoProcess::mousemotion_event(GLint xMouse, GLint yMouse)
{
	SENDST test;
	CMD_MOUSEPTZ mptz;

	test.cmd_ID = mouseptz;
	if(pThis->mptz_click == 1)
	{
		mptz.mptzx = xMouse - pThis->mptz_originX;
		mptz.mptzy = pThis->mptz_originY - yMouse;
		memcpy(test.param, &mptz, sizeof(mptz));
		ipc_sendmsg(&test, IPC_FRIMG_MSG);
	}
}


int CVideoProcess::map1080p2normal_point(float *x, float *y)
{
	if(NULL != x)
		*x /= 1920;
	if(NULL != y)
		*y /= 1080;

	return 0;
}

int CVideoProcess::mapnormal2curchannel_point(float *x, float *y, int w, int h)
{
	if(NULL != x)
		*x *= w;
	if(NULL != y)
		*y *= h;
	
	return 0;
}

int CVideoProcess::map1080p2normal_rect(mouserectf *rect)
{
	if(NULL != rect)
	{
		rect->x /= 1920;
		rect->w /= 1920;
		rect->y /= 1080;
		rect->h /= 1080;
		return 0;
	}

	return -1;
}

int CVideoProcess::mapnormal2curchannel_rect(mouserectf *rect, int w, int h)
{
	if(NULL != rect)
	{
		rect->x *= w;
		rect->w *= w;
		rect->y *= h;
		rect->h *= h;
		return 0;
	}
	return -1;
}

void CVideoProcess::mouse_event(int button, int state, int x, int y)
{
	unsigned int curId;

	int Critical_Point;	

		curId = pThis->m_curChId;

	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		{
			if( pThis->setrigon_flag && !m_bMoveDetect)
			{
				if(pThis->m_click == 0)
				{
					pThis->m_click = 1;
					pThis->m_rectn[curId] = 0;

					float floatx,floaty;
					floatx = x;
					floaty = y;
					pThis->map1080p2normal_point(&floatx, &floaty);
					pThis->mapnormal2curchannel_point(&floatx, &floaty, vdisWH[curId][0], vdisWH[curId][1]);
					
					pThis->mRect[curId][pThis->m_rectn[curId]].x1 = floatx;
					pThis->mRect[curId][pThis->m_rectn[curId]].y1 = floaty;

				}
				else
				{	
					pThis->m_click = 0;

					float floatx,floaty;
					floatx = x;
					floaty = y;
					pThis->map1080p2normal_point(&floatx, &floaty);
					pThis->mapnormal2curchannel_point(&floatx, &floaty, vdisWH[curId][0], vdisWH[curId][1]);
					
					pThis->mRect[curId][pThis->m_rectn[curId]].x2 = floatx;
					pThis->mRect[curId][pThis->m_rectn[curId]].y2 = floaty;


				/*printf("Rigion%d: Point1(%d,%d),Point2(%d,%d),Point3(%d,%d),Point4(%d,%d)\n",
					pThis->m_rectn[curId],
					pThis->mRect[curId][pThis->m_rectn[curId]].x1,pThis->mRect[curId][pThis->m_rectn[curId]].y1,
					pThis->mRect[curId][pThis->m_rectn[curId]].x2,pThis->mRect[curId][pThis->m_rectn[curId]].y1,
					pThis->mRect[curId][pThis->m_rectn[curId]].x1,pThis->mRect[curId][pThis->m_rectn[curId]].y2,
					pThis->mRect[curId][pThis->m_rectn[curId]].x2,pThis->mRect[curId][pThis->m_rectn[curId]].y2
				);*/
					//point1  ---  lefttop    ,  point2  --- righttop  , point3 --- leftbottom  ,point --- rightbottom

					mouserect rectsrcf;
					int redx, redy;
					redx = pThis->mRect[curId][pThis->m_rectn[curId]].x2 - pThis->mRect[curId][pThis->m_rectn[curId]].x1;
					redy = pThis->mRect[curId][pThis->m_rectn[curId]].y2 - pThis->mRect[curId][pThis->m_rectn[curId]].y1;
					rectsrcf.x = redx>0?pThis->mRect[curId][pThis->m_rectn[curId]].x1:pThis->mRect[curId][pThis->m_rectn[curId]].x2;
					rectsrcf.w = abs(redx);
					rectsrcf.y = redy>0?pThis->mRect[curId][pThis->m_rectn[curId]].y1:pThis->mRect[curId][pThis->m_rectn[curId]].y2;
					rectsrcf.h = abs(redy);

					pThis->polWarnRect[curId][0].x = rectsrcf.x;
					pThis->polWarnRect[curId][0].y = rectsrcf.y;
					pThis->polWarnRect[curId][1].x = rectsrcf.x+rectsrcf.w;
					pThis->polWarnRect[curId][1].y = rectsrcf.y;
					pThis->polWarnRect[curId][2].x = rectsrcf.x+rectsrcf.w;
					pThis->polWarnRect[curId][2].y = rectsrcf.y+rectsrcf.h;
					pThis->polWarnRect[curId][3].x = rectsrcf.x;
					pThis->polWarnRect[curId][3].y = rectsrcf.y+rectsrcf.h;
					pThis->polwarn_count[curId] = 4;
								
					std::vector<cv::Point> polyWarnRoi ;
					polyWarnRoi.resize(4);		
					polyWarnRoi[0] = cv::Point(rectsrcf.x, rectsrcf.y);
					polyWarnRoi[1] = cv::Point(rectsrcf.x+rectsrcf.w, rectsrcf.y);
					polyWarnRoi[2] = cv::Point(rectsrcf.x+rectsrcf.w, rectsrcf.y+rectsrcf.h);
					polyWarnRoi[3] = cv::Point(rectsrcf.x, rectsrcf.y+rectsrcf.h);

					pThis->m_pMovDetector->setWarningRoi( polyWarnRoi,	curId);		
					pThis->m_rectn[curId]++;
					if(pThis->m_rectn[curId]>=sizeof(pThis->mRect[0]))
					{
						printf("mouse rect reached maxnum:100!\n");
						pThis->m_rectn[curId]--;
					}
					pThis->m_draw = 1;
					pThis->setrigon_flag = 0;
				}
			}
			else if(pThis->setrigon_polygon && !m_bMoveDetect)
			{
				float floatx,floaty;
				floatx = x;
				floaty = y;
				pThis->map1080p2normal_point(&floatx, &floaty);
				pThis->mapnormal2curchannel_point(&floatx, &floaty, vdisWH[curId][0], vdisWH[curId][1]);
					
				pThis->polRect[curId][pThis->pol_rectn[curId]].x = floatx;
				pThis->polRect[curId][pThis->pol_rectn[curId]].y = floaty;
				pThis->pol_rectn[curId]++;
				if(pThis->pol_rectn[curId]>1)
					pThis->pol_draw = 1;
			}
			else
			{

			}
		}
		
	}

	if(button == 3)
	{
		pThis->m_click = 0;
		pThis->m_rectn[curId] = 0;
		pThis->m_draw = 1;
	}
	if(button == 4)
	{
		int setx, sety = 0;
		if(pThis->setrigon_polygon == 1)
		{
			memcpy(&pThis->polWarnRect, &pThis->polRect, sizeof(pThis->polRect));
			memcpy(&pThis->polwarn_count, &pThis->pol_rectn, sizeof(pThis->pol_rectn));
			std::vector<cv::Point> polyWarnRoi ;
			polyWarnRoi.resize(pThis->pol_rectn[curId]);
			for(int i = 0; i < pThis->pol_rectn[curId]; i++)
			{
				setx = pThis->polRect[curId][i].x;
				sety = pThis->polRect[curId][i].y;
				polyWarnRoi[i] = cv::Point(setx, sety);
			}		
			pThis->m_pMovDetector->setWarningRoi( polyWarnRoi,	curId);
			pThis->setrigon_polygon = 0;
			pThis->pol_rectn[curId] = 0;
			pThis->pol_draw = 1;
		}
	}
	
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		pThis->mptz_click = 1;
		pThis->mptz_originX = x;
		pThis->mptz_originY = y;
	}
	else if(button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		SENDST test;
		CMD_MOUSEPTZ mptz;
		
		pThis->mptz_click = 0;
		test.cmd_ID = mouseptz;
		mptz.mptzx = 0;
		mptz.mptzy = 0;
		memcpy(test.param, &mptz, sizeof(mptz));
		ipc_sendmsg(&test, IPC_FRIMG_MSG);

	}
}

void CVideoProcess::mousemove_event(GLint xMouse, GLint yMouse)
{
	unsigned int curId;
		curId = pThis->m_curChId;

	float floatx,floaty;
	floatx = xMouse;
	floaty = yMouse;	
	pThis->map1080p2normal_point(&floatx, &floaty);
	pThis->mapnormal2curchannel_point(&floatx, &floaty, vdisWH[curId][0], vdisWH[curId][1]);	

	if(pThis->m_click == 1)
	{
		pThis->m_tempX = floatx;
		pThis->m_tempY = floaty;
		
		pThis->m_draw = 1;
	}
	{
		pThis->pol_tempX = floatx;
		pThis->pol_tempY = floaty;
		pThis->pol_draw = 1;
	}
}

void CVideoProcess::menu_event(int value)
{
	switch(value)
    {
		case 0:
			pThis->OnKeyDwn('a');
			break;
		default:
			break;
	}
}

void CVideoProcess::processrigionMenu(int value)
{
	//printf("set rigion\n");
	pThis->setrigon_flag = 1;
}

void CVideoProcess::processrigionselMenu(int value)
{
	printf("%s start, value=%d\n", __FUNCTION__, value);
}

void CVideoProcess::processrigionpolygonMenu(int value)
{
	//printf("%s start, value=%d\n", __FUNCTION__, value);
	pThis->setrigon_polygon = 1;
}

#if __MOVE_DETECT__
void CVideoProcess::processmaxnumMenu(int value)
{
	if(0 == value)
		Mtd_Frame.detectNum = 5;
	else if(1 == value)
		Mtd_Frame.detectNum = 10;
}

void CVideoProcess::processmaxtargetsizeMenu(int value)
{
	if(0 == value)
		Mtd_Frame.tmpMaxPixel = 40000;
	else if(1 == value)
		Mtd_Frame.tmpMaxPixel = 50000;
}

void CVideoProcess::processmintargetsizeMenu(int value)
{
	if(0 == value)
		Mtd_Frame.tmpMinPixel = 100;
	else if(1 == value)
		Mtd_Frame.tmpMinPixel = 1000;
}
#endif
void CVideoProcess::keyboard_event(unsigned char key, int x, int y)
{
	pThis->OnKeyDwn(key);

	if(key == 27){
		pThis->destroy();
		exit(0);
	}
}

void CVideoProcess::keySpecial_event( int key, int x, int y)
{
	//pThis->OnKeyDwn((unsigned char)key);
	pThis->OnSpecialKeyDwn(key,x,y);
}

void CVideoProcess::visibility_event(int state)
{
	OSA_printf("visibility event: %d\n", state);
}

void CVideoProcess::close_event(void)
{
	OSA_printf("close event\n");
	pThis->destroy();
}

int CVideoProcess::init()
{
	DS_InitPrm dsInit;
	memset(&dsInit, 0, sizeof(DS_InitPrm));
	dsInit.motionfunc = mousemotion_event;
	dsInit.menufunc = menu_event;
	dsInit.mousefunc = mouse_event;
	dsInit.passivemotionfunc = mousemove_event;
	dsInit.setrigion = processrigionMenu;
	dsInit.rigionsel = processrigionselMenu;
	dsInit.rigionpolygon = processrigionpolygonMenu;
#if __MOVE_DETECT__
	dsInit.maxnum = processmaxnumMenu;
	dsInit.maxsize= processmaxtargetsizeMenu;
	dsInit.minsize= processmintargetsizeMenu;
#endif
	dsInit.disFPS = 30;
	dsInit.disSched = 33;   //  3.5;
	

//#if (!__IPC__)
	dsInit.keyboardfunc = keyboard_event; 
	dsInit.keySpecialfunc = keySpecial_event;
//#endif
	
	dsInit.timerfunc = call_run;
	//dsInit.idlefunc = call_run;
	dsInit.visibilityfunc = visibility_event;
	dsInit.timerfunc_value = 0;
	dsInit.timerInterval = 16;//ms
	dsInit.closefunc = close_event;
	dsInit.bFullScreen = true;
	dsInit.winPosX = 200;
	dsInit.winPosY = 100;
	dsInit.winWidth = vdisWH[1][0];
	dsInit.winHeight = vdisWH[1][1];

	m_display.init(&dsInit);

	m_display.m_bOsd = true;

	OnInit();
	prichnalid=1;//fir


#if __MOVE_DETECT__
	initMvDetect();
#endif
	
	return 0;
}


int CVideoProcess::dynamic_config(int type, int iPrm, void* pPrm)
{
	int iret = 0;
	unsigned int render=0;
	OSA_mutexLock(&m_mutex);

	if(type < CDisplayer::DS_CFG_Max){
		iret = m_display.dynamic_config((CDisplayer::DS_CFG)type, iPrm, pPrm);
	}

	switch(type)
	{
	case VP_CFG_MainChId:
		m_curChId = iPrm;
		m_iTrackStat = 0;
		mainProcThrObj.bFirst = true;
		m_display.dynamic_config(CDisplayer::DS_CFG_ChId, 0, &m_curChId);
		break;
	case VP_CFG_SubChId:
		m_curSubChId = iPrm;
		m_display.dynamic_config(CDisplayer::DS_CFG_ChId, 1, &m_curSubChId);
		break;
	case VP_CFG_TrkEnable:
		m_bTrack = iPrm;
		m_bakChId = m_curChId;
		m_iTrackStat = 0;
		m_iTrackLostCnt = 0;
		mainProcThrObj.bFirst = true;
		if(pPrm == NULL)
		{			
			UTC_RECT_float rc;
			rc.width 	=  60;//m_acqRectW;
			rc.height 	=  60;//m_acqRectH;
			rc.x 		=  msgextInCtrl->AvtPosX[m_curChId] - rc.width/2;
			rc.y 		=  msgextInCtrl->AvtPosY[m_curChId] - rc.height/2;
			m_rcTrack   = rc;
			m_rcAcq 	= rc;
		}
		else
		{
			m_rcTrack = *(UTC_RECT_float*)pPrm;
		}
		break;
	case VP_CFG_MmtEnable:
		m_bMtd = iPrm;
		break;
	case VP_CFG_SubPicpChId:
		m_curSubChId = iPrm;
		if(pPrm!=NULL)
		render=*(int *)pPrm;
		m_display.dynamic_config(CDisplayer::DS_CFG_ChId, render, &m_curSubChId);
		break;
	case VP_CFG_MvDetect:
		m_bMoveDetect = iPrm;
		if(m_bMoveDetect)
			m_sceneObj.start();
		break;
	case VP_CFG_SceneTrkEnable:
		m_bSceneTrack = iPrm;
		if(m_bSceneTrack)
			m_sceneObj.start();
		m_iSceneTrackStat = 0;
		m_iSceneTrackLostCnt = 0;
		mainProcThrObj.bFirst = true;	
		msgextInCtrl->SceneAvtTrkStat = m_bSceneTrack;
		break;
	case VP_CFG_PatterDetectEnable:
		m_bPatterDetect = iPrm;
		break;
	default:
		break;
	}

	if(iret == 0)
		OnConfig();

	OSA_mutexUnlock(&m_mutex);

	return iret;
}

#if 1
int CVideoProcess::configEnhFromFile()
{
	string CalibFile;
	CalibFile = "config.yml";

	char calib_x[11] = "config_x";


	FILE *fp = fopen(CalibFile.c_str(), "rt");
	if(fp != NULL)
	{
		fseek(fp, 0, SEEK_END);
		int len = ftell(fp);
		fclose(fp);
		
		if(len < 10)
			return -1;
		else
		{
			
			FileStorage fr(CalibFile, FileStorage::READ);
			if(fr.isOpened())
			{
				
					sprintf(calib_x, "enhmod_%d", 0);
					Enhmod= (int)fr[calib_x];

					sprintf(calib_x, "enhparm_%d", 1);
					Enhparm= (float)fr[calib_x];

					sprintf(calib_x, "mmtdparm_%d", 2);
					DetectGapparm= (int)fr[calib_x];

					sprintf(calib_x, "mmtdparm_%d", 3);
					MinArea= (int)fr[calib_x];

					sprintf(calib_x, "mmtdparm_%d", 4);
					MaxArea= (int)fr[calib_x];

					sprintf(calib_x, "mmtdparm_%d", 5);
					stillPixel= (int)fr[calib_x];

					sprintf(calib_x, "mmtdparm_%d", 6);
					movePixel= (int)fr[calib_x];

					sprintf(calib_x, "mmtdparm_%d", 7);
					lapScaler= (float)fr[calib_x];
				

					sprintf(calib_x, "mmtdparm_%d", 8);
					lumThred= (int)fr[calib_x];
					
					sprintf(calib_x, "timedisp_%d", 9);
					m_display.disptimeEnable= (int)fr[calib_x];
				return 0;
			}else
				return -1;
		}
	}else
		return -1;
}
#endif


int CVideoProcess::run()
{
	MultiCh.run();
	m_display.run();
	
	#if __TRACK__
	m_track = CreateUtcTrk();
	UtcSetAxisSech(m_track , false);
	#endif
	
	for(int i=0; i<MAX_CHAN; i++){
		m_mtd[i] = (target_t *)malloc(sizeof(target_t));
		if(m_mtd[i] != NULL)
			memset(m_mtd[i], 0, sizeof(target_t));

		OSA_printf(" %d:%s mtd malloc %p\n", OSA_getCurTimeInMsec(),__func__, m_mtd[0]);
	}
#if __MMT__
	m_MMTDObj.SetTargetNum(MAX_TARGET_NUMBER);
#endif
	OnRun();
	return 0;
}

int CVideoProcess::stop()
{
	if(m_track != NULL)
	{
		#if __TRACK__
			DestroyUtcTrk(m_track);
		#endif
	}
	m_track = NULL;
	
	m_display.stop();
	MultiCh.stop();

	OnStop();
	return 0;
}

void CVideoProcess::call_run(int value)
{
	pThis->process_event(0, 0, NULL);
}

void CVideoProcess::process_event(int type, int iPrm, void *pPrm)
{

	Ontimer();
	if(type == 0)//timer event from display
	{
	}
	
}

int CVideoProcess::callback_process(void *handle, int chId, int virchId, Mat frame)
{
	CVideoProcess *pThis = (CVideoProcess*)handle;
	return pThis->process_frame(chId, virchId, frame);
}

static void extractYUYV2Gray(Mat src, Mat dst)
{
	int ImgHeight, ImgWidth,ImgStride, stride16x8;

	ImgWidth = src.cols;
	ImgHeight = src.rows;
	ImgStride = ImgWidth*2;
	stride16x8 = ImgStride/16;

	OSA_assert((ImgStride&15)==0);
//#pragma omp parallel for
	for(int y = 0; y < ImgHeight; y++)
	{
		uint8x8_t  * __restrict__ pDst8x8_t;
		uint8_t * __restrict__ pSrc8_t;
		pSrc8_t = (uint8_t*)(src.data+ ImgStride*y);
		pDst8x8_t = (uint8x8_t*)(dst.data+ ImgWidth*y);
		for(int x=0; x<stride16x8; x++)
		{
			uint8x8x2_t d;
			d = vld2_u8((uint8_t*)(pSrc8_t+16*x));
			pDst8x8_t[x] = d.val[0];
		}
	}
}

void CVideoProcess::extractYUYV2Gray2(Mat src, Mat dst)
{
	int ImgHeight, ImgWidth,ImgStride;

	ImgWidth = src.cols;
	ImgHeight = src.rows;
	ImgStride = ImgWidth*2;
	uint8_t  *  pDst8_t;
	uint8_t *  pSrc8_t;

	pSrc8_t = (uint8_t*)(src.data);
	pDst8_t = (uint8_t*)(dst.data);
//#pragma UNROLL 4
//#pragma omp parallel for
	for(int y = 0; y < ImgHeight*ImgWidth; y++)
	{
		pDst8_t[y] = pSrc8_t[y*2];
	}
}


#if __TRACK__
void CVideoProcess::Track_fovreacq(int fov,int sensor,int sensorchange)
{
	//UTC_RECT_float rect;
	unsigned int currentx=0;
	unsigned int currenty=0;
	unsigned int TvFov[3] 	= {120,48,16};//Big-Mid-Sml:2400*5%,960*5%,330*5%
	unsigned int FrFov[5] 	= {200,120,50,16,6};//Big-Mid-Sml-SuperSml-Zoom:4000*5%,2400*5%,1000*5%,330*5%,120*5%

	if(sensorchange == 1){
		currentx = msgextInCtrl->AvtPosX[m_curChId];
		currenty = msgextInCtrl->AvtPosY[m_curChId];
	}
	else
	{		
			
		currentx=trackinfo_obj->trackrect.x+trackinfo_obj->trackrect.width/2;
		currenty=trackinfo_obj->trackrect.y+trackinfo_obj->trackrect.height/2;
		
	}
	int prifov=trackinfo_obj->trackfov;
	
	double ratio=tan(3.1415926*fov/36000)/tan(3.1415926*prifov/36000);
	
	unsigned int w=trackinfo_obj->trackrect.width/ratio;
	unsigned int h=trackinfo_obj->trackrect.height/ratio;
	if(sensorchange)
	{
		w=w*vcapWH[sensor^1][0]/vcapWH[sensor][0];
		h=h*vcapWH[sensor^1][1]/vcapWH[sensor][1];

	}
	
	trackinfo_obj->trackfov=fov;

	trackinfo_obj->reAcqRect.width=w;
	trackinfo_obj->reAcqRect.height=h;
	trackinfo_obj->reAcqRect.x=currentx-w/2;
	trackinfo_obj->reAcqRect.y=currenty-h/2;

	//OSA_printf("XY(%f,%f),WH(%f,%f)\n",trackinfo_obj->reAcqRect.x,trackinfo_obj->reAcqRect.y,trackinfo_obj->reAcqRect.width,trackinfo_obj->reAcqRect.height);
}
void CVideoProcess::Track_reacq(UTC_RECT_float & rcTrack,int acqinterval)
{
	m_intervalFrame=acqinterval;
	m_rcAcq=rcTrack;
}


int CVideoProcess::ReAcqTarget()
{
	//printf("m_intervalFrame = %d \n\n",m_intervalFrame);
	int iRet = m_iTrackStat;
	if(m_bakChId != m_curChId){
		iRet = 0;
		m_rcTrack = m_rcAcq;
		m_bakChId = m_curChId;
		m_iTrackLostCnt = 0;	
	}
	
	if(m_intervalFrame > 0){
		m_intervalFrame--;
		if(m_intervalFrame == 0){
			iRet = 0;
			m_rcTrack = m_rcAcq;
			m_iTrackLostCnt = 0;
			OSA_printf("----------------Setting m_intervalFrame----------------\n");
		}
	}

	return iRet;

}

#endif

extern void cutColor(cv::Mat src, cv::Mat &dst, int code);

#define TM
#undef TM 
int CVideoProcess::process_frame(int chId, int virchId, Mat frame)
{
	int format = -1;
	if(frame.cols<=0 || frame.rows<=0)
		return 0;

//	tstart = getTickCount();
	int  channel= frame.channels();

	

#ifdef TM
	cudaEvent_t	start, stop;
		float elapsedTime;
		( (		cudaEventCreate	(	&start)	) );
		( (		cudaEventCreate	(	&stop)	) );
		( (		cudaEventRecord	(	start,	0)	) );
#endif

	if(channel == 2){
//		cvtColor(frame,frame,CV_YUV2BGR_YUYV);
		if((chId == video_gaoqing0)||(chId == video_gaoqing)||(chId == video_gaoqing2)||(chId == video_gaoqing3))
			format = CV_YUV2BGR_YUYV;
		else if(chId == video_pal)
			format = CV_YUV2BGR_UYVY;
	}
	else {
//		cvtColor(frame,frame,CV_GRAY2BGR);
		format = CV_GRAY2BGR;
	}

	OSA_mutexLock(&m_mutex);


	if(chId == m_curChId)
	{
		if((chId==video_pal) && (virchId!= PAL_VIRCHID))
			;
		else
		{
			mainFrame[mainProcThrObj.pp] = frame;
			mainProcThrObj.cxt[mainProcThrObj.pp].bTrack = m_bTrack;
			mainProcThrObj.cxt[mainProcThrObj.pp].bMtd = m_bMtd;
			mainProcThrObj.cxt[mainProcThrObj.pp].bMoveDetect = m_bMoveDetect;
			mainProcThrObj.cxt[mainProcThrObj.pp].iTrackStat = m_iTrackStat;
			mainProcThrObj.cxt[mainProcThrObj.pp].bSceneTrack= m_bSceneTrack;
			mainProcThrObj.cxt[mainProcThrObj.pp].iSceneTrackStat = m_iSceneTrackStat;
			mainProcThrObj.cxt[mainProcThrObj.pp].bPatternDetect = m_bPatterDetect;
			mainProcThrObj.cxt[mainProcThrObj.pp].chId = chId;
			if(mainProcThrObj.bFirst){
				mainFrame[mainProcThrObj.pp^1] = frame;
				mainProcThrObj.cxt[mainProcThrObj.pp^1].bTrack = m_bTrack;
				mainProcThrObj.cxt[mainProcThrObj.pp^1].bMtd = m_bMtd;
				mainProcThrObj.cxt[mainProcThrObj.pp^1].bMoveDetect = m_bMoveDetect;
				mainProcThrObj.cxt[mainProcThrObj.pp^1].iTrackStat = m_iTrackStat;
				mainProcThrObj.cxt[mainProcThrObj.pp^1].bSceneTrack= m_bSceneTrack;
				mainProcThrObj.cxt[mainProcThrObj.pp^1].iSceneTrackStat = m_iSceneTrackStat;
				mainProcThrObj.cxt[mainProcThrObj.pp^1].bPatternDetect = m_bPatterDetect;
				mainProcThrObj.cxt[mainProcThrObj.pp^1].chId = chId;
				mainProcThrObj.bFirst = false;
			}
			OSA_semSignal(&mainProcThrObj.procNotifySem);
		}
	}

	
		
	//OSA_printf("chid =%d  m_curChId=%d m_curSubChId=%d\n", chId,m_curChId,m_curSubChId);


	if(chId == m_curChId || chId == m_curSubChId)
	{
		if((chId == video_pal)&&(virchId != PAL_VIRCHID));
		else
			m_display.display(frame,  chId, format);		
	}

	OSA_mutexUnlock(&m_mutex);


		

//	OSA_printf("process_frame: chId = %d, time = %f sec \n",chId,  ( (getTickCount() - tstart)/getTickFrequency()) );
	//获得结束时间，并显示结果

#ifdef TM
		(	(	cudaEventRecord(	stop,	0	)	)	);
		(	(	cudaEventSynchronize(	stop)	)	);

		(	cudaEventElapsedTime(	&elapsedTime,	start,	stop)	);
		OSA_printf("ChId = %d, Time to YUV2RGB:	%3.1f	ms \n", chId, elapsedTime);

		(	(	cudaEventDestroy(	start	)	)	);
		(	(	cudaEventDestroy(	stop	)	)	);
#endif
	return 0;
}


#if __TRACK__
int CVideoProcess::process_track(int trackStatus, Mat frame_gray, Mat frame_dis, UTC_RECT_float &rcResult)
{
	IMG_MAT image;

	image.data_u8 = frame_gray.data;
	image.width = frame_gray.cols;
	image.height = frame_gray.rows;
	image.channels = 1;
	image.step[0] = image.width;
	image.dtype = 0;
	image.size = frame_gray.cols*frame_gray.rows;

	if(trackStatus != 0)
	{
		rcResult = UtcTrkProc(m_track, image, &trackStatus);		
	}
	else
	{
		//printf("track********x=%f y=%f w=%f h=%f  ax=%d xy=%d\n",rcResult.x,rcResult.y,rcResult.width,rcResult.height);
		UTC_ACQ_param acq;
		acq.axisX 	=	msgextInCtrl->AvtPosX[m_curChId];// image.width/2;//m_ImageAxisx;//
		acq.axisY 	=	msgextInCtrl->AvtPosY[m_curChId];//image.height/2;//m_ImageAxisy;//
		acq.rcWin.x = 	(int)(rcResult.x);
		acq.rcWin.y = 	(int)(rcResult.y);
		acq.rcWin.width  = (int)(rcResult.width);
		acq.rcWin.height = (int)(rcResult.height);

		if(acq.rcWin.width<0)
			{
				acq.rcWin.width=0;

			}
		else if(acq.rcWin.width>= image.width)
			{
				acq.rcWin.width=60;
			}
		if(acq.rcWin.height<0)
			{
				acq.rcWin.height=0;

			}
		else if(acq.rcWin.height>= image.height)
			{
				acq.rcWin.height=60;
			}
		if(acq.rcWin.x<0)
			{
				acq.rcWin.x=0;
			}
		else if(acq.rcWin.x>image.width-acq.rcWin.width)
			{

				acq.rcWin.x=image.width-acq.rcWin.width;
			}
		if(acq.rcWin.y<0)
			{
				acq.rcWin.y=0;
			}
		else if(acq.rcWin.y>image.height-acq.rcWin.height)
			{

				acq.rcWin.y=image.height-acq.rcWin.height;
			}

		{
			//printf("=========movestat = %d\n",moveStat);
			rcResult = UtcTrkAcq(m_track, image, acq);
			moveStat = false;
		}
		
		trackStatus = 1;
	}

	return trackStatus;
}

#endif

vector<Rect> Box(MAX_TARGET_NUMBER);
int CVideoProcess::process_mtd(ALGMTD_HANDLE pChPrm, Mat frame_gray, Mat frame_dis)
{
	return 0;
}

#if __MOVE_DETECT__
void	CVideoProcess::initMvDetect()
{
	int	i;
	mouserect recttmp;
	OSA_printf("%s:mvDetect start ", __func__);
	OSA_assert(m_pMovDetector != NULL);

	m_pMovDetector->init(NotifyFunc, (void*)this);
					
	std::vector<cv::Point> polyWarnRoi ;
	polyWarnRoi.resize(4);

	for(i=0; i<MAX_CHAN; i++)
	{
		recttmp.x = vdisWH[i][0] * min_width_ratio;
		recttmp.y = vdisWH[i][1] * min_height_ratio;
		recttmp.w = vdisWH[i][0] * (max_width_ratio - min_width_ratio);
		recttmp.h = vdisWH[i][1] * (max_height_ratio - min_height_ratio); 

		pThis->polWarnRect[i][0].x = recttmp.x;
		pThis->polWarnRect[i][0].y = recttmp.y;
		pThis->polWarnRect[i][1].x = recttmp.x+recttmp.w;
		pThis->polWarnRect[i][1].y = recttmp.y;
		pThis->polWarnRect[i][2].x = recttmp.x+recttmp.w;
		pThis->polWarnRect[i][2].y = recttmp.y+recttmp.h;
		pThis->polWarnRect[i][3].x = recttmp.x;
		pThis->polWarnRect[i][3].y = recttmp.y+recttmp.h;
		pThis->polwarn_count[i] = 4;

		polyWarnRoi[0]	= cv::Point(recttmp.x,recttmp.y);
		polyWarnRoi[1]	= cv::Point(recttmp.x+recttmp.w,recttmp.y);
		polyWarnRoi[2]	= cv::Point(recttmp.x+recttmp.w,recttmp.y+recttmp.h);
		polyWarnRoi[3]	= cv::Point(recttmp.x,recttmp.y+recttmp.h);

		m_pMovDetector->setWarnMode(WARN_WARN_MODE, i);
		m_pMovDetector->setWarningRoi(polyWarnRoi,	i);
	}

		recttmp.x = Mtd_Frame.detectArea_X - Mtd_Frame.detectArea_wide/2;
		recttmp.y = Mtd_Frame.detectArea_Y - Mtd_Frame.detectArea_high/2;
		recttmp.w =Mtd_Frame.detectArea_wide;
		recttmp.h = Mtd_Frame.detectArea_high;

		pThis->polWarnRect[msgextInCtrl->SensorStat][0].x = recttmp.x;
		pThis->polWarnRect[msgextInCtrl->SensorStat][0].y = recttmp.y;
		pThis->polWarnRect[msgextInCtrl->SensorStat][1].x = recttmp.x+recttmp.w;
		pThis->polWarnRect[msgextInCtrl->SensorStat][1].y = recttmp.y;
		pThis->polWarnRect[msgextInCtrl->SensorStat][2].x = recttmp.x+recttmp.w;
		pThis->polWarnRect[msgextInCtrl->SensorStat][2].y = recttmp.y+recttmp.h;
		pThis->polWarnRect[msgextInCtrl->SensorStat][3].x = recttmp.x;
		pThis->polWarnRect[msgextInCtrl->SensorStat][3].y = recttmp.y+recttmp.h;
		pThis->polwarn_count[msgextInCtrl->SensorStat] = 4;

		polyWarnRoi[0]	= cv::Point(recttmp.x,recttmp.y);
		polyWarnRoi[1]	= cv::Point(recttmp.x+recttmp.w,recttmp.y);
		polyWarnRoi[2]	= cv::Point(recttmp.x+recttmp.w,recttmp.y+recttmp.h);
		polyWarnRoi[3]	= cv::Point(recttmp.x,recttmp.y+recttmp.h);
		
		m_pMovDetector->setWarningRoi(polyWarnRoi, msgextInCtrl->SensorStat);

}

void	CVideoProcess::DeInitMvDetect()
{
	if(m_pMovDetector != NULL)
		m_pMovDetector->destroy();
}

void CVideoProcess::NotifyFunc(void *context, int chId)
{
	CVideoProcess *pParent = (CVideoProcess*)context;
	pThis->detect_vect.clear();
	pThis->m_pMovDetector->getWarnTarget(pThis->detect_vect,chId);
	
	//pParent->m_display.m_bOsd = true;
	//pThis->m_display.UpDateOsd(0);
}
#endif

void CVideoProcess::getImgRioDelta(unsigned char* pdata,int width ,int height,UTC_Rect rio,double * value)
{
	unsigned char * ptr = pdata;
	int Iij;
	//gray max ,gray min ,gray average,gray delta
	double Imax = 0, Imin = 255, Iave = 0, Idelta = 0;
	for(int i=rio.y;i<rio.height+rio.y;i++)
	{
		for(int j=rio.x;j<rio.width+rio.x;j++)
		{
			Iij	= ptr[i*width+j];
			if(Iij > Imax)
				Imax = Iij;
			if(Iij < Imin)
				Imin = Iij;
			Iave = Iave + Iij;
		}
	}
	Iave = Iave/(rio.width*rio.height);
	for(int i=rio.y;i<rio.height+rio.y;i++)
	{
		for(int j=rio.x;j<rio.width+rio.x;j++)
		{
			Iij 	=  ptr[i*width+j];
			Idelta = Idelta + (Iij-Iave)*(Iij-Iave);
		}
	}
	Idelta = Idelta/(rio.width*rio.height);
	*value = Idelta;
}


void CVideoProcess::detectcall(vector<BoundingBox>& algbox)
{

	pThis->m_algbox.clear();
	OSA_mutexLock(&pThis->m_algboxLock);
	pThis->m_algbox.insert(pThis->m_algbox.begin(),algbox.begin(),algbox.end());
	OSA_mutexUnlock(&pThis->m_algboxLock);
}

void CVideoProcess::trackcall(vector<BoundingBox>& trackbox)
{
	pThis->m_trackbox.clear();
	OSA_mutexLock(&pThis->m_trackboxLock);
	pThis->m_trackbox.insert(pThis->m_trackbox.begin(),trackbox.begin(),trackbox.end());
	OSA_mutexUnlock(&pThis->m_trackboxLock);
}





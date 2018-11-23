

#include <glut.h>
#include "VideoProcess.hpp"
#include "vmath.h"
#include "arm_neon.h"

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "app_ctrl.h"
#include "Ipcctl.h"

using namespace vmath;
extern CMD_EXT *msgextInCtrl;
int CVideoProcess::m_mouseEvent = 0;
int CVideoProcess::m_mousex = 0;
int CVideoProcess::m_mousey = 0;
CVideoProcess * CVideoProcess::pThis = NULL;
bool CVideoProcess::m_bTrack = false;
bool CVideoProcess::m_bMtd = false;
bool CVideoProcess::m_bMoveDetect = false;
int CVideoProcess::m_iTrackStat = 0;
int CVideoProcess::m_iTrackLostCnt = 0;
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

void CVideoProcess::main_proc_func()
{
	OSA_printf("%s: Main Proc Tsk Is Entering...\n",__func__);
	unsigned int framecount=0;
	
	float speedx,speedy;
	float optValue;
	UTC_Rect AcqRect;
	
	static bool Movedetect=false;
	Point pt1,pt2,erspt1,erspt2,erspt3,erspt4;
	static UTC_ACQ_param acqRect;
	CMD_EXT tmpCmd={0};
	double value;

#if 1
	
	static int timeFlag = 2;
	static int speedcount = 0;

	time_t timep;  
	struct tm *p;  	
	char file[128];
	const int MvDetectAcqRectWidth  =  80;
	const int MvDetectAcqRectHeight =  80;
	
#endif
	while(mainProcThrObj.exitProcThread ==  false)
	{
		OSA_semWait(&mainProcThrObj.procNotifySem, OSA_TIMEOUT_FOREVER);

		Mat frame = mainFrame[mainProcThrObj.pp^1];
		bool bTrack = mainProcThrObj.cxt[mainProcThrObj.pp^1].bTrack;
		bool bMtd = mainProcThrObj.cxt[mainProcThrObj.pp^1].bMtd;
		bool bMoveDetect = mainProcThrObj.cxt[mainProcThrObj.pp^1].bMoveDetect;
		int chId = mainProcThrObj.cxt[mainProcThrObj.pp^1].chId;
		int iTrackStat = mainProcThrObj.cxt[mainProcThrObj.pp^1].iTrackStat;

		int channel = frame.channels();
		Mat frame_gray;

		cv::Mat	salientMap, sobelMap;

		mainProcThrObj.pp ^=1;
		if(!OnPreProcess(chId, frame))
			continue;

		if(!m_bTrack && !m_bMtd &&!m_bMoveDetect){
			OnProcess(chId, frame);
			continue;
		}

	#if LINKAGE_FUNC
		if(chId != m_curSubChId)
			continue;
	#else
		if(chId != m_curChId)
			continue;
	#endif
	
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
				m_pMovDetector->setFrame(frame_gray,0,2,minsize,maxsize,16);
			}
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

#if LINKAGE_FUNC
void CVideoProcess::linkage_init()
{
	m_GrayMat.create(1080,1920,CV_8UC1);
	if(m_GrayMat.empty()) 
		cout << "Create m_GrayMat Failed !!" << endl;
	else
		cout << "Create m_GrayMat Success !!" << endl;
	
	m_Gun_GrayMat.create(1080,1920,CV_8UC1);
	if(m_Gun_GrayMat.empty())
		cout << "Create m_Gun_GrayMat Failed !!" << endl;
	else
		cout << "Create m_Gun_GrayMat Success !!" << endl;
	
	m_Gun_GrayMat = Scalar(255);

	m_time_show = m_time_flag = 0;
	
	m_rgbMat.create(1080,1920,CV_8UC3);
	if( m_rgbMat.empty())
		cout << "Create m_rgbMat Failed !!" << endl;
	else
		cout << "Create m_rgbMat Success !!" << endl;
	
	if(m_camCalibra == NULL)
		cout << "Create CamCalibrate Object Failed !!" << endl;
	
	if( !m_camCalibra->Load_CameraParams("gun_camera_data.yml", "ball_camera_data.yml") )
		cout << " Load Camera Origin IntrinsicParameters Failed !!!" << endl;

	m_camCalibra->Init_CameraParams();
	m_camCalibra->RunService();
}

CcCamCalibra* CVideoProcess::m_camCalibra = new CcCamCalibra();

#endif

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

#if (!LINKAGE_FUNC)
	mptz_click = mptz_originX = mptz_originY = 0;
#endif
	
#if __MOVE_DETECT__
	m_pMovDetector	=NULL;
#endif

#if __MMT__
	memset(m_tgtBox, 0, sizeof(TARGETBOX)*MAX_TARGET_NUMBER);
#endif

#if __MOVE_DETECT__
	detectNum = 10;
	maxsize = 50000;
	minsize = 1000;
#endif

#if LINKAGE_FUNC
	m_curChId = video_gaoqing ;
	m_curSubChId = video_gaoqing0 ;
	Set_SelectByRect = false ;
	open_handleCalibra = false ;
	linkage_init();
#endif
	m_click = m_draw = m_tempX = m_tempY = 0;
	memset(m_rectn, 0, sizeof(m_rectn));
	memset(mRect, 0, sizeof(mRect));
	setrigon_flag = 0;

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
	OnCreate();

	
#if __MOVE_DETECT__
	if(m_pMovDetector == NULL)
		m_pMovDetector = MvDetector_Create();
	OSA_assert(m_pMovDetector != NULL);
#endif
	return 0;
}

int CVideoProcess::destroy()
{
	stop();
	OSA_mutexDelete(&m_mutex);
	MAIN_threadDestroy();

	MultiCh.destroy();
	m_display.destroy();

	OnDestroy();

#if __MOVE_DETECT__
	DeInitMvDetect();
#endif

	return 0;
}

#if LINKAGE_FUNC	
void CVideoProcess::processtimeMenu(int value)
{
	if(0 == value)
	{
		pThis->m_time_show = 1;
		pThis->m_time_flag = 1;
	}
	else if(1 == value)
		pThis->m_time_show = 0;
}

void CVideoProcess::processsmanualcarliMenu(int value)
{
	printf("%s start, value=%d\n", __FUNCTION__, value);
}

void CVideoProcess::processsautocarliMenu(int value)
{
	printf("%s start, value=%d\n", __FUNCTION__, value);
}

int CVideoProcess::click_legal(int x, int y)
{
	y = 1080 - y;
	if(in_gun_area(x, y))
	{
		click_in_area = 1;	//click in gun area
		return 1;
	}
	else if(in_ball_area(x, y))
	{
		click_in_area = 2; //click in ball area
		return 1;
	}
	else 
	{
		click_in_area = 0; //click in illegal area
		return 0;
	}
}

int CVideoProcess::move_legal(int x, int y)
{
	y = 1080 - y;
	if(1 == click_in_area)
	{
		if(in_gun_area(x, y))
			return 1;
		else
			return 0;
	}
	else if(2 == click_in_area)
	{
		if(in_ball_area(x, y))
			return 1;
		else
			return 0;
	}
}

int CVideoProcess::in_gun_area(int x, int y)
{
	int dismode = m_display.displayMode;
	switch(dismode)
	{
		case PREVIEW_MODE:
			if((x > 960 && x < 1920) && (y > 540 && y < 1080))
				return 1;
			else
				return 0;
			break;
		case PIC_IN_PIC:
			if(((x > 0 && x < 1440) && (y > 0 && y < 1080)) ||((x > 1440 && x < 1920) && (y > 0 && y < 810)))
				return 1;
			else
				return 0;
			break;
		case SIDE_BY_SIDE:
			if((x > 960 && x < 1920) && (y > 0 && y < 1080))
				return 1;
			else
				return 0;
			break;
		case LEFT_BALL_RIGHT_GUN:
			if((x > 480 && x < 1920) && (y > 270 && y < 1080))
				return 1;
			else
				return 0;
			break;
		default:
			printf("%s,%d, unknown displayMode:%d\n", __FILE__, __LINE__, dismode);
			return 0;
			break;
	}
}

int CVideoProcess::in_ball_area(int x, int y)
{
	int dismode = m_display.displayMode;
	switch(dismode)
	{
		case PREVIEW_MODE:
			if((x > 0 && x < 960) && (y > 540 && y < 1080))
				return 1;
			else
				return 0;
			break;
		case PIC_IN_PIC:
			if((x > 1440 && x < 1920) && (y > 810 && y < 1080))
				return 1;
			else
				return 0;
			break;
		case SIDE_BY_SIDE:
			if((x > 0 && x < 960) && (y > 0 && y < 1080))
				return 1;
			else
				return 0;
			break;
		case LEFT_BALL_RIGHT_GUN:
			if((x > 0 && x < 480) && (y > 810 && y < 1080))
				return 1;
			else
				return 0;
			break;
		default:
			printf("%s,%d, unknown displayMode:%d\n", __FILE__, __LINE__, dismode);
			return 0;
			break;
	}
}

mouserect CVideoProcess::map2preview(mouserect rectcur)
{
	int dismode = m_display.displayMode;
	switch(dismode)
	{
		case PREVIEW_MODE:
			return rectcur;
			break;
		case PIC_IN_PIC:
			return mappip2preview(rectcur);
			break;
		case SIDE_BY_SIDE:
			return mapsbs2preview(rectcur);
			break;
		case LEFT_BALL_RIGHT_GUN:
			return maplbrg2preview(rectcur);
			break;
		default:
			printf("%s,%d, unknown displayMode:%d\n", __FILE__, __LINE__, dismode);
			return rectcur;
			break;			
	}
}

mouserect CVideoProcess::mappip2preview(mouserect rectcur)
{
	mouserect rectpip;
	mouserect rectpreview;
	if(1 == click_in_area)
	{
		rectpip.x = 0;
		rectpip.y = 0;
		rectpip.w = 1920;
		rectpip.h = 1080;
		
		rectpreview.x = 960;
		rectpreview.y = 0;
		rectpreview.w = 960;
		rectpreview.h = 540;
	}
	else if(2 == click_in_area)
	{
	
		rectpip.x = 1440;
		rectpip.y = 0;
		rectpip.w = 480;
		rectpip.h = 270;
		
		rectpreview.x = 0;
		rectpreview.y = 0;
		rectpreview.w = 960;
		rectpreview.h = 540;
	}

	return maprect(rectcur, rectpip, rectpreview);
}

mouserect CVideoProcess::mapsbs2preview(mouserect rectcur)
{
	mouserect rectsbs;
	mouserect rectpreview;
	if(1 == click_in_area)
	{
		rectsbs.x = 960;
		rectsbs.y = 0;
		rectsbs.w = 960;
		rectsbs.h = 1080;
		
		rectpreview.x = 960;
		rectpreview.y = 0;
		rectpreview.w = 960;
		rectpreview.h = 540;
	}
	else if(2 == click_in_area)
	{
		rectsbs.x = 0;
		rectsbs.y = 0;
		rectsbs.w = 960;
		rectsbs.h = 1080;
		
		rectpreview.x = 0;
		rectpreview.y = 0;
		rectpreview.w = 960;
		rectpreview.h = 540;
	}

	return maprect(rectcur, rectsbs, rectpreview);
}

mouserect CVideoProcess::maplbrg2preview(mouserect rectcur)
{
	mouserect rectlbrg;
	mouserect rectpreview;
	if(1 == click_in_area)
	{
		rectlbrg.x = 480;
		rectlbrg.y = 0;
		rectlbrg.w = 1440;
		rectlbrg.h = 810;
		
		rectpreview.x = 960;
		rectpreview.y = 0;
		rectpreview.w = 960;
		rectpreview.h = 540;
	}
	else if(2 == click_in_area)
	{
		rectlbrg.x = 0;
		rectlbrg.y = 0;
		rectlbrg.w = 480;
		rectlbrg.h = 270;
		
		rectpreview.x = 0;
		rectpreview.y = 0;
		rectpreview.w = 960;
		rectpreview.h = 540;
	}

	return maprect(rectcur, rectlbrg, rectpreview);
}

mouserect CVideoProcess::mapfullscreen2gun(mouserect rectcur)
{
	mouserect rect1080p;
	mouserect rectgun;
	
	int dismode = m_display.displayMode;
	rect1080p.x = 0;
	rect1080p.y = 0;
	rect1080p.w = 1920;
	rect1080p.h = 1080;
		
	switch(dismode)
	{
		case PREVIEW_MODE:
			rectgun.x = 960;
			rectgun.y = 0;
			rectgun.w = 960;
			rectgun.h = 540;
			break;
		case PIC_IN_PIC:
			rectgun.x = 0;
			rectgun.y = 0;
			rectgun.w = 1920;
			rectgun.h = 1080;
			break;
		case SIDE_BY_SIDE:
			rectgun.x = 960;
			rectgun.y = 0;
			rectgun.w = 960;
			rectgun.h = 1080;
			break;
		case LEFT_BALL_RIGHT_GUN:
			rectgun.x = 480;
			rectgun.y = 0;
			rectgun.w = 1440;
			rectgun.h = 810;
			break;
		default:
			break;

	}
	
	return maprect(rectcur, rect1080p, rectgun);
}

mouserect CVideoProcess::mapgun2fullscreen(mouserect rectcur)
{
	mouserect rect1080p;
	mouserect rectgun;
	
	int dismode = m_display.displayMode;
	rect1080p.x = 0;
	rect1080p.y = 0;
	rect1080p.w = 1920;
	rect1080p.h = 1080;
		
	switch(dismode)
	{
		case PREVIEW_MODE:
			rectgun.x = 960;
			rectgun.y = 0;
			rectgun.w = 960;
			rectgun.h = 540;
			break;
		case PIC_IN_PIC:
			rectgun.x = 0;
			rectgun.y = 0;
			rectgun.w = 1920;
			rectgun.h = 1080;
			break;
		case SIDE_BY_SIDE:
			rectgun.x = 960;
			rectgun.y = 0;
			rectgun.w = 960;
			rectgun.h = 1080;
			break;
		case LEFT_BALL_RIGHT_GUN:
			rectgun.x = 480;
			rectgun.y = 0;
			rectgun.w = 1440;
			rectgun.h = 810;
			break;
		default:
			break;

	}
	
	return maprect(rectcur, rectgun, rect1080p);
}

mouserect CVideoProcess::maprect(mouserect rectcur,mouserect rectsrc,mouserect rectdest)
{
	mouserect rect_result;

	rect_result.x = (rectcur.x-rectsrc.x)*rectdest.w/rectsrc.w+rectdest.x;
	rect_result.y = (rectcur.y-rectsrc.y)*rectdest.h/rectsrc.h+rectdest.y;
	rect_result.w = rectcur.w*rectdest.w/rectsrc.w;
	rect_result.h = rectcur.h*rectdest.h/rectsrc.h;
	return rect_result;
}
#else

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
#endif





void CVideoProcess::mouse_event(int button, int state, int x, int y)
{
	unsigned int curId;
#if LINKAGE_FUNC
	if(pThis->m_display.g_CurDisplayMode == PIC_IN_PIC) {
		curId = 0;	
	}else{
		curId = pThis->m_curChId;
	}
#else
		curId = pThis->m_curChId;
#endif	
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
#if LINKAGE_FUNC

		//pThis->OnMouseLeftDwn(x, y);   // add by swj
		
		if(pThis->open_handleCalibra) // Press 'y' or 'Y' , set this flag to 1
		{
			pThis->OnMouseLeftDwn(x, y);
		}
		else
#endif
		{
			if( pThis->setrigon_flag && !m_bMoveDetect)
			{
				if(pThis->m_click == 0)
				{
					pThis->m_click = 1;
					pThis->mRect[curId][pThis->m_rectn[curId]].x1 = x;
					pThis->mRect[curId][pThis->m_rectn[curId]].y1 = y;
				}
				else
				{	
					pThis->m_click = 0;
					pThis->mRect[curId][pThis->m_rectn[curId]].x2 = x;
					pThis->mRect[curId][pThis->m_rectn[curId]].y2 = y;

				printf("Rigion%d: Point1(%d,%d),Point2(%d,%d),Point3(%d,%d),Point4(%d,%d)\n",
					pThis->m_rectn[curId],
					pThis->mRect[curId][pThis->m_rectn[curId]].x1,pThis->mRect[curId][pThis->m_rectn[curId]].y1,
					pThis->mRect[curId][pThis->m_rectn[curId]].x2,pThis->mRect[curId][pThis->m_rectn[curId]].y1,
					pThis->mRect[curId][pThis->m_rectn[curId]].x1,pThis->mRect[curId][pThis->m_rectn[curId]].y2,
					pThis->mRect[curId][pThis->m_rectn[curId]].x2,pThis->mRect[curId][pThis->m_rectn[curId]].y2
				);
					//point1  ---  lefttop    ,  point2  --- righttop  , point3 --- leftbottom  ,point --- rightbottom

					std::vector<cv::Point> polyWarnRoi ;
					polyWarnRoi.resize(4);
					polyWarnRoi[0]	= cv::Point(pThis->mRect[curId][pThis->m_rectn[curId]].x1,pThis->mRect[curId][pThis->m_rectn[curId]].y1);
					polyWarnRoi[1]	= cv::Point(pThis->mRect[curId][pThis->m_rectn[curId]].x2,pThis->mRect[curId][pThis->m_rectn[curId]].y1);
					polyWarnRoi[2]	= cv::Point(pThis->mRect[curId][pThis->m_rectn[curId]].x2,pThis->mRect[curId][pThis->m_rectn[curId]].y2);
					polyWarnRoi[3]	= cv::Point(pThis->mRect[curId][pThis->m_rectn[curId]].x1,pThis->mRect[curId][pThis->m_rectn[curId]].y2);

					pThis->preWarnRect.x = polyWarnRoi[0].x;
					pThis->preWarnRect.y = polyWarnRoi[0].y;
					pThis->preWarnRect.width = polyWarnRoi[2].x - polyWarnRoi[0].x;
					pThis->preWarnRect.height = polyWarnRoi[2].y - polyWarnRoi[0].y;	
					pThis->m_pMovDetector->setWarningRoi( polyWarnRoi,	0);
								
					
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
			else
			{
				#if LINKAGE_FUNC
					if(pThis->m_click == 0)
					{
						if(pThis->click_legal(x,y))
						{
							pThis->LeftPoint.x = x;
							pThis->LeftPoint.y = y;

							pThis->m_click = 1;
							pThis->m_rectn[curId] = 0;
							pThis->mRect[curId][pThis->m_rectn[curId]].x1 = x;
							pThis->mRect[curId][pThis->m_rectn[curId]].y1 = y;
							cout<<" start:("<<pThis->mRect[curId][pThis->m_rectn[curId]].x1<<","<<pThis->mRect[curId][pThis->m_rectn[curId]].y1<<")"<<endl;
						}
						else
							printf("click illegal!!!\n");
					}
					else
					{
						if(pThis->move_legal(x,y))
						{
						
							pThis->RightPoint.x = x;
							pThis->RightPoint.y = y;

							pThis->m_click = 0;
							pThis->mRect[curId][pThis->m_rectn[curId]].x2 = x;
							pThis->mRect[curId][pThis->m_rectn[curId]].y2 = y;
							cout<<" end:("<<pThis->mRect[curId][pThis->m_rectn[curId]].x2<<","<<pThis->mRect[curId][pThis->m_rectn[curId]].y2<<")\n"<<endl;

							mouserect rectsrc, recvdest;
							rectsrc.x = pThis->mRect[curId][pThis->m_rectn[curId]].x1;
							rectsrc.y = pThis->mRect[curId][pThis->m_rectn[curId]].y1;
							rectsrc.w = pThis->mRect[curId][pThis->m_rectn[curId]].x2 - pThis->mRect[curId][pThis->m_rectn[curId]].x1;
							rectsrc.h = pThis->mRect[curId][pThis->m_rectn[curId]].y2 - pThis->mRect[curId][pThis->m_rectn[curId]].y1;
							printf("###before map(%d,%d,%d,%d)\n",rectsrc.x,rectsrc.y,rectsrc.w,rectsrc.h);
							recvdest = pThis->map2preview(rectsrc);
							printf("###after map(%d,%d,%d,%d)\n",recvdest.x,recvdest.y,recvdest.w,recvdest.h);

							
							pThis->m_rectn[curId]++;  
							if(pThis->m_rectn[curId]>=sizeof(pThis->mRect[0]))
							{
								printf("mouse rect reached maxnum:100!\n");
								pThis->m_rectn[curId]--;
							}
							pThis->m_draw = 1;	

						switch( pThis->m_display.g_CurDisplayMode){

							case PREVIEW_MODE:
							if(x > 960) {							
								pThis->reMapCoords(x,y,true);								
							}
							else {
								pThis->moveToDest();
							}
							break;
						case LEFT_BALL_RIGHT_GUN:
							
							break;
						default:
							break;
						}
							
						}
						else
							printf("move illegal!!!\n");
					}
					
				#endif
			}
		}
		
	}

	if((button == 3)||(button == 4))
	{
		pThis->m_click = 0;
		pThis->m_rectn[curId] = 0;
		pThis->m_draw = 1;
	}
#if (!LINKAGE_FUNC)
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
#endif
}

void CVideoProcess::mousemove_event(GLint xMouse, GLint yMouse)
{
	if(pThis->m_click == 1)
	{
		pThis->m_tempX = xMouse;
		pThis->m_tempY = yMouse;
		pThis->m_draw = 1;
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
	printf("set rigion\n");
	pThis->setrigon_flag = 1;
}

void CVideoProcess::processrigionselMenu(int value)
{
	printf("%s start, value=%d\n", __FUNCTION__, value);
}

#if __MOVE_DETECT__
void CVideoProcess::processmaxnumMenu(int value)
{
	if(0 == value)
		pThis->detectNum = 5;
	else if(1 == value)
		pThis->detectNum = 10;
}

void CVideoProcess::processmaxtargetsizeMenu(int value)
{
	if(0 == value)
		pThis->maxsize= 40000;
	else if(1 == value)
		pThis->maxsize= 50000;
}

void CVideoProcess::processmintargetsizeMenu(int value)
{
	if(0 == value)
		pThis->minsize= 100;
	else if(1 == value)
		pThis->minsize= 1000;
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
#if LINKAGE_FUNC
	dsInit.timefunc = processtimeMenu;	
	dsInit.manualcarli = processsmanualcarliMenu;
	dsInit.autocarli = processsautocarliMenu;
	
#else
	dsInit.motionfunc = mousemotion_event;
#endif
	dsInit.menufunc = menu_event;
	dsInit.mousefunc = mouse_event;
	dsInit.passivemotionfunc = mousemove_event;
	dsInit.setrigion = processrigionMenu;
	dsInit.rigionsel = processrigionselMenu;
#if __MOVE_DETECT__
	dsInit.maxnum = processmaxnumMenu;
	dsInit.maxsize= processmaxtargetsizeMenu;
	dsInit.minsize= processmintargetsizeMenu;
#endif
	

//#if (!__IPC__)
	dsInit.keyboardfunc = keyboard_event; // keySpecial_event
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
	m_display.m_crossOsd = true;
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

#if LINKAGE_FUNC
	static bool copy_once = true;
#endif
	

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


#if LINKAGE_FUNC
	if(chId == m_curSubChId)
#else
	if(chId == m_curChId)
#endif
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
			mainProcThrObj.cxt[mainProcThrObj.pp].chId = chId;
			if(mainProcThrObj.bFirst){
				mainFrame[mainProcThrObj.pp^1] = frame;
				mainProcThrObj.cxt[mainProcThrObj.pp^1].bTrack = m_bTrack;
				mainProcThrObj.cxt[mainProcThrObj.pp^1].bMtd = m_bMtd;
				mainProcThrObj.cxt[mainProcThrObj.pp^1].bMoveDetect = m_bMoveDetect;
				mainProcThrObj.cxt[mainProcThrObj.pp^1].iTrackStat = m_iTrackStat;
				mainProcThrObj.cxt[mainProcThrObj.pp^1].chId = chId;
				mainProcThrObj.bFirst = false;
			}
			OSA_semSignal(&mainProcThrObj.procNotifySem);
		}
	}




	#if LINKAGE_FUNC
	
			if(m_camCalibra->start_cloneVideoSrc == true) 
			{
				//m_camCalibra->start_cloneVideoSrc = false;
				//printf("%s : cloneVideoSrc \n",__func__);
				#if !GUN_IMAGE_USEBMP
					if(chId == GUN_CHID){
						m_camCalibra->cloneGunSrcImgae(frame);
						m_camCalibra->cvtGunYuyv2Bgr();
					}
				#endif
					
				if( chId == BALL_CHID )	{	
					m_camCalibra->cloneBallSrcImgae(frame);
					m_camCalibra->cvtBallYuyv2Bgr();
				}
			}
			
			if(  chId == 0 && copy_once == true) {
				if(!frame.empty()) {
					frame.copyTo(gun_srcMat_remap);
					copy_once = false;
				}
			}
	#endif
	
		
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
	

#if 0
	if(pChPrm != NULL && (pChPrm->state > 0))
	{
//		medium.create(frame_gray.rows, frame_gray.cols, frame_gray.type());

//		MediumFliter(frame_gray.data, medium.data, pChPrm->i_width, pChPrm->i_height);

		GaussFliter(frame_gray.data, pChPrm->Img[0], pChPrm->i_width, pChPrm->i_height);
		

		for(i = 0; i < MAX_SCALER; i++)
		{
			DownSample(pChPrm, pChPrm->Img[i], pChPrm->Img[i+1],
										pChPrm->i_width>>i, pChPrm->i_height>>i);
		}

		IMG_sobel(pChPrm->Img[1], pChPrm->sobel, pChPrm->i_width>>1,	 pChPrm->i_height>>1);

		AutoDetectTarget(pChPrm, frame_gray.data);

		FilterMultiTarget(pChPrm);

	}
#endif
	return 0;
}

#if __MOVE_DETECT__
void	CVideoProcess::initMvDetect()
{
	int	i;
	OSA_printf("%s:mvDetect start ", __func__);
	OSA_assert(m_pMovDetector != NULL);

	m_pMovDetector->init(NotifyFunc, (void*)this);
	
	std::vector<cv::Point> polyWarnRoi ;
	polyWarnRoi.resize(4);
    polyWarnRoi[0]	= cv::Point(200,200);
    polyWarnRoi[1]	= cv::Point(1720,200);
    polyWarnRoi[2]	= cv::Point(1720,880);
    polyWarnRoi[3]	= cv::Point(200,880);


	preWarnRect.x = polyWarnRoi[0].x;
	preWarnRect.y = polyWarnRoi[0].y;
	preWarnRect.width = polyWarnRoi[2].x - polyWarnRoi[0].x;
	preWarnRect.height = polyWarnRoi[2].y - polyWarnRoi[0].y;

	for(i=0; i<DETECTOR_NUM; i++)
	{
		//m_pMovDetector->setDrawOSD(pThis->m_display.m_disOsd[1], i);
		//m_pMovDetector->enableSelfDraw(true, i);

		m_pMovDetector->setWarnMode(WARN_WARN_MODE, i);
		m_pMovDetector->setWarningRoi(polyWarnRoi,	i);
	
	} 
}

void	CVideoProcess::DeInitMvDetect()
{
	if(m_pMovDetector != NULL)
		m_pMovDetector->destroy();
}

void CVideoProcess::NotifyFunc(void *context, int chId)
{
	SENDST test;
	test.cmd_ID = mtdnum;
	CVideoProcess *pParent = (CVideoProcess*)context;
	pThis->m_pMovDetector->getWarnTarget(pThis->detect_vect,0);

	if(0 == pThis->detect_vect.size())
		test.param[0] = 0;
	else
		test.param[0] = 1;
	ipc_sendmsg(&test, IPC_FRIMG_MSG);

	
	//pParent->m_display.m_bOsd = true;
	//pThis->m_display.UpDateOsd(1);
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



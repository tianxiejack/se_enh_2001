
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.hpp>
#include <opencv2/opencv.hpp>

//#include <gl.h>
#include "glew.h"
#include <glut.h>
#include <freeglut_ext.h>
#include <cuda.h>
#include <cuda_gl_interop.h>
#include "cuda_runtime_api.h"
#include "osa.h"
#include "osa_mutex.h"
#include "osa_tsk.h"
#include "Displayer.hpp"
#include "enh.hpp"

#include "cuda_mem.cpp"
#include "app_status.h"

#include "osd_text.hpp"
#include "string.h"
#include "process51.hpp"
#include "locale.h"
#include "arm_neon.h"

#define HISTEN 0
#define CLAHEH 1
#define DARKEN 0

static CDisplayer *gThis = NULL;
extern CProcess* plat;
extern bool sceneLost;


double capTime = 0;

GLint Uniform_tex_in = -1;
GLint Uniform_tex_osd = -1;
GLint Uniform_tex_pbo = -1;
GLint Uniform_osd_enable = -1;
GLint Uniform_mattrans = -1;
GLint Uniform_font_color = -1;
static GLfloat m_glmat44fTransDefault[16] ={
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
};
static GLfloat m_glmat44fTransDefault2[16] ={
	1.1f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.1f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
};

GLfloat _fontColor[4] = {1.0,1.0,1.0,1.0};

static GLfloat m_glvVertsDefault[8] = {-1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f};
static GLfloat m_glvTexCoordsDefault[8] = {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};

OSD_CONFIG_USER gCFG_Osd = {0};
OSDSTATUS gSYS_Osd = {0};

CDisplayer::CDisplayer()
:m_renderCount(0),m_bRun(false),m_bFullScreen(false),m_bOsd(false),
 m_glProgram(0), m_bUpdateVertex(false),m_tmRender(0ul),m_waitSync(false),
 m_telapse(5.0), m_nSwapTimeOut(0),tend(0ul)
{
	int i;
	gThis = this;
	memset(&m_initPrm, 0, sizeof(m_initPrm));
	memset(cuda_pbo_resource, 0, sizeof(cuda_pbo_resource));
	//memset(m_dev_src_rgb, 0, sizeof(m_dev_src_rgb));
	memset(m_cuStream, 0, sizeof(m_cuStream));
	memset(m_bEnh, 0, sizeof(m_bEnh));
	memset(m_Mmt, 0, sizeof(m_Mmt));
	for(i=0; i<DS_DC_CNT; i++)
		buffId_osd[i] = -1;
	memset(updata_osd, 0, sizeof(updata_osd));
	dismodchanagcount=0;
	tv_pribuffid0=-1;
	tv_pribuffid1=-1;
	tv_pribuffid2=-1;
	tv_pribuffid3=-1;
	fir_pribuffid=-1;
	freezeonece=0;

	frameCount = 0;
	frameRate = 0.0;

	m_initPrm.disSched = 3.5;
}

CDisplayer::~CDisplayer()
{
	//destroy();
	gThis = NULL;
}

inline int extractYUYV2Gray2(Mat src, Mat dst)
{
	int ImgHeight, ImgWidth,ImgStride;

	ImgWidth = src.cols;
	ImgHeight = src.rows;
	ImgStride = ImgWidth*2;
	uint8_t  *  pDst8_t;
	uint8_t *  pSrc8_t;

	pSrc8_t = (uint8_t*)(src.data);
	pDst8_t = (uint8_t*)(dst.data);

	for(int y = 0; y < ImgHeight*ImgWidth; y++)
	{
		pDst8_t[y] = pSrc8_t[y*2];
	}
}

int CDisplayer::create()
{
	int i;
	unsigned char *d_src_rgb_novideo = NULL;
	cudaError_t et;
	memset(m_renders, 0, sizeof(m_renders));
	for(i=0; i<4; i++){
		memcpy(m_renders[i].transform, m_glmat44fTransDefault, sizeof(float)*16);
	}
	for(; i<DS_RENDER_MAX; i++){
		memcpy(m_renders[i].transform, m_glmat44fTransDefault2, sizeof(float)*16);
	}
	memset(m_videoSize, 0, sizeof(m_videoSize));

	for(i=0; i<DS_CHAN_MAX; i++){
		m_img[i].cols = 0;
		m_img[i].rows = 0;
	}
	m_img_novideo.cols=0;
	m_img_novideo.rows=0;

	for(i=0; i<DS_CUSTREAM_CNT; i++){
		et = cudaStreamCreate(&m_cuStream[i]);
		OSA_assert(et == cudaSuccess);
	}

	gl_create();

	for(i=0; i<DS_CHAN_MAX; i++){
		Videoeable[i]=1;
	}
	//tvvideo=1;
	//firvideo=1;
	unsigned int byteCount = mallocwidth * mallocheight* 3 * sizeof(unsigned char);
	
	cudaMalloc_share((void**)&d_src_rgb_novideo, byteCount, 5 + DS_CHAN_MAX);
	m_img_novideo= cv::Mat(mallocheight,mallocwidth, CV_8UC3, d_src_rgb_novideo);

	cudaMalloc((void **)&d_src_rgb_novideo,byteCount);
	m_img_novideo= cv::Mat(mallocheight,mallocwidth, CV_8UC3, d_src_rgb_novideo);

	OSA_mutexCreate(&m_mutex);

	return 0;
}

int CDisplayer::destroy()
{
	int i;
	cudaError_t et;

	stop();

	gl_destroy();

	uninitRender();

	OSA_mutexDelete(&m_mutex);

	for(i=0; i<SHAREARRAY_CNT; i++)
		cudaFree_share(NULL, i);

	for(i=0; i<RESOURCE_CNT; i++){
		cudaResource_unmapBuffer(i);
		cudaResource_UnregisterBuffer(i);
	}

	for(i=0; i<DS_CUSTREAM_CNT; i++){
		if(m_cuStream[i] != NULL){
			et = cudaStreamDestroy(m_cuStream[i]);
			OSA_assert(et == cudaSuccess);
			m_cuStream[i] = NULL;
		}
	}

	pthread_mutex_lock(&render_lock);
	pthread_cond_broadcast(&render_cond);
	pthread_mutex_unlock(&render_lock);

	pthread_mutex_destroy(&render_lock);
	pthread_cond_destroy(&render_cond);

	return 0;
}

int CDisplayer::initRender(bool bInitBind)
{	
	

	m_renderCount = 2;
#if 0
	m_renders[0].croprect.x=0;
	m_renders[0].croprect.y=0;
	m_renders[0].croprect.w=0;
	m_renders[0].croprect.h=0;
#endif
	

	//m_renders[0].videodect=1;
	

	for(int chId=0; chId<DS_CHAN_MAX; chId++)
	{
		m_img[chId].cols =0 ;
		m_img[chId].rows=0;

		m_renders[chId].videodect=0;

		m_renders[chId].croprect.x=0;
		m_renders[chId].croprect.y=0;
		m_renders[chId].croprect.w=0;
		m_renders[chId].croprect.h=0;

		m_renders[chId].video_chId = -1;
		m_renders[chId].displayrect.x = 0;
		m_renders[chId].displayrect.y = 0;
		m_renders[chId].displayrect.w = VIDEO_DIS_WIDTH/2;
		m_renders[chId].displayrect.h =  VIDEO_DIS_HEIGHT/2;

		m_renders[chId].bFreeze=0;
	}


	m_renders[0].croprect.x=0;
	m_renders[0].croprect.y=0;
	m_renders[0].croprect.w=0;
	m_renders[0].croprect.h=0;
	
	m_renders[0].video_chId = m_initPrm.initMainchId;
	m_renders[0].displayrect.x = 0;
	m_renders[0].displayrect.y = 0;
	m_renders[0].displayrect.w = VIDEO_DIS_WIDTH;
	m_renders[0].displayrect.h = VIDEO_DIS_HEIGHT;
	m_renders[0].videodect=1;

	m_renders[1].video_chId = -1;
	m_renders[1].displayrect.x = VIDEO_DIS_WIDTH*2/3;
	m_renders[1].displayrect.y = VIDEO_DIS_HEIGHT*2/3;
	m_renders[1].displayrect.w = VIDEO_DIS_WIDTH/3;
	m_renders[1].displayrect.h = VIDEO_DIS_HEIGHT/3;
	m_renders[1].videodect=1;

	m_renders[2].videodect=1;
	m_renders[3].videodect=1;
	m_renders[4].videodect=1;

	
	m_img_novideo.cols=0;
	m_img_novideo.rows=0;
	
	return 0;
}

void CDisplayer::uninitRender()
{
	m_renderCount = 0;
}

void CDisplayer::_display(void)
{
	static unsigned int count = 0;

	gThis->gl_textureLoad();
	gThis->gl_display();
	
	count ++;
}

void CDisplayer::_timeFunc(int value)
{
	if(!gThis->m_bRun){
		return ;
	}
	gThis->_display();
}

void CDisplayer::_reshape(int width, int height)
{
	assert(gThis != NULL);
	glViewport(0, 0, width, height);
	gThis->m_mainWinWidth_new[video_pal] = width;
	gThis->m_mainWinHeight_new[video_pal] = height;
	gThis->m_mainWinWidth_new[video_gaoqing0] = width;
	gThis->m_mainWinHeight_new[video_gaoqing0] = height;
	gThis->m_mainWinWidth_new[video_gaoqing] = width;
	gThis->m_mainWinHeight_new[video_gaoqing] = height;
	gThis->m_mainWinWidth_new[video_gaoqing2] = width;
	gThis->m_mainWinHeight_new[video_gaoqing2] = height;
	gThis->m_mainWinWidth_new[video_gaoqing3] = width;
	gThis->m_mainWinHeight_new[video_gaoqing3] = height;
	gThis->initRender(false);
	gThis->gl_updateVertex();
	gThis->gl_resize();
}

void CDisplayer::processSenMenu(int value)
{
	printf("%s start, value=%d\n", __FUNCTION__, value);
}

void CDisplayer::processtargetspeedMenu(int value)
{
	printf("%s start, value=%d\n", __FUNCTION__, value);
}

void CDisplayer::processtargetdircMenu(int value)
{
	printf("%s start, value=%d\n", __FUNCTION__, value);
}

void CDisplayer::processdetectcondMenu(int value)
{
	printf("%s start, value=%d\n", __FUNCTION__, value);
}

void CDisplayer::processpolarMenu(int value)
{

}

void CDisplayer::processdurationMenu(int value)
{
		
}

void CDisplayer::processmtdmodeMenu(int value)
{

}

void CDisplayer::processredetectMenu(int value)
{

}

void CDisplayer::processalarmputMenu(int value)
{

}

void CDisplayer::gl_resize()
{
}

void CDisplayer::_close(void)
{
	if(gThis->m_initPrm.closefunc != NULL)
		gThis->m_initPrm.closefunc();
}

int CDisplayer::init(DS_InitPrm *pPrm)
{

	//for tv buffer
	tskSendBufCreatetv0.numBuf = PICBUFFERCOUNT;
       for (int i = 0; i < tskSendBufCreatetv0.numBuf; i++)
       {
           cudaMalloc((void **)&tskSendBufCreatetv0.bufVirtAddr[i],mallocwidth*mallocheight*3);
           OSA_assert(tskSendBufCreatetv0.bufVirtAddr[i] != NULL);
       }
       OSA_bufCreate(&tskSendBuftv0, &tskSendBufCreatetv0);

	tskSendBufCreatetv1.numBuf = PICBUFFERCOUNT;
       for (int i = 0; i < tskSendBufCreatetv1.numBuf; i++)
       {
           cudaMalloc((void **)&tskSendBufCreatetv1.bufVirtAddr[i],mallocwidth*mallocheight*3);
           OSA_assert(tskSendBufCreatetv1.bufVirtAddr[i] != NULL);
       }
       OSA_bufCreate(&tskSendBuftv1, &tskSendBufCreatetv1);

	tskSendBufCreatetv2.numBuf = PICBUFFERCOUNT;
       for (int i = 0; i < tskSendBufCreatetv2.numBuf; i++)
       {
           cudaMalloc((void **)&tskSendBufCreatetv2.bufVirtAddr[i],mallocwidth*mallocheight*3);
           OSA_assert(tskSendBufCreatetv2.bufVirtAddr[i] != NULL);
       }
       OSA_bufCreate(&tskSendBuftv2, &tskSendBufCreatetv2);

	tskSendBufCreatetv3.numBuf = PICBUFFERCOUNT;
       for (int i = 0; i < tskSendBufCreatetv3.numBuf; i++)
       {
           cudaMalloc((void **)&tskSendBufCreatetv3.bufVirtAddr[i],mallocwidth*mallocheight*3);
           OSA_assert(tskSendBufCreatetv3.bufVirtAddr[i] != NULL);
       }
       OSA_bufCreate(&tskSendBuftv3, &tskSendBufCreatetv3);
	   

	tskSendBufCreatefir.numBuf = PICBUFFERCOUNT;
       for (int i = 0; i < tskSendBufCreatefir.numBuf; i++)
       {
           cudaMalloc((void **)&tskSendBufCreatefir.bufVirtAddr[i],mallocwidth*mallocheight*3);
           OSA_assert(tskSendBufCreatefir.bufVirtAddr[i] != NULL);
       }
       OSA_bufCreate(&tskSendBuffir, &tskSendBufCreatefir);

	tskSendBufCreatepal.numBuf = PICBUFFERCOUNT;
       for (int i = 0; i < tskSendBufCreatepal.numBuf; i++)
       {
           cudaMalloc((void **)&tskSendBufCreatepal.bufVirtAddr[i],mallocwidth*mallocheight*3);
           OSA_assert(tskSendBufCreatepal.bufVirtAddr[i] != NULL);
          
       }
       OSA_bufCreate(&tskSendBufpal, &tskSendBufCreatepal);


	if(pPrm != NULL)
		memcpy(&m_initPrm, pPrm, sizeof(DS_InitPrm));
	
	if(m_initPrm.timerInterval<=0)
		m_initPrm.timerInterval = 40;
	if(m_initPrm.disFPS<=0)
		m_initPrm.disFPS = 25;

	//pthread_mutex_init(&render_lock, NULL);
	//pthread_cond_init(&render_cond, NULL);

	pthread_mutexattr_t mutex_attr;
	pthread_condattr_t cond_attr;
	int status=0;
	status |= pthread_mutexattr_init(&mutex_attr);
	status |= pthread_condattr_init(&cond_attr);
	status |= pthread_condattr_setclock(&cond_attr, CLOCK_MONOTONIC);
	status |= pthread_mutex_init(&render_lock, &mutex_attr);
	status |= pthread_cond_init(&render_cond, &cond_attr);
	pthread_condattr_destroy(&cond_attr);
	pthread_mutexattr_destroy(&mutex_attr);

	
	setFPS(m_initPrm.disFPS);

    //glutInitWindowPosition(m_initPrm.winPosX, m_initPrm.winPosY);
    glutInitWindowSize(VIDEO_DIS_WIDTH,VIDEO_DIS_HEIGHT);
    glutCreateWindow("DSS");
	//glutSetCursor(GLUT_CURSOR_NONE);
	glutDisplayFunc(&_display);
	if(m_initPrm.idlefunc != NULL)
		glutIdleFunc(m_initPrm.idlefunc);
	glutReshapeFunc(_reshape);

	//
	if(m_initPrm.keyboardfunc != NULL)
		glutKeyboardFunc(m_initPrm.keyboardfunc);
	if(m_initPrm.keySpecialfunc != NULL)
		glutSpecialFunc(m_initPrm.keySpecialfunc);

	//mouse event:
	if(m_initPrm.mousefunc != NULL)
		glutMouseFunc(m_initPrm.mousefunc);//GLUT_LEFT_BUTTON GLUT_MIDDLE_BUTTON GLUT_RIGHT_BUTTON; GLUT_DOWN GLUT_UP
	if(m_initPrm.passivemotionfunc != NULL)
		glutPassiveMotionFunc(m_initPrm.passivemotionfunc);
	
	if(m_initPrm.motionfunc != NULL)
		glutMotionFunc(m_initPrm.motionfunc);
	
	if(0)//(m_initPrm.menufunc != NULL)
	{
		int t_sub_menu2 = glutCreateMenu(processmtdmodeMenu);
		glutAddMenuEntry("Prohibit",0);
		glutAddMenuEntry("Allow",1);
		int t_maxnum_submenu = glutCreateMenu(m_initPrm.maxnum);
		glutAddMenuEntry("5",0);
		glutAddMenuEntry("10",1);
		int t_minsi_submenu = glutCreateMenu(m_initPrm.minsize);
		glutAddMenuEntry("100",0);
		glutAddMenuEntry("1000",1);
		int t_maxsi_submenu = glutCreateMenu(m_initPrm.maxsize);
		glutAddMenuEntry("40000",0);
		glutAddMenuEntry("50000",1);
		int t_rig_submenu = glutCreateMenu(m_initPrm.setrigion);
		glutAddMenuEntry("Rigion1",0);
		int t_rigsel_submenu = glutCreateMenu(m_initPrm.rigionsel);
		glutAddMenuEntry("Rigion1",0);
		int t_rigpolygon_submenu = glutCreateMenu(m_initPrm.rigionpolygon);
		glutAddMenuEntry("Rigion1",0);
		int t_dc_submenu = glutCreateMenu(processdetectcondMenu);
		glutAddMenuEntry("Condition1",0);
		glutAddMenuEntry("Condition2",1);
		int t_redetect_submenu = glutCreateMenu(processredetectMenu);
		glutAddMenuEntry("Disable",0);
		glutAddMenuEntry("Enable",1);
		int t_output_submenu = glutCreateMenu(processalarmputMenu);
		glutAddMenuEntry("Disable",0);
		glutAddMenuEntry("Enable",1);
		int t_polar_submenu = glutCreateMenu(processpolarMenu);
		glutAddMenuEntry("+",0);
		glutAddMenuEntry("-",1);
		int t_dur_submenu = glutCreateMenu(processdurationMenu);
		glutAddMenuEntry("1s",0);
		glutAddMenuEntry("3s",1);
		glutAddMenuEntry("5s",2);
		glutAddMenuEntry("7s",3);
		glutAddMenuEntry("9s",4);
		
		glutCreateMenu(NULL);
		glutAddSubMenu("Auto Detect Enable",t_sub_menu2);
		glutAddSubMenu("Rigion Set",t_rig_submenu);
		glutAddSubMenu("Rigion Select",t_rigsel_submenu);
		glutAddSubMenu("Rigion Set Polygon",t_rigpolygon_submenu);
		glutAddSubMenu("Detect Condition",t_dc_submenu);
		glutAddSubMenu("Redetect after lost",t_redetect_submenu);
#if __MOVE_DETECT__
		glutAddSubMenu("Max Num",t_maxnum_submenu);
		glutAddSubMenu("Minimum Target Size",t_minsi_submenu);
		glutAddSubMenu("Maximum Target Size",t_maxsi_submenu);
#endif
		glutAddSubMenu("Alarm Output",t_output_submenu);
		glutAddSubMenu("Output Polar",t_polar_submenu);
		glutAddSubMenu("Duration",t_dur_submenu);
		glutAttachMenu(GLUT_RIGHT_BUTTON);
	}

	if(m_initPrm.visibilityfunc != NULL)
		glutVisibilityFunc(m_initPrm.visibilityfunc);
	if(m_initPrm.bFullScreen){
		glutFullScreen();
		m_bFullScreen = true;
	}

	glutCloseFunc(_close);
	
	initRender();

	gl_updateVertex();

	gl_init();

	gl_Loadinit();

	return 0;
}

int CDisplayer::get_videoSize(int chId, DS_Size &size)
{
	if(chId < 0 || chId >= DS_CHAN_MAX)
		return -1;
	size = m_videoSize[chId];

	return 0;
}
int CDisplayer::dynamic_config(DS_CFG type, int iPrm, void* pPrm)
{
	int iRet = 0;
	int chId;
	bool bEnable;
	DS_Rect *rc;
	DS_fRect *frc;

	
	if(type == DS_CFG_ChId){
		if(iPrm >= m_renderCount || iPrm < 0)
			return -1;
		if(pPrm == NULL)
			return -2;
		chId = *(int*)pPrm;

		m_renders[iPrm].video_chId = chId;

		OSA_printf("%%%%%%%%%%%%render=%d  chid=%d\n",iPrm,chId);
	}
	
	if(type == DS_CFG_RenderPosRect){
		if(iPrm >= m_renderCount || iPrm < 0)
			return -1;
		if(pPrm == NULL)
			return -2;
		rc = (DS_Rect*)pPrm;
		OSA_mutexLock(&m_mutex);
		m_renders[iPrm].displayrect= *rc;
		OSA_mutexUnlock(&m_mutex);
		printf("the DS_CFG_RenderPosRect x=%d y=%d w=%d h=%d  pIStuts->PicpPosStat=%d\n",
			m_renders[iPrm].displayrect.x,m_renders[iPrm].displayrect.y,
			m_renders[iPrm].displayrect.w,m_renders[iPrm].displayrect.h,iPrm);
	}

	if(type == DS_CFG_EnhEnable){
		if(iPrm >= m_renderCount || iPrm < 0)
			return -1;
		if(pPrm == NULL)
			return -2;
		bEnable = *(bool*)pPrm;
		m_bEnh[iPrm] = bEnable;
		OSA_printf("the video enhanceiPrm[%d] =%d\n",iPrm,m_bEnh[iPrm]);
	}

	if(type == DS_CFG_MMTEnable){
		if(iPrm >= m_renderCount || iPrm < 0)
			return -1;
		if(pPrm == NULL)
			return -2;
		bEnable = *(bool*)pPrm;
		m_Mmt[iPrm] = bEnable;
		OSA_printf("the video mmtEnableiPrm[%d] =%d\n",iPrm,m_Mmt[iPrm]);
	}

	if(type == DS_CFG_CropEnable){
		if(iPrm >= m_renderCount || iPrm < 0)
			return -1;
		if(pPrm == NULL)
			return -2;
		bEnable = *(bool*)pPrm;
		m_renders[iPrm].bCrop = bEnable;
	}

	if(type == DS_CFG_CropRect){
		if(iPrm >= m_renderCount || iPrm < 0)
			return -1;
		if(pPrm == NULL)
			return -2;
		rc = (DS_Rect*)pPrm;
		m_renders[iPrm].croprect = *rc;

		gl_updateVertex();
	}

	if(type == DS_CFG_VideoTransMat){ //src transform
		if(iPrm >= DS_CHAN_MAX || iPrm < 0)
			return -1;
		if(pPrm == NULL)
			return -2;
		memcpy(m_glmat44fTrans[iPrm], pPrm, sizeof(float)*16);
	}

	if(type == DS_CFG_ViewTransMat){ //dst transform
		if(iPrm >= m_renderCount || iPrm < 0)
			return -1;
		if(pPrm == NULL)
			return -2;
		memcpy(m_renders[iPrm].transform , pPrm, sizeof(float)*16);
		gl_updateVertex();
	}

	if(type == DS_CFG_BindRect){
		if(iPrm >= m_renderCount || iPrm < 0)
			return -1;
		if(pPrm == NULL)
			return -2;
		frc = (DS_fRect*)pPrm;
		m_renders[iPrm].bindrect = *frc;
		//initRender(false);
		gl_updateVertex();
	}

	if(type == DS_CFG_FreezeEnable){
		if(iPrm >= m_renderCount || iPrm < 0)
			return -1;
		if(pPrm == NULL)
			return -2;
		bEnable = *(bool*)pPrm;
		m_renders[iPrm].bFreeze = bEnable;
	}

	if(type == DS_CFG_VideodetEnable){
		if(iPrm >= m_renderCount || iPrm < 0)
			return -1;
		if(pPrm == NULL)
			return -2;
		bEnable = *(bool*)pPrm;
		m_renders[iPrm].videodect = bEnable;
		//printf("DS_CFG_VideodetEnableiPrm=%d status=%d\n",iPrm,bEnable);
	}

	if(type ==DS_CFG_Renderdisplay)
		{
		if(iPrm >= m_renderCount || iPrm < 0)
			return -1;
		if(pPrm == NULL)
			return -2;
		bEnable = *(bool*)pPrm;
		Videoeable[iPrm]=bEnable;
		}

	if(type ==DS_CFG_Rendercount)
		{

		if(iPrm >= DS_RENDER_MAX || iPrm < 0)
			return -1;

		m_renderCount=iPrm;
		}

	return iRet;
}

extern "C" int yuyv2bgr_(
	unsigned char *dst, const unsigned char *src,
	int width, int height, cudaStream_t stream);

extern "C" int uyvy2bgr_(
	unsigned char *dst, const unsigned char *src,
	int width, int height, cudaStream_t stream);


void extractYUYV2Gray(Mat src, Mat dst)
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


void extractUYVY2Gray1(Mat src, Mat dst)
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

#if 0
void CDisplayer::display(Mat frame, int chId, int code)
{
	static int saveCount = 0;
	int i;
	int bufId=0;
	unsigned char *d_src = NULL;
	unsigned char *d_src_rgb = NULL;

	unsigned char tvbuffer0=0;
	unsigned char tvbuffer1=0;
	unsigned char tvbuffer2=0;
	unsigned char tvbuffer3=0;
	unsigned char firbuffer=0;
	unsigned char palbuffer=0;

	int nChannel = frame.channels();
	unsigned int byteCount = frame.rows * frame.cols * nChannel * sizeof(unsigned char);
	assert(chId>=0 && chId<DS_CHAN_MAX);

	#if 0
	if(disptimeEnable == 1){	
		//test zhou qi  time
		int64 captime = 0;

		captime = getTickCount();
		
		double curtime = (captime/getTickFrequency())*1000;///( (getTickCount() - trktime)/getTickFrequency())*1000;
		
		static double pretime = 0.0;
		capTime = curtime - pretime;
		pretime = curtime;
	}
	#endif
/*
	cv::Mat frame_gray;
	frame_gray = Mat(frame.rows, frame.cols, CV_8UC1);

	extractYUYV2Gray2(frame, frame_gray);
	cv::imshow("111",frame_gray);
	cv::waitKey(1);
*/

	if(nChannel == 1 || code == -1){
		cudaMalloc_share((void**)&d_src_rgb, byteCount, chId + DS_CHAN_MAX);
		firbuffer= OSA_bufGetEmpty(&(tskSendBuffir), &bufId, OSA_TIMEOUT_NONE);
		if(firbuffer==0)
			d_src_rgb=(unsigned char *)tskSendBuffir.bufInfo[bufId].virtAddr;

		OSA_mutexLock(&m_mutex);

		cudaMemcpyAsync(d_src_rgb, frame.data, byteCount, cudaMemcpyHostToDevice,  m_cuStream[0]);
		m_img[chId] = cv::Mat(frame.rows, frame.cols, CV_8UC1, d_src_rgb);

		OSA_mutexUnlock(&m_mutex);

		if(firbuffer==0)
			OSA_bufPutFull(&(tskSendBuffir), bufId);

		cudaFree_share(d_src_rgb, chId + DS_CHAN_MAX);

	}else{
		unsigned int byteCount_rgb = frame.rows * frame.cols * 3* sizeof(unsigned char);
		cudaMalloc_share((void**)&d_src, byteCount, chId);
		cudaMalloc_share((void**)&d_src_rgb, byteCount_rgb, chId + DS_CHAN_MAX);
		if(chId == video_pal)
		{
			palbuffer= OSA_bufGetEmpty(&(tskSendBufpal), &bufId, OSA_TIMEOUT_NONE);
			if(palbuffer==0)
				d_src_rgb=(unsigned char *)tskSendBufpal.bufInfo[bufId].virtAddr;
		}
		else if(chId==video_gaoqing0)
		{
			tvbuffer0= OSA_bufGetEmpty(&(tskSendBuftv0), &bufId, OSA_TIMEOUT_NONE);
			if(tvbuffer0==0)
				d_src_rgb=(unsigned char *)tskSendBuftv0.bufInfo[bufId].virtAddr;
		}
		else if(chId==video_gaoqing)
		{
			tvbuffer1= OSA_bufGetEmpty(&(tskSendBuftv1), &bufId, OSA_TIMEOUT_NONE);
			if(tvbuffer1==0)
				d_src_rgb=(unsigned char *)tskSendBuftv1.bufInfo[bufId].virtAddr;
		}
		else if(chId==video_gaoqing2)
		{
			tvbuffer2= OSA_bufGetEmpty(&(tskSendBuftv2), &bufId, OSA_TIMEOUT_NONE);
			if(tvbuffer2==0)
				d_src_rgb=(unsigned char *)tskSendBuftv2.bufInfo[bufId].virtAddr;
		}
		else if(chId==video_gaoqing3)
		{
			tvbuffer3= OSA_bufGetEmpty(&(tskSendBuftv3), &bufId, OSA_TIMEOUT_NONE);
			if(tvbuffer3==0)
				d_src_rgb=(unsigned char *)tskSendBuftv3.bufInfo[bufId].virtAddr;
		}

		OSA_mutexLock(&m_mutex);


		cudaMemcpyAsync(d_src + (byteCount>>2)*0, frame.data + (byteCount>>2)*0, (byteCount>>2), cudaMemcpyHostToDevice, m_cuStream[0]);
		cudaMemcpyAsync(d_src + (byteCount>>2)*1, frame.data + (byteCount>>2)*1, (byteCount>>2), cudaMemcpyHostToDevice, m_cuStream[1]);
		cudaMemcpyAsync(d_src + (byteCount>>2)*2, frame.data + (byteCount>>2)*2, (byteCount>>2), cudaMemcpyHostToDevice, m_cuStream[2]);
		cudaMemcpyAsync(d_src + (byteCount>>2)*3, frame.data + (byteCount>>2)*3, (byteCount>>2), cudaMemcpyHostToDevice, m_cuStream[3]);
		
		if(code == CV_YUV2BGR_YUYV){
			yuyv2bgr_(d_src_rgb + (byteCount_rgb>>2)*0, d_src + (byteCount>>2)*0, frame.cols, (frame.rows>>2), m_cuStream[0]);
			yuyv2bgr_(d_src_rgb + (byteCount_rgb>>2)*1, d_src + (byteCount>>2)*1, frame.cols, (frame.rows>>2), m_cuStream[1]);
			yuyv2bgr_(d_src_rgb + (byteCount_rgb>>2)*2, d_src + (byteCount>>2)*2, frame.cols, (frame.rows>>2), m_cuStream[2]);
			yuyv2bgr_(d_src_rgb + (byteCount_rgb>>2)*3, d_src + (byteCount>>2)*3, frame.cols, (frame.rows>>2), m_cuStream[3]);
		}
		else if(code == CV_YUV2BGR_UYVY){
			uyvy2bgr_(d_src_rgb + (byteCount_rgb>>2)*0, d_src + (byteCount>>2)*0, frame.cols, (frame.rows>>2), m_cuStream[0]);
			uyvy2bgr_(d_src_rgb + (byteCount_rgb>>2)*1, d_src + (byteCount>>2)*1, frame.cols, (frame.rows>>2), m_cuStream[1]);
			uyvy2bgr_(d_src_rgb + (byteCount_rgb>>2)*2, d_src + (byteCount>>2)*2, frame.cols, (frame.rows>>2), m_cuStream[2]);
			uyvy2bgr_(d_src_rgb + (byteCount_rgb>>2)*3, d_src + (byteCount>>2)*3, frame.cols, (frame.rows>>2), m_cuStream[3]);
		}
		m_img[chId] = cv::Mat(frame.rows, frame.cols, CV_8UC3, d_src_rgb);

		OSA_mutexUnlock(&m_mutex);

		cudaFree_share(d_src, chId);

		if((chId==video_gaoqing0)&&(tvbuffer0==0))
		{
			OSA_bufPutFull(&(tskSendBuftv0), bufId);
			
		}
		else if((chId==video_gaoqing)&&(tvbuffer1==0))
		{
			OSA_bufPutFull(&(tskSendBuftv1), bufId);
			
		}
		else if((chId==video_gaoqing2)&&(tvbuffer2==0))
		{
			OSA_bufPutFull(&(tskSendBuftv2), bufId);
			
		}
		else if((chId==video_gaoqing3)&&(tvbuffer3==0))
		{
			OSA_bufPutFull(&(tskSendBuftv3), bufId);
			
		}
		else if((chId==video_pal)&&(palbuffer==0))
		{
			OSA_bufPutFull(&(tskSendBufpal), bufId);
		}
		cudaFree_share(d_src_rgb, chId + DS_CHAN_MAX);
	}

}
#else
void CDisplayer::display(Mat frame, int chId, int code)
{
	assert(chId>=0 && chId<DS_CHAN_MAX);

	OSA_mutexLock(&m_mutex);

	m_frame[chId][pp[chId]] = frame;
	m_code[chId] = code;

	OSA_mutexUnlock(&m_mutex);
}
#endif

void CDisplayer::transfer()
{
	int winId, chId, nChannel,code;
	unsigned char *d_src = NULL;
	unsigned char *d_src_rgb = NULL;
	unsigned int byteCount ;
	unsigned int byteBlock ;
	int mask = 0;
	int i =0;
	for(winId=0; winId<m_renderCount; winId++)
	{
		chId = m_renders[winId].video_chId;

		if(chId < 0 || chId >= DS_CHAN_MAX)
		{
			continue;
		}

		OSA_mutexLock(&m_mutex);
		dism_img[chId]=	m_frame[chId][pp[chId]];
		code = m_code[chId];
		OSA_mutexUnlock(&m_mutex);

		if(dism_img[chId].cols <=0 || dism_img[chId].rows <=0 || dism_img[chId].channels() == 0)
		{
			continue;
		}
		nChannel = dism_img[chId].channels();
		byteCount = dism_img[chId].rows * dism_img[chId].cols * nChannel * sizeof(unsigned char);
		byteBlock = byteCount/DS_CUSTREAM_CNT;

		if(!((mask >> chId)&1))
		{
			if(nChannel == 1|| code == -1)
			{
				cudaMalloc_share((void**)&d_src_rgb, byteCount, chId + DS_CHAN_MAX);
				cudaMemcpyAsync(d_src_rgb, dism_img[chId].data, byteCount, cudaMemcpyHostToDevice,  m_cuStream[0]);
				m_img[chId] = cv::Mat(dism_img[chId].rows, dism_img[chId].cols, CV_MAKETYPE(CV_8U,nChannel), d_src_rgb);
				cudaFree_share(d_src_rgb, chId + DS_CHAN_MAX);
			}
			else
			{
				unsigned int byteCount_rgb = dism_img[chId].rows * dism_img[chId].cols * 3* sizeof(unsigned char);
				unsigned int byteBlock_rgb = byteCount_rgb/DS_CUSTREAM_CNT;
				unsigned char *d_src_gray = NULL;

				cudaMalloc_share((void**)&d_src, byteCount, chId);
				cudaMalloc_share((void**)&d_src_rgb, byteCount_rgb, chId + DS_CHAN_MAX);

				for(i = 0; i<DS_CUSTREAM_CNT; i++)
					cudaMemcpyAsync(d_src + byteBlock*i, dism_img[chId].data + byteBlock*i, byteBlock, cudaMemcpyHostToDevice, m_cuStream[i]);

				if(code == CV_YUV2BGR_YUYV)
				{
					for(i = 0; i<DS_CUSTREAM_CNT; i++)
					{
						yuyv2bgr_(d_src_rgb + byteBlock_rgb*i, d_src + byteBlock*i, dism_img[chId].cols, (dism_img[chId].rows/DS_CUSTREAM_CNT), m_cuStream[i]);
					}
				}

				if(code == CV_YUV2BGR_UYVY)
				{
					for(i = 0; i<DS_CUSTREAM_CNT; i++)
					{
						uyvy2bgr_(d_src_rgb + byteBlock_rgb*i, d_src + byteBlock*i, dism_img[chId].cols, (dism_img[chId].rows/DS_CUSTREAM_CNT), m_cuStream[i]);
					}
				}

				m_img[chId] = cv::Mat(dism_img[chId].rows, dism_img[chId].cols, CV_8UC3, d_src_rgb);

				if(m_bEnh[chId])
				{
					cuClahe( m_img[chId] ,m_img[chId], 8,8,3.5,1);
				
					#if 0
					#if HISTEN
					cuHistEnh( m_img[chId], dst);
					#elif CLAHEH 
					cuClahe( m_img[chId], dst,8,8,3.5,1);
					#elif DARKEN
					cuUnhazed( m_img[chId], dst);
					#else
					cuHistEnh( m_img[chId], dst);
					#endif	
					#endif
				}

				cudaFree_share(d_src, chId);
				cudaFree_share(d_src_rgb, chId + DS_CHAN_MAX);
			}
			mask |= (1<<chId);
		}
	}
}

GLuint CDisplayer::async_display(int chId, int width, int height, int channels)
{
	assert(chId>=0 && chId<DS_CHAN_MAX);

	if(m_videoSize[chId].w  == width  && m_videoSize[chId].h == height && m_videoSize[chId].c == channels )
		return buffId_input[chId];

	//OSA_printf("%s: w = %d h = %d (%dx%d) cur %d\n", __FUNCTION__, width, height, m_videoSize[chId].w, m_videoSize[chId].h, buffId_input[chId]);

	if(m_videoSize[chId].w != 0){
		glDeleteBuffers(1, &buffId_input[chId]);
		glGenBuffers(1, &buffId_input[chId]);
	}
	
	
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffId_input[chId]);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, width*height*channels, NULL, GL_DYNAMIC_COPY);//GL_STATIC_DRAW);//GL_DYNAMIC_DRAW);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	m_videoSize[chId].w = width;
	m_videoSize[chId].h = height;
	m_videoSize[chId].c = channels;

	gl_updateVertex();

	//OSA_printf("%s: w = %d h = %d (%dx%d) out %d\n", __FUNCTION__, width, height, m_videoSize[chId].w, m_videoSize[chId].h, buffId_input[chId]);
	return buffId_input[chId];
}

void CDisplayer::run()
{
	m_bRun = true;
	glutTimerFunc(0, _timeFunc, m_initPrm.timerfunc_value);
}

void CDisplayer::stop()
{
	m_bRun = false;
}

int CDisplayer::setFullScreen(bool bFull)
{
	if(bFull)
		glutFullScreen();
	else
		;
	m_bFullScreen = bFull;
	return 0;
}
void CDisplayer::reDisplay(void)
{
	glutPostRedisplay();
}

void CDisplayer::UpDateOsd(int idc)
{
	if(idc<0 || idc>=DS_DC_CNT )
		return;
	updata_osd[idc] = true;
}

/***********************************************************************/



static char glName[] = {"DS_RENDER"}; 

int CDisplayer::gl_create()
{
	char *argv[1] = {glName};
	int argc = 1;
    	glutInit(&argc, argv);  
    	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);

	glClearColor(0.0f, 0.0f, 0.01f, 0.0f );

	cudaEventCreate(&m_startEvent);
	cudaEventCreate(&m_stopEvent);
	return 0;
}
void CDisplayer::gl_destroy()
{
	gl_uninit();
	glutLeaveMainLoop();
	cudaEventDestroy(m_startEvent);
	cudaEventDestroy(m_stopEvent);
}

#define TEXTURE_ROTATE (0)
#define ATTRIB_VERTEX 3
#define ATTRIB_TEXTURE 4

void CDisplayer::gl_init()
{
	int i;

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
		return;
	}

	m_glProgram = gltLoadShaderPairWithAttributes("Shader.vsh", "Shader.fsh", 2, 
		ATTRIB_VERTEX, "vVertex", ATTRIB_TEXTURE, "vTexCoords");

	m_fontProgram = gltLoadShaderPairWithAttributes("fontShader.vp", "fontShader.fp", 0);
	
	glGenBuffers(DS_CHAN_MAX, buffId_input);
	glGenTextures(DS_CHAN_MAX, textureId_input);
	
	for(i=0; i<DS_CHAN_MAX; i++)
	{
		glBindTexture(GL_TEXTURE_2D, textureId_input[i]);
		assert(glIsTexture(textureId_input[i]));
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);//GL_NEAREST);//GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);//GL_NEAREST);//GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);//GL_CLAMP);//GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);//GL_CLAMP);//GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0,3, vcapWH[i][0], vcapWH[i][1], 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, NULL);
	}
	glGenBuffers(DS_DC_CNT, buffId_osd);
	glGenTextures(DS_DC_CNT, textureId_osd); 

	for(i=0; i<DS_DC_CNT; i++)
	{
		m_disOsd[i]    = cv::Mat(vcapWH[i][1], vcapWH[i][0], CV_8UC4, cv::Scalar(0,0,0,0));
		m_imgOsd[i] = cv::Mat(vcapWH[i][1], vcapWH[i][0], CV_8UC4, cv::Scalar(0,0,0,0));
		
		glBindTexture(GL_TEXTURE_2D, textureId_osd[i]);
		assert(glIsTexture(textureId_osd[i]));
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_imgOsd[i].cols, m_imgOsd[i].rows, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, NULL);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffId_osd[i]);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, m_imgOsd[i].rows*m_imgOsd[i].cols*m_imgOsd[i].channels(), m_imgOsd[i].data, GL_DYNAMIC_COPY);//GL_STATIC_DRAW);//GL_DYNAMIC_DRAW);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	}
	
	x11disbuffer=(unsigned char *)malloc(mallocwidth*mallocheight*4);

	for(i=0; i<DS_CHAN_MAX; i++){
		memcpy(m_glmat44fTrans[i], m_glmat44fTransDefault, sizeof(m_glmat44fTransDefault));
	}

	glClear(GL_COLOR_BUFFER_BIT);
	OSDCreatText();
}

void CDisplayer::gl_uninit()
{
	int i;
	if(m_glProgram != 0)
		glDeleteProgram(m_glProgram);
	m_glProgram = 0;

	glDeleteTextures(DS_CHAN_MAX, textureId_input);
	glDeleteBuffers(DS_CHAN_MAX, buffId_input);
	//glDeleteTextures(1, &textureId_osd); 
	//glDeleteTextures(1, &textureId_pbo);
	//glDeleteBuffers(1, pixBuffObjs);
	glDeleteTextures(DS_DC_CNT, textureId_osd);
	glDeleteBuffers(DS_DC_CNT, buffId_osd);
}


void* CDisplayer::displayerload(void *pPrm)
{
	int i=0;	
	while(1)
	{
		
		OSA_semWait(&(gThis->tskdisSemmain),OSA_TIMEOUT_FOREVER);
		for(i=0; i<DS_DC_CNT; i++)
		{
			if(i == gThis->m_renders[0].video_chId)
			{
				if(!gThis->updata_osd[i])
				{
					unsigned int  size=gThis->m_imgOsd[i].cols*gThis->m_imgOsd[i].rows*gThis->m_imgOsd[i].channels();
					memcpy(gThis->m_disOsd[i].data,gThis->m_imgOsd[i].data,size);
					gThis->updata_osd[i]=true;
				}
			}
		}
	}
}


void CDisplayer::gl_Loadinit()
{
	int status=0;	
       status = OSA_semCreate(&tskdisSemmain, 1, 0);
       OSA_assert(status == OSA_SOK);

       status = OSA_thrCreate(
                 &tskdisHndlmain,
                 displayerload,
                 OSA_THR_PRI_DEFAULT,
                 0,
             NULL
             );
  	OSA_assert(status == OSA_SOK);
}


int CDisplayer::gl_updateVertex(void)
{
	int iRet = 0;
	int winId, chId, i;
	DS_Rect rc;
	//GLfloat ftmp;

	for(winId=0; winId<m_renderCount; winId++)
	{
		m_glvVerts[winId][0] = -1.0f; m_glvVerts[winId][1] = 1.0f;
		m_glvVerts[winId][2] = 1.0f; m_glvVerts[winId][3] = 1.0f;
		m_glvVerts[winId][4] = -1.0f; m_glvVerts[winId][5] = -1.0f;
		m_glvVerts[winId][6] = 1.0f; m_glvVerts[winId][7] = -1.0f;

		for(i=0; i<4; i++){
			float x = m_glvVerts[winId][i*2+0];
			float y = m_glvVerts[winId][i*2+1];
			m_glvVerts[winId][i*2+0] = m_renders[winId].transform[0][0] * x + m_renders[winId].transform[0][1] * y + m_renders[winId].transform[0][3];
			m_glvVerts[winId][i*2+1] = m_renders[winId].transform[1][0] * x + m_renders[winId].transform[1][1] * y + m_renders[winId].transform[1][3];
		}

		if(m_renders[winId].bBind){
			GLfloat subw, subh;
			subw = m_glvVerts[winId][2] - m_glvVerts[winId][0];
			subh = m_glvVerts[winId][5] - m_glvVerts[winId][1];
			m_glvVerts[winId][0] += m_renders[winId].bindrect.x*subw;
			m_glvVerts[winId][1] += m_renders[winId].bindrect.y*subh;
			m_glvVerts[winId][2] = m_glvVerts[winId][0] + m_renders[winId].bindrect.w*subw;
			m_glvVerts[winId][3] = m_glvVerts[winId][1];
			m_glvVerts[winId][4] = m_glvVerts[winId][0];
			m_glvVerts[winId][5] = m_glvVerts[winId][1] + m_renders[winId].bindrect.h*subh;
			m_glvVerts[winId][6] = m_glvVerts[winId][0] + m_renders[winId].bindrect.w*subw;
			m_glvVerts[winId][7] = m_glvVerts[winId][1] + m_renders[winId].bindrect.h*subh;
		}

		m_glvTexCoords[winId][0] = 0.0; m_glvTexCoords[winId][1] = 0.0;
		m_glvTexCoords[winId][2] = 1.0; m_glvTexCoords[winId][3] = 0.0;
		m_glvTexCoords[winId][4] = 0.0; m_glvTexCoords[winId][5] = 1.0;
		m_glvTexCoords[winId][6] = 1.0; m_glvTexCoords[winId][7] = 1.0;
	}

	//rc.x=0,rc,y=0,m_videoSize[0].w=720,m_videoSize[0].h=576,m_videoSize[1].w=1920,m_videoSize[1].h=1080
	for(winId=0; winId<m_renderCount; winId++)
	{		
		chId = m_renders[winId].video_chId;
		if(chId < 0 || chId >= DS_CHAN_MAX)
			continue;
		rc = m_renders[winId].croprect;
		//winId=0/1,rc.x=840,rc.y=444,rc.w=240,rc.h=192
		if(m_videoSize[chId].w<=0 || m_videoSize[chId].h<=0){
			iRet ++;
			continue;
		}
		if(rc.w == 0 && rc.h == 0){
			continue;
		}
		//OSA_printf("%s: crop : %d,%d  %dx%d\n", __func__, rc.x, rc.y, rc.w, rc.h);
		//OSA_printf("%s: crop : curvideo %d  %d x %d\n", __func__, chId, m_videoSize[chId].w, m_videoSize[chId].h);
		m_glvTexCoords[winId][0] = (GLfloat)rc.x/m_videoSize[chId].w; 
		m_glvTexCoords[winId][1] = (GLfloat)rc.y/m_videoSize[chId].h;

		m_glvTexCoords[winId][2] = (GLfloat)(rc.x+rc.w)/m_videoSize[chId].w;
		m_glvTexCoords[winId][3] = (GLfloat)rc.y/m_videoSize[chId].h;

		m_glvTexCoords[winId][4] = (GLfloat)rc.x/m_videoSize[chId].w;
		m_glvTexCoords[winId][5] = (GLfloat)(rc.y+rc.h)/m_videoSize[chId].h;

		m_glvTexCoords[winId][6] = (GLfloat)(rc.x+rc.w)/m_videoSize[chId].w;
		m_glvTexCoords[winId][7] = (GLfloat)(rc.y+rc.h)/m_videoSize[chId].h;
		//pal
		// m_glvTexCoords[1][0]=1.166667    840/720
		// m_glvTexCoords[1][1]=0.770833    440/576
		//m_glvTexCoords[1][2]=1.500000    
		//m_glvTexCoords[1][3]=0.770833
		//m_glvTexCoords[1][4]=1.166667
		//m_glvTexCoords[1][5]=1.104167
		//m_glvTexCoords[1][6]=1.500000
		//m_glvTexCoords[1][7]=1.104167
		
		//gaoqing
		//m_glvTexCoords[1][0]=0.437500    840/1920
		//m_glvTexCoords[1][1]=0.411111    440/1080
		//m_glvTexCoords[1][2]=0.562500
		//m_glvTexCoords[1][3]=0.411111
		//m_glvTexCoords[1][4]=0.437500
		// m_glvTexCoords[1][5]=0.588889
		//m_glvTexCoords[1][6]=0.562500
		//m_glvTexCoords[1][7]=0.588889
	}

	return iRet;
}

static int64 tstart = 0;
static int64 tstartBK = 0;
static float offtime = 0;
double enhancetime=0;
static unsigned  int frametextcout=0;
static unsigned  int frametextcout1=0;

static unsigned int tvframecount=0;
static unsigned int tvdupcount=800;
static unsigned int firframecount=0;
static unsigned int firdupcount=800;

static unsigned int tvclearbuffer=1;
static unsigned int firclearbuffer=1;

#if 0
void CDisplayer::gl_textureLoad(void)
{
		
	int winId, chId;
	unsigned int mask = 0;
	float elapsedTime;

	unsigned char disbuffer=0;
	int bufid=0;
	unsigned char *disbuf=NULL;
	unsigned int byteCount=0;

	tstart = getTickCount();
	if(tstartBK != 0){
		offtime = (tstart - tstartBK)/getTickFrequency();
		//OSA_printf("chId = %d, gl_display offtime: time = %f sec \n", chId,  offtime);
	}
	tstartBK = tstart;
	cudaEventRecord(m_startEvent, 0);

	
	for(winId=0; winId<m_renderCount; winId++)
	{
		chId = m_renders[winId].video_chId;
		//if(winId>2)
		//	chId=1;
		if(chId < 0 || chId >= DS_CHAN_MAX)
		{
			continue;
		}
		dism_img[chId]=m_img[chId];

		if(dism_img[chId].cols <=0 || dism_img[chId].rows <=0 || dism_img[chId].channels() == 0)
		{
			//printf("[chId =%d  winId=%d] w=%d h=%d c=%d\n",chId,winId,m_img[chId].cols,m_img[chId].rows,m_img[chId].channels());
			continue;
		}
		
		if(!((mask >> chId)&1))
		{
			if(!m_renders[winId].bFreeze)
			{
				GLuint pbo = async_display(chId, dism_img[chId].cols, dism_img[chId].rows, dism_img[chId].channels());
				byteCount = dism_img[chId].cols*dism_img[chId].rows*dism_img[chId].channels()*sizeof(unsigned char);
				unsigned char *dev_pbo = NULL;
				size_t tmpSize;
				freezeonece=1;
				OSA_assert(pbo == buffId_input[chId]);
	
				cudaResource_RegisterBuffer(chId, pbo, byteCount);
				cudaResource_mapBuffer(chId, (void **)&dev_pbo, &tmpSize);

				frametextcout++;
				tvframecount++;
				firframecount++;

				int drupfram=PICBUFFERCOUNT;
				if(chId==video_gaoqing0)
				{
					int framecount=OSA_bufGetBufcount(&(tskSendBuftv0),0);
					if(framecount>=drupfram)
					{
						if(OSA_bufGetFull(&tskSendBuftv0, &bufid, OSA_TIMEOUT_NONE)==0)
							OSA_bufPutEmpty(&tskSendBuftv0, bufid);
						if(OSA_bufGetFull(&tskSendBuftv0, &bufid, OSA_TIMEOUT_NONE)==0)
							OSA_bufPutEmpty(&tskSendBuftv0, bufid);
					}
					disbuffer=OSA_bufGetFull(&tskSendBuftv0, &bufid, OSA_TIMEOUT_NONE);
				}
				else if(chId==video_gaoqing)
				{
					int framecount=OSA_bufGetBufcount(&(tskSendBuftv1),0);
					if(framecount>=drupfram)
					{
						if(OSA_bufGetFull(&tskSendBuftv1, &bufid, OSA_TIMEOUT_NONE)==0)
							OSA_bufPutEmpty(&tskSendBuftv1, bufid);
						if(OSA_bufGetFull(&tskSendBuftv1, &bufid, OSA_TIMEOUT_NONE)==0)
							OSA_bufPutEmpty(&tskSendBuftv1, bufid);
					}
					disbuffer=OSA_bufGetFull(&tskSendBuftv1, &bufid, OSA_TIMEOUT_NONE);
				}
				else if(chId==video_gaoqing2)
				{
					int framecount=OSA_bufGetBufcount(&(tskSendBuftv2),0);
					if(framecount>=drupfram)
					{
						if(OSA_bufGetFull(&tskSendBuftv2, &bufid, OSA_TIMEOUT_NONE)==0)
							OSA_bufPutEmpty(&tskSendBuftv2, bufid);
						if(OSA_bufGetFull(&tskSendBuftv2, &bufid, OSA_TIMEOUT_NONE)==0)
							OSA_bufPutEmpty(&tskSendBuftv2, bufid);
					}
					disbuffer=OSA_bufGetFull(&tskSendBuftv2, &bufid, OSA_TIMEOUT_NONE);
				}
				else if(chId==video_gaoqing3)
				{
					int framecount=OSA_bufGetBufcount(&(tskSendBuftv3),0);
					if(framecount>=drupfram)
					{
						if(OSA_bufGetFull(&tskSendBuftv3, &bufid, OSA_TIMEOUT_NONE)==0)
							OSA_bufPutEmpty(&tskSendBuftv3, bufid);
						if(OSA_bufGetFull(&tskSendBuftv3, &bufid, OSA_TIMEOUT_NONE)==0)
							OSA_bufPutEmpty(&tskSendBuftv3, bufid);
					}
					disbuffer=OSA_bufGetFull(&tskSendBuftv3, &bufid, OSA_TIMEOUT_NONE);
				}
				else if(chId==video_pal)
				{
					int framecount=OSA_bufGetBufcount(&(tskSendBufpal),0);
					if(framecount>=drupfram)
					{
						if(OSA_bufGetFull(&tskSendBufpal, &bufid, OSA_TIMEOUT_NONE)==0)
							OSA_bufPutEmpty(&tskSendBufpal, bufid);
						if(OSA_bufGetFull(&tskSendBufpal, &bufid, OSA_TIMEOUT_NONE)==0)
							OSA_bufPutEmpty(&tskSendBufpal, bufid);
					}
					disbuffer=OSA_bufGetFull(&tskSendBufpal, &bufid, OSA_TIMEOUT_NONE);
				}
				if((disbuffer==0)&&(chId==video_pal))
				{
					dism_img[chId].data=(unsigned char *)tskSendBufpal.bufInfo[bufid].virtAddr;
					pal_pribuffid=bufid;
				}
				else if((disbuffer==0)&&(chId==video_gaoqing0))
				{				
					dism_img[chId].data=(unsigned char *)tskSendBuftv0.bufInfo[bufid].virtAddr;
					tv_pribuffid0=bufid;
				}
				else if((disbuffer==0)&&(chId==video_gaoqing))
				{					
					dism_img[chId].data=(unsigned char *)tskSendBuftv1.bufInfo[bufid].virtAddr;
					tv_pribuffid1=bufid;
				}
				else if((disbuffer==0)&&(chId==video_gaoqing2))
				{
					dism_img[chId].data=(unsigned char *)tskSendBuftv2.bufInfo[bufid].virtAddr;
					tv_pribuffid2=bufid;
				}
				else if((disbuffer==0)&&(chId==video_gaoqing3))
				{			
					dism_img[chId].data=(unsigned char *)tskSendBuftv3.bufInfo[bufid].virtAddr;
					tv_pribuffid3=bufid;
				}

			#if 0
			if(disptimeEnable == 1){//0
				//test zhou qi  time
				int64 disptime = 0;

				disptime = getTickCount();
				
				double curtime = (disptime/getTickFrequency())*1000;///( (getTickCount() - trktime)/getTickFrequency())*1000;
				
				static double pretime = 0.0;
				double time = curtime - pretime;
				pretime = curtime;

				putText(m_imgOsd[1],dispstrDisplay,
						Point( m_imgOsd[1].cols-350, 80),
						FONT_HERSHEY_TRIPLEX,0.8,
						cvScalar(0,0,0,0), 1
						);
				sprintf(dispstrDisplay, "disp time = %0.3fFPS", 1000.0/time);

				putText(m_imgOsd[1],dispstrDisplay,
						Point(m_imgOsd[1].cols-350, 80),
						FONT_HERSHEY_TRIPLEX,0.8,
						cvScalar(255,255,0,255), 1
						);

				putText(m_imgOsd[1],capstrDisplay,
						Point( m_imgOsd[1].cols-350, 55),
						FONT_HERSHEY_TRIPLEX,0.8,
						cvScalar(0,0,0,0), 1
						);
				sprintf(capstrDisplay, "cap time = %0.3fFPS", 1000.0/capTime);

				putText(m_imgOsd[1],capstrDisplay,
						Point(m_imgOsd[1].cols-350, 55),
						FONT_HERSHEY_TRIPLEX,0.8,
						cvScalar(255,255,0,255), 1
						);
			}
			#endif

				if(m_renders[chId].videodect)
				{
					//cudaMemcpy(dev_pbo, m_img[chId].data, byteCount, cudaMemcpyDeviceToDevice);
					cudaMemcpy(x11disbuffer, dism_img[chId].data,byteCount, cudaMemcpyDeviceToHost);
				}
				else
				{
					//cudaMemcpy(dev_pbo, m_img_novideo.data, byteCount, cudaMemcpyDeviceToDevice);
					cudaMemcpy(x11disbuffer, m_img_novideo.data,byteCount, cudaMemcpyDeviceToHost);
				}

	

				if((disbuffer==0)&&(chId==video_gaoqing0))
					OSA_bufPutEmpty(&tskSendBuftv0, bufid);
				if((disbuffer==0)&&(chId==video_gaoqing))
					OSA_bufPutEmpty(&tskSendBuftv1, bufid);
				if((disbuffer==0)&&(chId==video_gaoqing2))
					OSA_bufPutEmpty(&tskSendBuftv2, bufid);
				if((disbuffer==0)&&(chId==video_gaoqing3))
					OSA_bufPutEmpty(&tskSendBuftv3, bufid);		
				if((disbuffer==0)&&(chId==video_pal))
					OSA_bufPutEmpty(&tskSendBufpal, bufid);		
			
			}
			//glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffId_input[chId]);
			glBindTexture(GL_TEXTURE_2D, textureId_input[chId]);
			if(dism_img[chId].channels() == 1){
				if(!m_renders[winId].bFreeze)
				glTexImage2D(GL_TEXTURE_2D, 0, m_videoSize[chId].c, m_videoSize[chId].w, m_videoSize[chId].h, 0, GL_RED, GL_UNSIGNED_BYTE, x11disbuffer);
			}else{
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_videoSize[chId].w, m_videoSize[chId].h, GL_BGR_EXT, GL_UNSIGNED_BYTE, x11disbuffer);
			}

			mask |= (1<<chId);

		}
	}

	if(m_bOsd)
	{
		for(int i=0; i<DS_DC_CNT;  i++)
		{
			if(updata_osd[i])
			{
			
				glBindTexture(GL_TEXTURE_2D, textureId_osd[i]);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_imgOsd[i].cols, m_imgOsd[i].rows, GL_BGRA_EXT, GL_UNSIGNED_BYTE, m_disOsd[i].data);
				updata_osd[i] = false;
			}			
		}

	}

	
	cudaEventRecord(m_stopEvent, 0);
	cudaEventSynchronize(m_stopEvent);
	cudaEventElapsedTime(	&elapsedTime, m_startEvent, m_stopEvent);
	//if(elapsedTime > 5.0f)
	//	OSA_printf("%s: -------elapsed %.3f ms.\n", __func__, elapsedTime);

	float telapse = ( (getTickCount() - tstart)/getTickFrequency());
}
#else
void CDisplayer::gl_textureLoad(void)
{
	int winId, chId;
	unsigned int mask = 0;
	unsigned int byteCount=0;

	tStamp[0] = getTickCount();
	tstart = tStamp[0];
	if(tend == 0ul)
		tend = tstart;
	if(1)
	{
		uint64_t telapse_ns = (uint64_t)(m_telapse*1000000.f);
		uint64_t sleep_ns = render_time_nsec - telapse_ns;
		if (last_render_time.tv_sec != 0 && last_render_time.tv_nsec != 0 && render_time_nsec>telapse_ns)
		{
			pthread_mutex_lock(&render_lock);
			struct timespec now;
			clock_gettime(CLOCK_MONOTONIC, &now);
			struct timespec waittime = last_render_time;
			waittime.tv_nsec += sleep_ns;
			waittime.tv_sec += waittime.tv_nsec / 1000000000UL;
			waittime.tv_nsec %= 1000000000UL;
	        __suseconds_t cur_us, to_us;
	        cur_us = (now.tv_sec * 1000000.0 + now.tv_nsec/1000.0);
	        to_us = (waittime.tv_sec * 1000000.0 + waittime.tv_nsec / 1000.0);
	        if(to_us > cur_us)
	        	pthread_cond_timedwait(&render_cond, &render_lock,&waittime);
			pthread_mutex_unlock(&render_lock);
		}
		else{
			//OSA_printf("%s %d: last_render_time.tv_sec=%ld render_time_nsec=%ld telapse_ns=%ld sleep_ns=%ld",
			//		__func__, __LINE__, last_render_time.tv_sec, render_time_nsec, telapse_ns, sleep_ns);
		}
	}
	tStamp[1] = getTickCount();

	transfer();

	tStamp[2] = getTickCount();

	for(winId=0; winId<m_renderCount; winId++)
	{
		chId = m_renders[winId].video_chId;

		if(chId < 0 || chId >= DS_CHAN_MAX)
		{
			continue;
		}
		dism_img[chId]=m_img[chId];

		if(dism_img[chId].cols <=0 || dism_img[chId].rows <=0 || dism_img[chId].channels() == 0)
		{
			continue;
		}

		if(!((mask >> chId)&1))
		{
			if(!m_renders[winId].bFreeze)
			{
				GLuint pbo = async_display(chId, dism_img[chId].cols, dism_img[chId].rows, dism_img[chId].channels());
				byteCount = dism_img[chId].cols*dism_img[chId].rows*dism_img[chId].channels()*sizeof(unsigned char);
				unsigned char *dev_pbo = NULL;
				size_t tmpSize;
				freezeonece=1;
				OSA_assert(pbo == buffId_input[chId]);

				cudaResource_RegisterBuffer(chId, pbo, byteCount);
				cudaResource_mapBuffer(chId, (void **)&dev_pbo, &tmpSize);
				if(tmpSize != byteCount)
				{
					OSA_printf("%s: WARNING!!! %d - %d \n", __func__, (unsigned int)tmpSize, byteCount);
				}
			
				if(m_renders[chId].videodect)
					cudaMemcpy(dev_pbo, dism_img[chId].data, byteCount, cudaMemcpyDeviceToDevice);
				else
					cudaMemcpy(dev_pbo, m_img_novideo.data, byteCount, cudaMemcpyDeviceToDevice);

				cudaResource_unmapBuffer(chId);
				cudaResource_UnregisterBuffer(chId);
			}
			
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffId_input[chId]);
			glBindTexture(GL_TEXTURE_2D, textureId_input[chId]);
			if(dism_img[chId].channels() == 1){
				//glTexImage2D(GL_TEXTURE_2D, 0, m_videoSize[chId].c, m_videoSize[chId].w, m_videoSize[chId].h, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_videoSize[chId].w, m_videoSize[chId].h, GL_RED, GL_UNSIGNED_BYTE, NULL);
			}else{
				//glTexImage2D(GL_TEXTURE_2D, 0, m_videoSize[chId].c, m_videoSize[chId].w, m_videoSize[chId].h, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, NULL);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_videoSize[chId].w, m_videoSize[chId].h, GL_BGR_EXT, GL_UNSIGNED_BYTE, NULL);
			}
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

			mask |= (1<<chId);
		}
	}

	if(m_bOsd)
	{
		for(int i=0; i<DS_DC_CNT;  i++)	{
			if(updata_osd[i]) {
				glBindTexture(GL_TEXTURE_2D, textureId_osd[i]);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_disOsd[i].cols, m_disOsd[i].rows, GL_BGRA_EXT, GL_UNSIGNED_BYTE, m_disOsd[i].data);
				updata_osd[i] = false;
			}
		}
	}
	
}
#endif

int CDisplayer::setFPS(float fps)
{
    if (fps == 0)
    {
        OSA_printf("[Render]Fps 0 is not allowed. Not changing fps");
        return -1;
    }
    pthread_mutex_lock(&render_lock);
    m_initPrm.disFPS = fps;
    m_interval = (1000000000ul)/(uint64)m_initPrm.disFPS;
    render_time_sec = m_interval / 1000000000ul;
    render_time_nsec = (m_interval % 1000000000ul);
    memset(&last_render_time, 0, sizeof(last_render_time));
    pthread_mutex_unlock(&render_lock);
    return 0;
}

void CDisplayer::disp_fps(){
    static GLint frames = 0;
    static GLint t0 = 0;
    static char  fps_str[20] = {'\0'};
    GLint t = glutGet(GLUT_ELAPSED_TIME);
 //   sprintf(fps_str, "%6.1f FPS\n", 0);
    if (t - t0 >= 200) {
        GLfloat seconds = (t - t0) / 1000.0;
        GLfloat fps = frames / seconds;
        sprintf(fps_str, "%6.1f FPS\n", fps);
       // printf("%6.1f FPS\n", fps);
        t0 = t;
        frames = 0;
    }
    glColor3f(0.0, 0.0, 1.0);
    glRasterPos2f(0, 0);
    glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char *)fps_str);
    frames++;
}


void CDisplayer::gl_display(void)
{	
	int winId, chId;
	unsigned int mask = 0;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(m_glProgram);
	
	Uniform_tex_in = glGetUniformLocation(m_glProgram, "tex_in");
	//Uniform_tex_osd = glGetUniformLocation(m_glProgram, "tex_osd");
	//Uniform_tex_pbo = glGetUniformLocation(m_glProgram, "tex_pbo");
	//Uniform_osd_enable = glGetUniformLocation(m_glProgram, "bOsd");
	Uniform_mattrans = glGetUniformLocation(m_glProgram, "mTrans");
	Uniform_font_color = glGetUniformLocation(m_fontProgram,"fontColor");



	for(winId=0; winId<m_renderCount; winId++)
	{			
		chId = m_renders[winId].video_chId;
		if(chId < 0 || chId >= DS_CHAN_MAX)
		{
			continue;
		}

		if(m_img[chId].cols <=0 || m_img[chId].rows <=0 || m_img[chId].channels() == 0)
		{
			continue;
		}
	
		glUniformMatrix4fv(Uniform_mattrans, 1, GL_FALSE, m_glmat44fTrans[0]);

		glUniform1i(Uniform_tex_in, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureId_input[chId]);
		glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, m_glvVerts[0]);
		glVertexAttribPointer(ATTRIB_TEXTURE, 2, GL_FLOAT, GL_FALSE, 0, m_glvTexCoords[0]);
		glEnableVertexAttribArray(ATTRIB_VERTEX);
		glEnableVertexAttribArray(ATTRIB_TEXTURE);

		glViewport(m_renders[winId].displayrect.x,m_renders[winId].displayrect.y,m_renders[winId].displayrect.w,m_renders[winId].displayrect.h);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	}	

	if(m_bOsd)
	{
		glUniformMatrix4fv(Uniform_mattrans, 1, GL_FALSE, m_glmat44fTransDefault);

		glUniform1i(Uniform_tex_in, 0);
		glActiveTexture(GL_TEXTURE0);

		glEnable(GL_BLEND);
		
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		for(int i=0; i<DS_DC_CNT; i++)
		{
			if(m_renders[0].video_chId != i)
				continue;
			glBindTexture(GL_TEXTURE_2D, textureId_osd[i]);
			glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, m_glvVertsDefault);
			glVertexAttribPointer(ATTRIB_TEXTURE, 2, GL_FLOAT, GL_FALSE, 0, m_glvTexCoordsDefault);
			glEnableVertexAttribArray(ATTRIB_VERTEX);
			glEnableVertexAttribArray(ATTRIB_TEXTURE);
			glViewport(0, 0, m_mainWinWidth_new[i], m_mainWinHeight_new[i]);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}
		glDisable(GL_BLEND);

		
		//IrisAndFocus();
		//OSDChid();
		//OSDWorkMode();
		//if(m_userOsd)
			//OSDFunc();
	}
	
	glUseProgram(0);

	tStamp[5] = getTickCount();
	int64 tcur = tStamp[5];
	m_telapse = (tStamp[5] - tStamp[1])*0.000001f + m_initPrm.disSched;

	if (last_render_time.tv_sec != 0 && last_render_time.tv_nsec != 0)
	{
		pthread_mutex_lock(&render_lock);
		struct timespec waittime = last_render_time;
		last_render_time.tv_sec += render_time_sec;
		last_render_time.tv_nsec += render_time_nsec;
		last_render_time.tv_sec += last_render_time.tv_nsec / 1000000000UL;
		last_render_time.tv_nsec %= 1000000000UL;
		struct timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		__suseconds_t cur_us, rd_us;
		cur_us = (now.tv_sec * 1000000.0 + now.tv_nsec/1000.0);
		rd_us = (last_render_time.tv_sec * 1000000.0 + last_render_time.tv_nsec / 1000.0);
		if (rd_us>cur_us)
		{
			waittime.tv_sec += render_time_sec;
			waittime.tv_nsec += render_time_nsec-500000UL;
			waittime.tv_sec += waittime.tv_nsec / 1000000000UL;
			waittime.tv_nsec %= 1000000000UL;
			pthread_cond_timedwait(&render_cond, &render_lock,&waittime);
		}
		else if(rd_us+10000UL<cur_us)
		{
			//OSA_printf("%s %d: win%d frame_is_late(%ld us)", __func__, __LINE__, glutGetWindow(), cur_us - rd_us);
			memset(&last_render_time, 0, sizeof(last_render_time));
		}
		pthread_mutex_unlock(&render_lock);
	}

	int64 tSwap = getTickCount();


	glutSwapBuffers();
	glutPostRedisplay();
	GetFPS();

	tStamp[6] = getTickCount();

	if(tStamp[6]-tSwap>5000000UL)
		m_nSwapTimeOut++;
	else
		m_nSwapTimeOut = 0;
	if ((last_render_time.tv_sec == 0 && last_render_time.tv_nsec == 0) || m_nSwapTimeOut>=3)
	{
    	struct timespec now;
    	clock_gettime(CLOCK_MONOTONIC, &now);
		last_render_time.tv_sec = now.tv_sec;
		last_render_time.tv_nsec = now.tv_nsec;
		//printf("\r\nReset render timer. fps(%d) swp(%ld ns)", m_initPrm.disFPS, tStamp[6]-tSwap);
		//fflush(stdout);
	}

	tend = tStamp[6];
	float renderIntv = (tend - m_tmRender)/getTickFrequency();

#if 1
	if(rCount%(m_initPrm.disFPS*100) == 0)
	{
		printf("\r\n[%d] %.4f (ws%.4f,cu%.4f,tv%.4f,to%.4f,rd%.4f,wp%.4f) %.4f(%.4f)",
			OSA_getCurTimeInMsec(),renderIntv,
			(tStamp[1]-tStamp[0])/getTickFrequency(),
			(tStamp[2]-tStamp[1])/getTickFrequency(),
			(tStamp[3]-tStamp[2])/getTickFrequency(),
			(tStamp[4]-tStamp[3])/getTickFrequency(),
			(tStamp[5]-tStamp[4])/getTickFrequency(),
			(tStamp[6]-tStamp[5])/getTickFrequency(),
			m_telapse, m_initPrm.disSched
			);
		fflush(stdout);
	}
	rCount ++;
#endif
	m_tmRender = tend;
}

void CDisplayer::IrisAndFocus()
{
	static int irisdir = 0;
	static int focusdir = 0;
	drawtriangle(plat->m_display.m_imgOsd[plat->extInCtrl->SensorStat], irisdir, 0);
	drawtriangle(plat->m_display.m_imgOsd[plat->extInCtrl->SensorStat], focusdir, 0);
	switch(gSYS_Osd.IrisAndFocusAndExit)
	{
	case Enable_Iris:
		chinese_osd(905,1000,L"光圈调节",1,4,255,0,0,255,VIDEO_DIS_WIDTH,VIDEO_DIS_HEIGHT);
		drawtriangle(plat->m_display.m_imgOsd[plat->extInCtrl->SensorStat], gSYS_Osd.cmd_triangle.dir, gSYS_Osd.cmd_triangle.alpha);
		irisdir = gSYS_Osd.cmd_triangle.dir;
		break;
	case Enable_Focus:
		chinese_osd(905,1000,L"聚焦调节",1,4,255,0,0,255,VIDEO_DIS_WIDTH,VIDEO_DIS_HEIGHT);
		drawtriangle(plat->m_display.m_imgOsd[plat->extInCtrl->SensorStat], gSYS_Osd.cmd_triangle.dir, gSYS_Osd.cmd_triangle.alpha);
		focusdir = gSYS_Osd.cmd_triangle.dir;
		break;
	case Disable:
		break;
	}
}

void CDisplayer::OSDWorkMode()
{
	int x = 15, x1 = 240, y = 1040;
	int R = 255, G = 255, B = 255;
	if(gSYS_Osd.m_workOsd)
	{
		switch(gSYS_Osd.workMode)
		{
		case 2:
			chinese_osd(x,y,L"工作模式:自动监视",1,4,R,G,B,255,VIDEO_DIS_WIDTH,VIDEO_DIS_HEIGHT);
			break;

		case 3:
			chinese_osd(x,y,L"工作模式:场景跟踪",1,4,R,G,B,255,VIDEO_DIS_WIDTH,VIDEO_DIS_HEIGHT);
			chinese_osd(x1,y,L"状态:捕获",1,4,R,G,B,255,VIDEO_DIS_WIDTH,VIDEO_DIS_HEIGHT);
			break;

		case 4:
			chinese_osd(x,y,L"工作模式:常规跟踪",1,4,R,G,B,255,VIDEO_DIS_WIDTH,VIDEO_DIS_HEIGHT);
			chinese_osd(x1,y,L"目标选择方式:移动转台",1,4,R,G,B,255,VIDEO_DIS_WIDTH,VIDEO_DIS_HEIGHT);
			break;

		case 5:
			chinese_osd(x,y,L"工作模式:常规跟踪",1,4,R,G,B,255,VIDEO_DIS_WIDTH,VIDEO_DIS_HEIGHT);
			chinese_osd(x1,y,L"目标选择方式:移动波门",1,4,R,G,B,255,VIDEO_DIS_WIDTH,VIDEO_DIS_HEIGHT);
			break;

		case 6:
			chinese_osd(x,y,L"工作模式:常规跟踪",1,4,R,G,B,255,VIDEO_DIS_WIDTH,VIDEO_DIS_HEIGHT);
			chinese_osd(x1,y,L"目标选择方式:移动检测",1,4,R,G,B,255,VIDEO_DIS_WIDTH,VIDEO_DIS_HEIGHT);
			break;

		case 7:
			chinese_osd(x,y,L"工作模式:场景跟踪",1,4,R,G,B,255,VIDEO_DIS_WIDTH,VIDEO_DIS_HEIGHT);
			if(sceneLost)
				chinese_osd(x1,y,L"状态:丟失",1,4,R,G,B,255,VIDEO_DIS_WIDTH,VIDEO_DIS_HEIGHT);
			else
				chinese_osd(x1,y,L"状态:跟踪",1,4,R,G,B,255,VIDEO_DIS_WIDTH,VIDEO_DIS_HEIGHT);
			break;

			default:
			break;
		}
	}
}

int CDisplayer::OSDChid()
{
	if(m_chidIDOsd)
	{
		switch(plat->extInCtrl->SensorStat)
		{
		case 0:
			chinese_osd(15,20,L"通道号：1",1,4,255,0,0,255,VIDEO_DIS_WIDTH,VIDEO_DIS_HEIGHT);
			break;
		case 1:
			chinese_osd(15,20,L"通道号：2",1,4,255,0,0,255,VIDEO_DIS_WIDTH,VIDEO_DIS_HEIGHT);
			break;
		case 2:
			chinese_osd(15,20,L"通道号：3",1,4,255,0,0,255,VIDEO_DIS_WIDTH,VIDEO_DIS_HEIGHT);
			break;
		case 3:
			chinese_osd(15,20,L"通道号：4",1,4,255,0,0,255,VIDEO_DIS_WIDTH,VIDEO_DIS_HEIGHT);
			break;
		case 4:
			chinese_osd(15,20,L"通道号：5",1,4,255,0,0,255,VIDEO_DIS_WIDTH,VIDEO_DIS_HEIGHT);
			break;
		}
	}

	if(m_chidNameOsd)
	{
		switch(plat->extInCtrl->SensorStat)
		{
		case 0:
			chinese_osd(15,65,L"通道名称：测试1",1,4,255,0,0,255,VIDEO_DIS_WIDTH,VIDEO_DIS_HEIGHT);
			break;
		case 1:
			chinese_osd(15,65,L"通道名称：测试2",1,4,255,0,0,255,VIDEO_DIS_WIDTH,VIDEO_DIS_HEIGHT);
			break;
		case 2:
			chinese_osd(15,65,L"通道名称：测试3",1,4,255,0,0,255,VIDEO_DIS_WIDTH,VIDEO_DIS_HEIGHT);
			break;
		case 3:
			chinese_osd(15,65,L"通道名称：测试4",1,4,255,0,0,255,VIDEO_DIS_WIDTH,VIDEO_DIS_HEIGHT);
			break;
		case 4:
			chinese_osd(15,65,L"通道名称：测试5",1,4,255,0,0,255,VIDEO_DIS_WIDTH,VIDEO_DIS_HEIGHT);
			break;
		}
	}
	return 0;
}

int CDisplayer::OSDFunc()
{
	unsigned char r, g, b, a, color, colorbak, Enable;
	short x, y;
	char font,fontsize;

	for(int i = 0;i<32;i++){
		if(gCFG_Osd.items[i].senbind)
			Enable = (gCFG_Osd.items[i].senID == plat->extInCtrl->SensorStat)?gCFG_Osd.items[i].ctrl:0;	// only userosd of current sensor need show
		else
			Enable = gCFG_Osd.items[i].ctrl;
		if(Enable){
			 x = gCFG_Osd.items[i].posx;
			 y = gCFG_Osd.items[i].posy;
		 	 a = gCFG_Osd.items[i].alpha;
			 color = gCFG_Osd.items[i].color;
			 font = gCFG_Osd.items[i].font;
			 fontsize = gCFG_Osd.items[i].fontsize;

			switch(color)
			{
				case 1:
					r = 0;
					g = 0;
					b = 0;
					break;
				case 2:
					r = 255;
					g = 255;
					b = 255;
					break;
				case 3:
					r = 255;
					g = 0;
					b = 0;
					break;
				case 4:
					r = 255;
					g = 255;
					b = 0;
					break;
				case 5:
					r = 0;
					g = 0;
					b = 255;
					break;
				case 6:
					r = 0;
					g = 255;
					b = 0;
					break;
				case 7:
					color = colorbak;
					break;
				}

			if(a > 0x0a)
				a = 0x0a;
			if(a == 0x0a)
				a = 0;
			else
				a = 255 - a*16;
			chinese_osd(x, y, &(gCFG_Osd.disBuf[i][0]),font ,fontsize, r, g, b, a, VIDEO_DIS_WIDTH, VIDEO_DIS_HEIGHT);
		}
	}

	return 0;
}

void CDisplayer::drawtriangle(Mat frame, int direction, int alpha)
{
	Point root_points[1][3];
	int npt[] = {3};

	switch(direction)
	{
	case up:
	root_points[0][0] = Point(1015,1006);
	root_points[0][1] = Point(1005,1026);
	root_points[0][2] = Point(1025,1026);
	break;

	case down:
		root_points[0][0] = Point(1015,1026);
		root_points[0][1] = Point(1005,1006);
		root_points[0][2] = Point(1025,1006);
		break;
		default:
		break;
	}
	const Point* ppt[1] = {root_points[0]};
	polylines(frame, ppt, npt, 1, 1, Scalar(255),1,8,0);
	fillPoly(frame, ppt, npt, 1, Scalar(0,0,255,alpha));
}

//////////////////////////////////////////////////////////////////////////
// Load the shader from the source text
void CDisplayer::gltLoadShaderSrc(const char *szShaderSrc, GLuint shader)
{
	GLchar *fsStringPtr[1];

	fsStringPtr[0] = (GLchar *)szShaderSrc;
	glShaderSource(shader, 1, (const GLchar **)fsStringPtr, NULL);
}

#define MAX_SHADER_LENGTH   8192
static GLubyte shaderText[MAX_SHADER_LENGTH];
////////////////////////////////////////////////////////////////
// Load the shader from the specified file. Returns false if the
// shader could not be loaded
bool CDisplayer::gltLoadShaderFile(const char *szFile, GLuint shader)
{
	GLint shaderLength = 0;
	FILE *fp;

	// Open the shader file
	fp = fopen(szFile, "r");
	if(fp != NULL)
	{
		// See how long the file is
		while (fgetc(fp) != EOF)
			shaderLength++;

		// Allocate a block of memory to send in the shader
		assert(shaderLength < MAX_SHADER_LENGTH);   // make me bigger!
		if(shaderLength > MAX_SHADER_LENGTH)
		{
			fclose(fp);
			return false;
		}

		// Go back to beginning of file
		rewind(fp);

		// Read the whole file in
		if (shaderText != NULL){
			size_t ret = fread(shaderText, 1, (size_t)shaderLength, fp);
			OSA_assert(ret == shaderLength);
		}

		// Make sure it is null terminated and close the file
		shaderText[shaderLength] = '\0';
		fclose(fp);
	}
	else
		return false;    

	// Load the string
	gltLoadShaderSrc((const char *)shaderText, shader);

	return true;
}   


/////////////////////////////////////////////////////////////////
// Load a pair of shaders, compile, and link together. Specify the complete
// source text for each shader. After the shader names, specify the number
// of attributes, followed by the index and attribute name of each attribute
GLuint CDisplayer::gltLoadShaderPairWithAttributes(const char *szVertexProg, const char *szFragmentProg, ...)
{
	// Temporary Shader objects
	GLuint hVertexShader;
	GLuint hFragmentShader; 
	GLuint hReturn = 0;   
	GLint testVal;

	// Create shader objects
	hVertexShader = glCreateShader(GL_VERTEX_SHADER);
	hFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Load them. If fail clean up and return null
	// Vertex Program
	if(gltLoadShaderFile(szVertexProg, hVertexShader) == false)
	{
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
		fprintf(stderr, "The shader at %s could ot be found.\n", szVertexProg);
		return (GLuint)NULL;
	}

	// Fragment Program
	if(gltLoadShaderFile(szFragmentProg, hFragmentShader) == false)
	{
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
		fprintf(stderr,"The shader at %s  could not be found.\n", szFragmentProg);
		return (GLuint)NULL;
	}

	// Compile them both
	glCompileShader(hVertexShader);
	glCompileShader(hFragmentShader);

	// Check for errors in vertex shader
	glGetShaderiv(hVertexShader, GL_COMPILE_STATUS, &testVal);
	if(testVal == GL_FALSE)
	{
		char infoLog[1024];
		glGetShaderInfoLog(hVertexShader, 1024, NULL, infoLog);
		fprintf(stderr, "The shader at %s failed to compile with the following error:\n%s\n", szVertexProg, infoLog);
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
		return (GLuint)NULL;
	}

	// Check for errors in fragment shader
	glGetShaderiv(hFragmentShader, GL_COMPILE_STATUS, &testVal);
	if(testVal == GL_FALSE)
	{
		char infoLog[1024];
		glGetShaderInfoLog(hFragmentShader, 1024, NULL, infoLog);
		fprintf(stderr, "The shader at %s failed to compile with the following error:\n%s\n", szFragmentProg, infoLog);
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
		return (GLuint)NULL;
	}

	// Create the final program object, and attach the shaders
	hReturn = glCreateProgram();
	glAttachShader(hReturn, hVertexShader);
	glAttachShader(hReturn, hFragmentShader);


	// Now, we need to bind the attribute names to their specific locations
	// List of attributes
	va_list attributeList;
	va_start(attributeList, szFragmentProg);

	// Iterate over this argument list
	char *szNextArg;
	int iArgCount = va_arg(attributeList, int);	// Number of attributes
	for(int i = 0; i < iArgCount; i++)
	{
		int index = va_arg(attributeList, int);
		szNextArg = va_arg(attributeList, char*);
		glBindAttribLocation(hReturn, index, szNextArg);
	}
	va_end(attributeList);

	// Attempt to link    
	glLinkProgram(hReturn);

	// These are no longer needed
	glDeleteShader(hVertexShader);
	glDeleteShader(hFragmentShader);  

	// Make sure link worked too
	glGetProgramiv(hReturn, GL_LINK_STATUS, &testVal);
	if(testVal == GL_FALSE)
	{
		char infoLog[1024];
		glGetProgramInfoLog(hReturn, 1024, NULL, infoLog);
		fprintf(stderr,"The programs %s and %s failed to link with the following errors:\n%s\n",
			szVertexProg, szFragmentProg, infoLog);
		glDeleteProgram(hReturn);
		return (GLuint)NULL;
	}

	// All done, return our ready to use shader program
	return hReturn;  
}   


void CDisplayer::GetFPS()
{
	#define FR_SAMPLES 10
	static struct timeval last={0,0};
	struct timeval now;
	float delta;
	if ((++frameCount) >= FR_SAMPLES) {
		gettimeofday(&now, NULL);
		delta= (now.tv_sec - last.tv_sec +(now.tv_usec - last.tv_usec)/1000000.0);
		last = now;
		setFrameRate(FR_SAMPLES / delta);
		setFrameCount(0);
	}
}

void CDisplayer::chinese_osd(int x,int y,wchar_t* text,char font,char fontsize,unsigned char r,unsigned char g,unsigned char b,unsigned char a,int win_width,int win_height)
{
	glUseProgram(m_fontProgram);
	_fontColor[0] = (float)r/float(255);
	_fontColor[1] = (float)g/float(255);
	_fontColor[2] = (float)b/float(255);
	_fontColor[3] = (float)a/float(255);
	glUniform4fv(Uniform_font_color,1,_fontColor);
	OSDdrawText(x,y,text,font,fontsize,win_width,win_height);
	glUseProgram(0);
}



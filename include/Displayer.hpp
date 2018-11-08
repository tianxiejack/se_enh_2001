
#ifndef DISPLAYER_HPP_
#define DISPLAYER_HPP_

#define DS_RENDER_MAX		(9)
#define DS_CHAN_MAX         (5)
//4
#define DS_CUSTREAM_CNT		(4)

#define DS_DC_CNT		(5)
#include "osa.h"
#include <osa_sem.h>
#include <cuda.h>
#include "cuda_runtime_api.h"


#include "osa.h"
#include "osa_thr.h"
#include "osa_buf.h"
#include "osa_sem.h"
#include "app_status.h"
#include "configable.h"

using namespace std;
using namespace cv;

typedef struct _ds_size{
	int w;
	int h;
	int c;
}DS_Size;

typedef struct _ds_rect{
	int x;
	int y;
	int w;
	int h;
}DS_Rect;
typedef struct _ds_rectmmt{
	int x;
	int y;
	int w;
	int h;
	int valid;
}DS_Rectmmt;

typedef struct _ds_frect{
	float x;
	float y;
	float w;
	float h;
}DS_fRect;

typedef struct _ds_render
{
	int video_chId;
	DS_Rect displayrect;
	bool bCrop;
	DS_Rect croprect;
	DS_fRect bindrect;
	bool bBind;
	float transform[4][4];
	bool bFreeze;
	bool videodect;
}DS_Render;

typedef struct _ds_init_param{
	bool bFullScreen;
	int winPosX;
	int winPosY;
	int winWidth;
	int winHeight;
	bool bScript;
	char szScriptFile[256];
	int initloyerId;
	//void (*displayfunc)(void);
#if APP_LINKAGE_MODE
	void (*passivemotionfunc)(GLint xMouse, GLint yMouse);
	void (*mousefunc)(int button, int state, int x, int y);
	void (*menufunc)(int value);
#endif

#if APP_TRACKER_MODE
	void (*motionfunc)(GLint xMouse, GLint yMouse);
	void (*mousefunc)(int button, int state, int x, int y);
	void (*menufunc)(int value);
#endif
	//void (*reshapefunc)(int width, int height);
	void (*keyboardfunc)(unsigned char key, int x, int y);
	void (*keySpecialfunc)( int, int, int );
	void (*visibilityfunc)(int state);
	void (*timerfunc)(int value);
	void (*idlefunc)(void);
	void (*closefunc)(void);
	int timerfunc_value;//context
	int timerInterval;//ms
}DS_InitPrm;


#if LINKAGE_FUNC
	typedef enum _DISPLAYMODE {
		PREVIEW_MODE = 0,
		PIC_IN_PIC,
		SIDE_BY_SIDE,
		LEFT_BALL_RIGHT_GUN,
		TOTAL_MODE_COUNT
	}DISPLAYMODE;
#endif


class CDisplayer 
{
#if LINKAGE_FUNC
public:
	cv::Mat gun_UndistorMat;
	char savePicName[20];
	cv::Mat gun_BMP;
    DISPLAYMODE displayMode;

	typedef enum _WindowSize{
		WINDOW_WIDTH = 1920,
		WINDOW_HEIGHT = 1080,
			
	}WindowSize;
	typedef enum _VIDEOCNANNEL {
		VIDEO_0 = 0,
		VIDEO_1,
		VIDEO_2,
		VIDEO_3,
		VIDEO_4,
		VIDEO_COUNT			
	}VIDEOCNANNEL;
public:
	DISPLAYMODE g_CurDisplayMode;
	void RenderVideoOnOrthoView(int videoChannel,int x, int y, int width, int height);
	void changeDisplayMode(DISPLAYMODE mode);
	void switchDisplayMode( );
	DISPLAYMODE getDisplayMode( );
	void linkage_init();
#endif

public:
	CDisplayer();
	~CDisplayer();
	int create();
	int destroy();
	int init(DS_InitPrm *pPrm);
	void run();
	void stop();

	typedef enum{
		DS_CFG_ChId = 0,
		DS_CFG_RenderPosRect,
		DS_CFG_EnhEnable,
		DS_CFG_CropEnable,
		DS_CFG_CropRect,
		DS_CFG_VideoTransMat,
		DS_CFG_ViewTransMat,
		DS_CFG_BindRect,
		DS_CFG_FreezeEnable,
		DS_CFG_VideodetEnable,
		DS_CFG_RenderPosinit,
		DS_CFG_Renderdisplay,
		DS_CFG_Rendercount,
		DS_CFG_MMTEnable,
		DS_CFG_Max
	}DS_CFG;

	int dynamic_config(DS_CFG type, int iPrm, void* pPrm);
	int get_videoSize(int chId, DS_Size &size);
	void display(Mat frame, int chId, int code = -1);/*CV_YUV2BGR_UYVY*/
	GLuint async_display(int chId, int width, int height, int channels);
	int setFullScreen(bool bFull);
	void reDisplay(void);
	void UpDateOsd(int idc);

	int m_mainWinWidth_new[eSen_Max];
	int m_mainWinHeight_new[eSen_Max];
	bool m_bRun;
	bool m_bFullScreen;
	bool m_bOsd;
	bool m_crossOsd;
	Mat m_disOsd[DS_DC_CNT];
	Mat m_imgOsd[DS_DC_CNT];
	DS_Size m_videoSize[DS_CHAN_MAX];
	GLuint buffId_input[DS_CHAN_MAX];
	bool m_bEnh[DS_CHAN_MAX];
	bool m_Mmt[DS_CHAN_MAX];
	bool  Videoeable[DS_CHAN_MAX];
	unsigned int dismodchanag;
	unsigned int dismodchanagcount;
	 int tv_pribuffid0;
	 int tv_pribuffid1;
	 int tv_pribuffid2;
	 int tv_pribuffid3;
	 int fir_pribuffid;
	 int pal_pribuffid;
	unsigned  int freezeonece;

	#if 1
	OSA_MutexHndl disLock;
  	OSA_SemHndl tskdisSemmain;
  	OSA_ThrHndl tskdisHndlmain;
	#endif
	void gl_Loadinit();
	static  void* displayerload(void *pPrm);
	void disp_fps();
	
	int enhancemod;
	float enhanceparam;

	char capstrDisplay[128];
	char dispstrDisplay[128];
	int disptimeEnable;

	OSA_BufCreate tskSendBufCreatetv0;
       OSA_BufHndl tskSendBuftv0;
	OSA_BufCreate tskSendBufCreatetv1;
       OSA_BufHndl tskSendBuftv1;
	OSA_BufCreate tskSendBufCreatetv2;
       OSA_BufHndl tskSendBuftv2;
	OSA_BufCreate tskSendBufCreatetv3;
       OSA_BufHndl tskSendBuftv3;

	OSA_BufCreate tskSendBufCreatefir;
        OSA_BufHndl tskSendBuffir;

	OSA_BufCreate tskSendBufCreatepal;
	OSA_BufHndl tskSendBufpal;
	
protected:
	DS_InitPrm m_initPrm;
	DS_Render m_renders[DS_RENDER_MAX];
	int m_renderCount;
	Mat m_img[DS_CHAN_MAX];
	Mat dism_img[DS_CHAN_MAX];
	Mat m_img_novideo;

	Mat x11m_img;
	unsigned char *x11disbuffer;
	int initRender(bool bInitBind = true);
	void uninitRender();

protected:
	static void _display(void);
	static void _timeFunc(int value);
	static void _reshape(int width, int height);
	static void processLinkageMenu(int value);
	static void processDMMenu(int value);
	static void processgunResMenu(int value);
	static void processballResMenu(int value);
	static void processposMenu(int value);
	static void processsizeMenu(int value);
	static void processnameMenu(int value);
	static void processtimeMenu(int value);
	static void processfontsizeMenu(int value);
	static void processosdposMenu(int value);
	static void processSenMenu(int value);
	static void processCarMenu(int value);
	static void processrigionMenu(int value);
	static void processrigionselMenu(int value);
	static void processtargetsizeMenu(int value);
	static void processtargetspeedMenu(int value);
	static void processtargetdircMenu(int value);
	static void processmaxnumMenu(int value);
	static void processdetectcondMenu(int value);
	static void processpolarMenu(int value);
	static void processdurationMenu(int value);
	static void processbuadrateMenu(int value);
	static void processdatabitMenu(int value);
	static void processstopbitMenu(int value);
	static void processparityMenu(int value);
	static void processipMenu(int value);
	static void processprotocolMenu(int value);
	static void processmtdswMenu(int value);
	static void processmtdmodeMenu(int value);
	static void processredetectMenu(int value);
	static void processalarmputMenu(int value);
	static void processtrkcondMenu(int value);

	static void _close(void);
	void gl_resize(void);

protected:
	GLint	m_glProgram;
	GLint	m_fontProgram;
	GLfloat m_glvVerts[DS_RENDER_MAX][8];
	GLfloat m_glvTexCoords[DS_RENDER_MAX][8];
	bool m_bUpdateVertex;
	GLfloat m_glmat44fTrans[DS_CHAN_MAX][16];
	GLuint textureId_input[DS_CHAN_MAX];
	//GLuint textureId_osd;//[DS_DC_CNT];

	GLuint textureId_osd[DS_DC_CNT];
	GLuint buffId_osd[DS_DC_CNT];
	unsigned char *dev_pbo_osd[DS_DC_CNT];
	GLboolean updata_osd[DS_DC_CNT];
	
	struct cudaGraphicsResource *cuda_pbo_resource[DS_CHAN_MAX];


	int gl_create();
	void gl_destroy();
	void gl_init();
	void gl_uninit();
	void gl_display();
	void gl_textureLoad();
	int gl_updateVertex();
	void gltLoadShaderSrc(const char *szShaderSrc, GLuint shader);
	bool gltLoadShaderFile(const char *szFile, GLuint shader);
	GLuint gltLoadShaderPairWithAttributes(const char *szVertexProg, const char *szFragmentProg, ...);

	Mat m_temp;

private:
	OSA_MutexHndl m_mutex;
	//unsigned char *m_dev_src_rgb[DS_CHAN_MAX];
	cudaStream_t m_cuStream[DS_CUSTREAM_CNT];

	cudaEvent_t	m_startEvent, m_stopEvent;


public:
	float frameRate ;
	int frameCount;
	void setFrameRate(float rate)	{ frameRate  = rate;	}; 
	void setFrameCount(int count)	{ frameCount = count;	};
	float getFrameRate()			{return frameRate;	};
	void GetFPS();
	void chinese_osd(int x,int y,wchar_t* text,char font,char fontsize,unsigned char r,unsigned char g,unsigned char b,unsigned char a,int win_width,int win_height);

	void IrisAndFocus();
	int OSDFunc();
	void drawtriangle(Mat frame, char direction, char alpha);

	
};

#define mallocwidth 1920
#define mallocheight 1080

#define PICBUFFERCOUNT 4

#endif /* DISPLAYER_HPP_ */

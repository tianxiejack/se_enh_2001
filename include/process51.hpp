
#ifndef PROCESS021_HPP_
#define PROCESS021_HPP_

#include "VideoProcess.hpp"
#include "osd_cv.h"


using namespace cv;

#if LINKAGE_FUNC

typedef struct _Resolution{
	int raw, col ;
}Resolution;

typedef struct _SysParam{
	Resolution gun_camera;
	Resolution ball_camera;
}SysParam;

#endif

class CProcess : public CVideoProcess
{
	UTC_RECT_float rcTrackBak[2],resultTrackBak;
	TARGET tgBak[MAX_TARGET_NUMBER];
	TARGETDRAW Mdrawbak[MAX_TARGET_NUMBER];
	Osd_cvPoint crossBak;
	Osd_cvPoint crossWHBak;
	cv::Rect acqRectBak;
	Osd_cvPoint freezecrossBak;
	Osd_cvPoint crosspicpBak;
	Osd_cvPoint crosspicpWHBak;
	Osd_cvPoint rectfovBak[2];
	Osd_cvPoint secBak[2];
	DS_Rect rendpos[4];
	int Osdflag[20];
	int osdindex;
	int Mmtsendtime;
	int prisensorstatus;
	int Fovpri[2];
	DS_Rectmmt Mmtpos[5];
	char trkFPSDisplay[128];
	char warnTargetIndex[8];
	Osd_cvPoint debugBak;
	char timedisplay[128];
	bool forwardflag,backflag;

#if LINKAGE_FUNC
	int key_point1_cnt ;
	int key_point2_cnt ;
	int AllPoints_Num  ;
	char show_key[64][6];
	char show_key2[64][6];
	Point key1_pos;
	Point key2_pos;
	Point textPos1_record[64];
	Point textPos2_record[64];
	Point circle_point;
private:
	void Cmp_SysParam();
	SysParam m_sysparm;
	FileStorage readfs;
	FileStorage writefs;
	void getParams();
	void setParams();

public:
	bool readParams(const char* file);
	bool writeParams(const char* file);
	void reMapCoords(int x, int y , bool mode);
	void Init_CameraMatrix();
	Mat undisImageGun;
	
	void manualHandleKeyPoints(int &x,int &y);
	
	int checkZoomPosTable(int delta);
#endif

public:
	CProcess();
	~CProcess();

	void OnCreate();
	void OnDestroy();
	void OnInit();
	void OnConfig();
	void OnRun();
	void OnStop();
	void Ontimer();
	bool OnPreProcess(int chId, Mat &frame);
	bool OnProcess(int chId, Mat &frame);
	void OnMouseLeftDwn(int x, int y);
	void OnMouseLeftUp(int x, int y);
	void OnMouseRightDwn(int x, int y);
	void OnMouseRightUp(int x, int y);
	void OnKeyDwn(unsigned char key);
	
	CMD_EXT* extInCtrl;
	static CProcess *sThis;
	void process_osd_test(void *pPrm);


protected:
	void msgdriv_event(MSG_PROC_ID msgId, void *prm);
	void osd_mtd_show(TARGET tg[], bool bShow = true);
	void DrawCross(cv::Rect rec,int fcolour ,int sensor,bool bShow /*= true*/);
	void drawmmt(TARGET tg[], bool bShow = true);
	void drawmmtnew(TARGET tg[], bool bShow = true);
	void erassdrawmmt(TARGET tg[], bool bShow = true);
	void erassdrawmmtnew(TARGETDRAW tg[], bool bShow = true);
	void DrawdashCross(int x,int y,int fcolour , bool bShow = true);
	void DrawdashRect(int startx,int starty,int endx,int endy,int colour);
	void DrawMeanuCross(int x,int y,int fcolour , bool bShow,int centerx,int centery);
	float  PiexltoWindowsx(int x,int channel);
	float  PiexltoWindowsy(int y,int channel);
	int  PiexltoWindowsxzoom(int x,int channel);
	int  PiexltoWindowsyzoom(int y,int channel);
	int  PiexltoWindowsxzoom_TrkRect(int x,int channel);
	int  PiexltoWindowsyzoom_TrkRect(int y,int channel);
	int  WindowstoPiexlx(int x,int channel);
	int  WindowstoPiexly(int y,int channel);
	float PiexltoWindowsxf(float x,int channel);
	float PiexltoWindowsyf(float y,int channel);



	 static int  MSGAPI_initial(void);
	 static void MSGAPI_init_device(long lParam );
	 static void MSGAPI_inputsensor(long lParam );
	 static void MSGAPI_picp(long lParam );
	 static void MSGAPI_inputtrack(long lParam );
	 static void MSGAPI_inpumtd(long lParam );
	 static void MSGAPI_inpuenhance(long lParam );
	 static void MSGAPI_inputbdt(long lParam );
	 static void MSGAPI_inputzoom(long lParam );
	 static void  MSGAPI_setAimRefine(long lParam          /*=NULL*/);
	 static void MSGAPI_setAimSize(long lParam );
	 static void MSGAPI_inputfrezz(long lParam );
	 static void MSGAPI_inputmmtselect(long lParam );
	 static void MSGAPI_croppicp(long lParam );
	 static void MSGAPI_inpumtdSelect(long lParam );
	 static void MSGAPI_inputpositon(long lParam );
	 static void MSGAPI_inputcoast(long lParam );
	 static void MSGAPI_inputfovselect(long lParam );
	 static void MSGAPI_inputfovchange(long lParam );
	 static void MSGAPI_inputsearchmod(long lParam );
	 static void MSGAPI_inputvideotect(long lParam );
	 static void MSGAPI_mmtshow(long lParam );
	 static void MSGAPI_FOVcmd(long lParam );
	 static void MSGAPI_SaveCfgcmd(long lParam );
	 static void MSGAPI_setMtdState(long lParam);

	 static void MSGAPI_update_osd(long IParam);
	 static void MSGAPI_update_alg(long IParam);
	 static void MSGAPI_update_camera(long IParam);
	 static void MSGAPI_input_algosdrect(long lParam);
	 static void MSGAPI_setMtdSelect(long lParam );

#if LINKAGE_FUNC
	 static void MSGAPI_update_ballPos(long lParam );
#endif

private:
	ACK_EXT extOutAck;
	bool    m_bCast;
	UInt32 	m_castTm;

	void process_status(void);

	void osd_init(void);
	void DrawLine(Mat frame, int startx, int starty, int endx, int endy, int width, UInt32 colorRGBA);
	void DrawHLine(Mat frame, int startx, int starty, int width, int len, UInt32 colorRGBA);
	void DrawVLine(Mat frame, int startx, int starty, int width, int len, UInt32 colorRGBA);
	void DrawChar(Mat frame, int startx, int starty, char *pChar, UInt32 frcolor, UInt32 bgcolor);
	void DrawString(Mat frame, int startx, int starty, char *pString, UInt32 frcolor, UInt32 bgcolor);
	void DrawStringcv(Mat& frame, int startx, int starty, char *pChar, UInt32 frcolor, UInt32 bgcolor);
	void osd_draw_cross(Mat frame, void *prm);
	void osd_draw_rect(Mat frame, void *prm);
	void osd_draw_rect_gap(Mat frame, void *prm);
	void osd_draw_text(Mat frame, void *prm);
	void osd_draw_cross_black_white(Mat frame, void *prm);

	int process_draw_line(Mat frame, int startx, int starty, int endx, int linewidth,char R, char G, char B, char A);
	int process_draw_instance(Mat frame);
	int draw_circle_display(Mat frame);

	void DrawRect(Mat frame,cv::Rect rec,int frcolor);
	void DrawAcqRect(cv::Mat frame,cv::Rect rec,int frcolor,bool bshow);

	void initAcqRect();
	void initAimRect();
	void set_trktype(CMD_EXT *p, unsigned int stat);

	#if __MOVE_DETECT__
	void mvIndexHandle(std::vector<TRK_RECT_INFO> &mvList,std::vector<TRK_RECT_INFO> &detect,int detectNum);
	#endif
	
public:
	void update_param_alg();
	void update_param_osd();
	RectfNode mRectbak[MAX_CHAN][100];
	int m_tempXbak, m_tempYbak, m_rectnbak[MAX_CHAN];
	char timearr[128];
	char timearrbak[128];
	int timexbak, timeybak;;

};



#endif /* PROCESS021_HPP_ */

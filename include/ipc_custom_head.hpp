/*
 * ipc_custom_head.hpp
 *
 *  Created on: 2019年3月14日
 *      Author: alex
 */

#ifndef IPC_CUSTOM_HEAD_HPP_
#define IPC_CUSTOM_HEAD_HPP_

#include "Ipcctl.h"

#define CAMERACHID 5

#define DEFAULTPATH "/"

typedef enum 
{
    sensor,  /*0*/
    pinp,
    trk,
    sectrk,
    enh,
    mtd,
    mtdSelect,
    mtdnum,
    mtdmode,
    trkdoor,
    posmove,/*10*/
    zoom,
    autocheck,
    mmt,
    mmtselect,
    axismove,
    read_shm_trkpos,
    read_shm_config,
    read_shm_osd,
    read_shm_utctrk,
    read_shm_camera,/*20*/
    elecZoom,
    exit_img,
    sensor_bind,
    sensor_video_trans,
    vframerate,
    vquality,
    osdbuffer,
    acqBox,
    Iris,
    focus,/*30*/
    exit_IrisAndFocus,
    menu,
    read_shm_lkosd,
    mmtLock,
    BoresightPos,
    trktype,
    AcqPos,
    algosdrect,
    ipcwordFont,
    ipcwordSize,/*40*/
    ipclosttime,
    ipcresolution,
    read_shm_osdtext,
    switchtarget,
    mouseptz,
    mtdredetect,
    mtdoutput,
    mtdpolar,
    mtdtrktime,
    setconfig,/*50*/
    querypos,
    speedloop,
    acqPosAndZoom,
    reset_swtarget_timer,
    mtdFrame,
    ballbaud,
    sceneTrk,
    ipcjoyevent,
    refreshptz,
    josctrl,
    workmode,
    enter_gridmap_view,
    invalid
}CMD_ID;


typedef enum 
{
	shm_rdonly,
	shm_rdwr
}shm_perms;

typedef enum
{
	cursor_move = 1,
	jos_button,
	jos_Dir,
	mouse_button,
	enter,
	jos_menu,
	workMode,
	ctrlMode
}josType;

typedef enum{
	manual_linkage = 1,
	Auto_linkage,
	ballctrl
}work_Mode;

typedef enum{
	jos = 1,
	mouse
}jos_Mode;

typedef enum{
	cursor_up = 1,
	cursor_down,
	cursor_left,
	cursor_right,
}josDir;

typedef struct{
	int type;
	int cursor_x;  //0~1920
	int cursor_y;	//0~1080
	int jos_button;	//0~9
	int jos_Dir;	//上，下，左，右
	int mouse_button;	//左键 右键
	int mouse_state;	//抬起2；按下1
	int enter;	//0,1
	int menu;	//0,1
	int workMode;	//1~3
	int ctrlMode;	//1,2
}ctrlParams;

typedef struct{
	unsigned int panPos;
	unsigned int tilPos;
	unsigned int zoom;
}LinkagePos;

typedef struct{
	volatile int dir;
	volatile int alpha;
}CMD_triangle;

typedef enum
{
    TRK_MODE,
    LOCK_MODE,
    SCAN_MODE,
    SMALL_AREA_MODE,
    FOLLOW_MODE,
    SEARCH_TRK_MODE
}to_osd_work_mode_enum;

typedef enum
{
    SINGLE_RANGE_MODE,
    CONTINUE_RANGE_MODE,
    MARK_RANGE_MODE,
    EXPOSURE_MODE
}to_osd_laser_work_mode_enum;

typedef enum 
{
    IPC_TOIMG_MSG,  		// SERVER TO CLIENT
    IPC_FRIMG_MSG,		// CLIENT TO SERCER
    IPC_SHA,
    IPC_OSD_SHA,
    IPC_UTCTRK_SHA,
    IPC_LKOSD_SHA,
    IPC_OSDTEXT_SHA,
    IPC_SEM,
    IPC_MAX
}IPC_MTYPE;

typedef enum {
        ipc_eSen_CH0 = 0x00,
        ipc_eSen_CH1 = 0x01,
        ipc_eSen_CH2 = 0x02,
        ipc_eSen_CH3 = 0x03,
        ipc_eSen_CH4 = 0x04,
        ipc_eSen_CH5 = 0x05,
        ipc_eSen_Max = 0x06
}ipc_eSenserStat;

typedef enum img_zoom
{
    zoom_0 = 0x00,
    zoom_2 = 0x02,
    zoom_4 = 0x04,
    zoom_8 = 0x08
}ipc_img_zoom;

typedef enum ipc_Dram_TrkAim
{
	IPC_AIM_NO1		= 0x00,
	IPC_AIM_NO2		= 0x01,
	IPC_AIM_NO3		= 0x02,
	IPC_AIM_NO4		= 0x03,
	IPC_AIM_NO5		= 0x04,
	IPC_AIM_MAX		= 0x05,
} ipc_eTrkAim;

typedef enum {
	ipc_ePicp_top_left = 0x00,
	ipc_ePicp_top_right = 0x01,
	ipc_ePicp_bot_left = 0x02,
	ipc_ePicp_bot_right = 0x03,
}ipc_ePicpPosStat;

typedef enum ipc_Dram_SysMode
{
	IPC_INIT_MODE           = 0x00,
	IPC_CHECK_MODE      = 0x01,
	IPC_AXIS_MODE           = 0x02,
	IPC_NORMAL_MODE     = 0x03,
} ipc_eSysMode;

typedef enum ipc_Dram_TrkType
{
	IPC_AUTO_MODE		= 0x00,
	IPC_CONTRAST_MODE	= 0x01,
	IPC_CENTRE_MODE		= 0x02,
} ipc_eTrkType;

typedef enum ipc_Dram_zoomCtrl
{
	ipc_eZoom_Ctrl_Pause    = 0x00,
	ipc_eZoom_Ctrl_SCAL2    = 0x01,
	ipc_eZoom_Ctrl_SCAL4    = 0x02,
	ipc_eZoom_Ctrl_ADD      = 0x01,
	ipc_eZoom_Ctrl_SUB      = 0x02,
} ipc_eZoomCtrl;

typedef enum ipc_Dram_ImgAlg
{
	ipc_eImgAlg_Disable     = 0x00,
	ipc_eImgAlg_Enable      = 0x01,
} ipc_eImgAlgStat;  // use by img zoom/enh/stb/mtd/rst

typedef enum ipc_Dram_MMTSelect
{
	ipc_eMMT_No     = 0x00,
	ipc_eMMT_Next   = 0x01,
	ipc_eMMT_Prev   = 0x02,
	ipc_eMMT_Select = 0x03,
} ipc_eMMTSelect;


typedef enum ipc_Dram_MtdSelect
{
	ipc_eMTD_No     = 0x00,
	ipc_eMTD_Next   = 0x01,
	ipc_eMTD_Prev   = 0x02,
	ipc_eMTD_Select = 0x03,
} ipc_eMtdSelect;

typedef enum ipc_Dram_DispGradeStat
{
	ipc_eDisp_hide      = 0x00,
	ipc_eDisp_show_rect     = 0x01,
	ipc_eDisp_show_text     = 0x02,
	ipc_eDisp_show_dbg      = 0x04,
} ipc_eDispGrade;

typedef enum ipc_Dram_DispGradeColor
{
	ipc_ecolor_Black = 0x01,
	ipc_ecolor_White    = 0x02,
	ipc_ecolor_Red = 0x03,
	ipc_ecolor_Yellow = 0x04,
	ipc_ecolor_Blue = 0x05,
	ipc_ecolor_Green = 0x06,
	ipc_ecolor_Default  = 0x07,
} ipc_eOSDColor;

typedef enum
{
	ipc_eTrk_Acq        = 0x00,
	ipc_eTrk_Normal = 0x01,
	ipc_eTrk_Assi       = 0x02,
	ipc_eTrk_Lost       = 0x03,
} ipc_eTrkStat;

typedef enum ipc_Dram_trkMode
{
	// mode cmd
	ipc_eTrk_mode_acq       = 0x00,
	ipc_eTrk_mode_target    = 0x01,
	ipc_eTrk_mode_mtd       = 0x02,
	ipc_eTrk_mode_sectrk    = 0x03,
	ipc_eTrk_mode_search    = 0x04,
} ipc_eTrkMode;

typedef enum ipc_Dram_trkRefine
{
	ipc_eTrk_ref_no     = 0x00,
	ipc_eTrk_ref_left   = 0x01,
	ipc_eTrk_ref_right  = 0x02,
	ipc_eTrk_ref_up     = 0x01,
	ipc_eTrk_ref_down   = 0x02,
} ipc_eTrkRefine;

typedef enum ipc_Dram_saveMode
{
	ipc_eSave_Disable       = 0x00,
	ipc_eSave_Enable        = 0x01,
	ipc_eSave_Cancel        = 0x02,
} ipc_eSaveMode;

typedef enum {
	IPC_JS_EVENT_BUTTON = 1,
	IPC_JS_EVENT_AXIS = 2,
}JOY_EVENT_TYPE;

typedef enum{
	IPC_MSGID_INPUT_AXISX = 0,
	IPC_MSGID_INPUT_AXISY = 1,
}JOY_EVENT_AXIS_NUMBER;


typedef struct{
	int areaSetBox;
	int detectNum;
	int detectSpeed;
	int tmpUpdateSpeed;
	int tmpMaxPixel;
	int tmpMinPixel;
	int sensitivityThreshold;
	int TrkMaxTime;
	int output;
	int outputPolarity;
	int detectArea_X;
	int detectArea_Y;
	int detectArea_wide;
	int detectArea_high;
	int priority;
	int alarm_delay;
}CMD_Mtd_Frame;

typedef struct{
    volatile unsigned char unitFaultStat;/*0:FR ok,TV ok; 1:FR ok,TV err; 2:FR err,TV ok; 3:FR err,TV err*/
    volatile unsigned char ComStat;/*\u4e3b\u901a\u4fe1\u72b6\u6001\uff0c0\u6b63\u5e38\uff0c1\u6545\u969c*/
    volatile unsigned char RomdeviceStat;/*\u5b58\u50a8\u8bbe\u5907\u72b6\u6001\uff1a0\u6b63\u5e38\uff0c1\u6545\u969c*/
    volatile unsigned char GPUStat;/*GPU\u72b6\u6001\uff1a0\u6b63\u5e38\uff0c1\u6545\u969c*/
}CMD_AUTOCHECK;


typedef struct{
	volatile unsigned char SensorStat;
}CMD_SENSOR;

typedef struct{
	volatile unsigned char ImgPicp;
	volatile unsigned char PicpSensorStat;
	volatile unsigned char PicpZoomStat;//pinp zoom
}CMD_PinP;

typedef struct{
	volatile unsigned char AvtTrkStat;// eTrkMode 
    	volatile unsigned char TrkCmd;//only for ack trk mod
}CMD_TRK;
	
typedef struct{
	volatile unsigned char SecAcqStat;// eSecTrkMode
	volatile  int ImgPixelX; //SecTrk X
	volatile  int ImgPixelY; //SecTrk Y
}CMD_SECTRK;

typedef struct{
	volatile unsigned char AcqStat;
	volatile unsigned int BoresightPos_x;
	volatile unsigned int BoresightPos_y;
}CMD_AcqBoxPos;

typedef struct{
    volatile unsigned char ImgEnhStat;// eImgAlgStat enh 0:close 1:open
}CMD_ENH;

typedef struct{
	volatile unsigned char ImgMmtStat;// eImgAlgStat mtd 0:close 1:open
}CMD_MMT;

typedef struct{
	volatile unsigned char ImgMtdStat;// eImgAlgStat mtd 0:close 1:open
	volatile unsigned char mtdMode;//0:auto-mode   1:manual-mode
}CMD_MTD;

typedef struct{
	volatile unsigned char ImgMmtSelect;// eMMTSelect or range 0-MTD_TARGET_NUM
}CMD_MMTSELECT;

typedef struct{
	volatile unsigned char unitMtdValid;//mmt  0-disable 1-valid 2-unvalid
	volatile unsigned char unitMtdPixelX1;//report mmt x1
	volatile unsigned char unitMtdPixelX2;//report mmt x2
	volatile unsigned char unitMtdPixelY1;//report mmt y1
	volatile unsigned char unitMtdPixelY2;//report mmt y2
}CMD_sMTD;

typedef struct{
	volatile unsigned char unitAimW;//aim w
	volatile unsigned char unitAimH;//aim h
}CMD_AIMWH;

typedef struct{
	volatile unsigned char unitAimX1; //aim x1
	volatile unsigned char unitAimX2; //aim x2
	volatile unsigned char unitAimY1; //aim y1
	volatile unsigned char unitAimY2; //aim y2
}CMD_AIMXY;

typedef struct{
	volatile unsigned char AvtTrkAimSize;
}CMD_TRKDOOR;

typedef struct{
	volatile unsigned char UartSpeedSelect;//sync422 rate
}CMD_SYNC422;

typedef struct{
	volatile unsigned char AvtMoveX;// eTrkRefine (axis or aim) x
	volatile unsigned char AvtMoveY;// eTrkRefine (axis or aim) y
}CMD_POSMOVE;

typedef struct{
	volatile unsigned int BoresightPos_x;
	volatile unsigned int BoresightPos_y;
}CMD_BoresightPos;

typedef struct{
	volatile unsigned char unitTrkX1;// for report and osd text x1
	volatile unsigned char unitTrkX2;// for report and osd text x2
	volatile unsigned char unitTrkY1;// for report and osd text y1
	volatile unsigned char unitTrkY2;// for report and osd text y2
}CMD_sTRKXY;

typedef struct{
	volatile unsigned char unitAxisX1;//cross x1
	volatile unsigned char unitAxisX2;//cross x2
	volatile unsigned char unitAxisY1;//cross y1
	volatile unsigned char unitAxisY2;//cross y2
}CMD_CROSSXY;

typedef struct{
    volatile unsigned char ImgZoomStat;
}CMD_ZOOM;


typedef struct{
    volatile unsigned char vframerate;
    volatile unsigned char vfSensorStat;
}CMD_VFRAMERATE;

typedef struct{
    volatile unsigned char vquality;
    volatile unsigned char vqSensorStat;
}CMD_VQUALITY;

typedef struct{
    volatile unsigned char color;
}CMD_OSDCOLOR;

typedef struct{
    volatile unsigned char font;
}CMD_OSDFONT;

typedef struct{
    volatile unsigned char size;
}CMD_OSDSIZE;

typedef struct{
    volatile unsigned char ctrl;
}CMD_OSDCTRL;


typedef struct{
    volatile unsigned char id;
}CMD_OSDID;

typedef struct{
    volatile short pos_X;
    volatile short pos_Y;
}CMD_OSDPOS;

typedef struct{
    volatile unsigned char alpha;
}CMD_OSDALPHA;

typedef struct{
	volatile unsigned char Imgalgosdrect;
}CMD_ALGOSDRECT;

typedef struct{
	volatile unsigned char resolution[ipc_eSen_Max];
	volatile unsigned char outputresol;
}CMD_IPCRESOLUTION;

typedef struct{
	volatile int mptzx;
	volatile int mptzy;
}CMD_MOUSEPTZ;

typedef struct{
	volatile unsigned int seconds;
}CMD_MTDTRKTIME;

typedef struct{
	volatile unsigned char block;
	volatile unsigned char field;
	volatile float value;
}CMD_SETCONFIG;

typedef struct{
    signed short value;
    unsigned char type;
    unsigned char number;
}CMD_JOY_EVENT;


typedef struct{
	unsigned int AimW;
	unsigned int AimH;
}AcqBoxWH;



/** universal status **/
typedef struct
{
	/***** new status *****/
	volatile int axisMoveStepX;
	volatile int axisMoveStepY;
	volatile int aimRectMoveStepX;
	volatile int aimRectMoveStepY;
	volatile int validChId;
	volatile int opticAxisPosX[ipc_eSen_Max];	//may be same to unitAxisX[eSen_Max]
	volatile int opticAxisPosY[ipc_eSen_Max];
	volatile int AxisPosX[ipc_eSen_Max];	
	volatile int AxisPosY[ipc_eSen_Max];
	volatile int AvtPosX[ipc_eSen_Max];	//target avt x,y for each channel
	volatile int AvtPosY[ipc_eSen_Max];
	
	volatile int AcqRectW[ipc_eSen_Max];
	volatile int AcqRectH[ipc_eSen_Max];
	volatile int AimW[ipc_eSen_Max];
	volatile int AimH[ipc_eSen_Max];
	volatile int crossAxisWidth[ipc_eSen_Max];
	volatile int crossAxisHeight[ipc_eSen_Max];
	volatile int picpCrossAxisWidth;
	volatile int picpCrossAxisHeight;
	volatile bool osdTextShow;
	volatile bool osdDrawShow;
	volatile bool crossDrawShow[CAMERACHID];
	volatile bool osdUserShow;
	volatile bool osdBoxShow[CAMERACHID];
	volatile bool osdChidIDShow[CAMERACHID];
	volatile bool osdChidNameShow[CAMERACHID];
	volatile int osdTextColor;
	volatile int osdTextAlpha;
	volatile int osdTextFont;
	volatile int osdTextSize;
	volatile int osdDrawColor;

	volatile int ipcballComAddress;
	volatile int ipcballComBaud;

	/***** old status ,remaining tidy*****/	
	volatile unsigned int  unitVerNum;      	// 1.23=>0x0123
	volatile unsigned int  unitFaultStat;   		// bit0:tv input bit1:fr input bit2:avt21
	volatile unsigned int  unitFaultStatpri;   	// bit0:tv input bit1:fr input bit2:avt21
	volatile unsigned char  SysMode; 		// 0 --- init ; 1 ---normal  2---settiing
	volatile unsigned char  FovCtrl; 
	volatile unsigned char  FovStat;       /* 1 byte ext-input fov:0 Large fov ,1 midle fov,2 small fov,3 electric x2 fov */
	volatile float  unitFovAngle[ipc_eSen_Max];
	
	volatile unsigned int TrkStatpri;
	volatile unsigned int TrkStat;     // acp/trk/assi/lost

	volatile int  unitAimW;      	// aim size
	volatile int  unitAimH;      	// aim size
	volatile int  unitAimX;	   	// track aimRect x ,mean to the avtPosX
	volatile int  unitAimY;
	
	volatile float TrkX;    		// for report and osd text 	what
	volatile float TrkY;    		// for report and osd text
	volatile float TrkXtmp;    	// for report and osd text
	volatile float TrkYtmp;    	// for report and osd text

	volatile unsigned int  AvtTrkStat;      		// eTrkMode 
	volatile unsigned int  AvtTrkAimSize;   	// 0-4
	volatile unsigned int  AvtCfgSave;      	// eSaveMode
	volatile unsigned int  AvtTrkCoast;
	volatile unsigned int  TrkErrFeedback;  	// eTrkMode 

	volatile unsigned int SceneAvtTrkStat; 
	volatile unsigned int PatternDetect;

	volatile float  trkerrx;
	volatile float  trkerry;	

	/***** cmd stat part *****/
	volatile unsigned int SensorStatBegin;
	volatile unsigned int  SensorStat; 
	volatile unsigned int  SensorStatpri;       		
	volatile unsigned int  changeSensorFlag;
	volatile unsigned int  PicpSensorStat;  		
	volatile unsigned int  PicpSensorStatpri; 		
	volatile unsigned int  PicpPosStat;			
	volatile unsigned int  ImgZoomStat[ipc_eSen_Max];   	
	volatile unsigned int  ImgEnhStat[ipc_eSen_Max];    	
	volatile unsigned int  ImgBlobDetect[ipc_eSen_Max];    
	volatile unsigned int  ImgFrezzStat[ipc_eSen_Max];
	volatile unsigned int  ImgVideoTrans[ipc_eSen_Max]; 
	volatile unsigned int  ImgPicp[ipc_eSen_Max];   	

	volatile unsigned int  MmtValid;    			
	volatile unsigned int  MmtPixelX;
	volatile unsigned int  MmtPixelY;
	volatile unsigned int  MmtStat[ipc_eSen_Max];    	
	volatile unsigned int  MmtSelect[ipc_eSen_Max];

	volatile unsigned char  MMTTempStat;		//ack mmt stat
	volatile unsigned char  Mmttargetnum; 			

	volatile unsigned char  MtdState[ipc_eSen_Max];	//record moving obj detect state of each channel
	volatile unsigned int  MtdSelect[ipc_eSen_Max];
	volatile unsigned int  MtdSetRigion;
		
	/***** cmd osd part *****/
	volatile unsigned int  DispGrp[ipc_eSen_Max];       	// eDispGrade
	volatile unsigned int  DispColor[ipc_eSen_Max];  	// eOSDColor or eGRPColor
	volatile unsigned char Imgalgosdrect;
	
	//don't know the usage
	volatile unsigned int  TrkCoastCount;
	volatile unsigned int  FreezeresetCount;

	//may be not useful
	volatile unsigned int  TvCollimation;   //dianshi zhunzhi   not understanding
	volatile unsigned int  FrCollimation;   //rexiang zhunzhi
	
	volatile unsigned int  	ImgMmtshow[ipc_eSen_Max];	//not sure show what
	volatile unsigned char 	MmtOffsetXY[20]; 		//not sure the func
	
} IMGSTATUS;

typedef struct {
	int MAIN_Sensor;	//13--0
	int Timedisp_9;
	bool OSD_text_show;
	int OSD_text_color;
	int OSD_text_alpha;
	int OSD_text_font;
	int OSD_text_size;
	bool OSD_draw_show;
	bool CROSS_draw_show[CAMERACHID];
	bool osdUserShow;
	bool osdBoxShow[CAMERACHID];
	bool osdChidIDShow[CAMERACHID];
	bool osdChidNmaeShow[CAMERACHID];
	int OSD_draw_color;
	int CROSS_AXIS_WIDTH;
	int CROSS_AXIS_HEIGHT;
	int Picp_CROSS_AXIS_WIDTH;
	int Picp_CROSS_AXIS_HEIGHT;
	int ch0_acqRect_width;
	int ch1_acqRect_width;
	int ch2_acqRect_width;	//13--15
	int ch3_acqRect_width;	// 14 -- 0
	int ch4_acqRect_width;
	int ch5_acqRect_width;
	int ch0_acqRect_height;
	int ch1_acqRect_height;
	int ch2_acqRect_height;
	int ch3_acqRect_height;
	int ch4_acqRect_height;
	int ch5_acqRect_height;
	int ch0_aim_width;
	int ch1_aim_width;
	int ch2_aim_width;
	int ch3_aim_width;
	int ch4_aim_width;
	int ch5_aim_width;
	int ch0_aim_height;
	int ch1_aim_height;	
	int ch2_aim_height;
	int ch3_aim_height;
	int ch4_aim_height;
	int ch5_aim_height;
}OSDSTATUS;

typedef struct {
	float occlusion_thred;//9--0
	float retry_acq_thred;
	float up_factor;
	int res_distance;
	int res_area;
	int gapframe;
	bool enhEnable;
	float cliplimit;
	bool dictEnable;
	int moveX;
	int moveY;
	int moveX2;
	int moveY2;
	int segPixelX;
	int segPixelY;
	bool  algOsdRect_Enable;  //9--15
	
	int	ScalerLarge;//10--0
	int	ScalerMid; 
	int	ScalerSmall;
	int	Scatter;
	float	ratio;
	bool	FilterEnable;
	bool	BigSecEnable;
	int	SalientThred;
	bool	ScalerEnable;
	bool	DynamicRatioEnable;
	int	acqSize_width;
	int	acqSize_height;
	bool	TrkAim43_Enable;
	bool	SceneMVEnable;
	bool	BackTrackEnable;
	int	bAveTrkPos; //10--15

	float	fTau; //11--0
	int	buildFrms;
	int	LostFrmThred;
	float	histMvThred;
	int	detectFrms;
	int	stillFrms;
	float	stillThred;
	bool	bKalmanFilter;
	float	xMVThred;
	float	yMVThred;
	float	xStillThred;
	float	yStillThred;
	float	slopeThred;
	float	kalmanHistThred;
	float	kalmanCoefQ;
	float	kalmanCoefR; //11--15

	int Enhmod_0; //12--0
	float Enhparm_1;
	int Mmtdparm_2;
	int Mmtdparm_3;
	int Mmtdparm_4;
	int Mmtdparm_5;
	int Mmtdparm_6;
	float Mmtdparm_7;
	int Mmtdparm_8; //12--8

}UTCTRKSTATUS;

typedef struct{
	unsigned int distance:1;
	unsigned int speed:1;
	unsigned int focus:1;
	unsigned int time:1;
	unsigned int plantAgle:1;
	unsigned int targetPost:1;
	unsigned int navDuration:1;
	unsigned int commuDuration:1;

	unsigned int  workPlace:1;
	unsigned int softVer:1;
	unsigned int reserve0:6;
	
	unsigned int   focusFeedBack:16;
	unsigned int   speedInfo:8;
	unsigned int   yearInfo:8;
	unsigned int   monthInfo:8;
	unsigned int   dayInfo:8;
	unsigned int   timeHours:8;
	unsigned int  timeMini:8;
	unsigned int  timeSecnd:8;
	unsigned int plantTilt1:8;
	unsigned int plantTilt2:8;
	unsigned int plantPan:16;
	unsigned int lgitudeAgl:8;
	unsigned int lgitudeMini:8;
	unsigned int lgitudeSencd:8;
  	unsigned int latitudeAgl:8;
	unsigned int latitudeMini:8;
	unsigned int latitudeSencd:8;
	unsigned int distanceInfo:16;
	unsigned int navDuratInfo1:8;
	unsigned int navDuratInfo2:8;
	unsigned int commuDuratInfo:16;
}LKOSDSTATUS;

typedef struct{
	int IrisAndFocusAndExit;
	int OSD_workMode;
}OSD_ctrlParams;

typedef struct out_frame_angle
{
    volatile unsigned char out_frame_angle_a[3];
    volatile unsigned char out_frame_angle_e[3];
}out_frame_angle_t;

typedef struct in_frame_angle
{
    volatile unsigned char  in_frame_angle_a[3];
    volatile unsigned char  in_frame_angle_e[3];
}in_frame_angle_t;

typedef struct to_osd_time
{
    volatile unsigned char year[2];
    volatile unsigned char month;
    volatile unsigned char day;
    volatile unsigned char hour;
    volatile unsigned char min;
    volatile unsigned char sec;
    volatile unsigned char msec;
}to_osd_time_t;

typedef struct work_mode
{
    volatile unsigned char mode;
}work_mode_t;

typedef struct laser_work_mode
{
    volatile unsigned char laser_mode;
}laser_work_mode_t;

typedef struct laser_range
{
    volatile unsigned char range[2];
}laser_range_t;

typedef struct man_focal_length
{
    volatile unsigned char length[2];
}man_focal_length_t;

typedef struct photo_dect_temp
{
    volatile  char temp;
}photo_dect_temp_t;

typedef struct prevent_jam
{
    volatile unsigned char stat;
    volatile unsigned char level;
}prevent_jam_t;

typedef struct osdbuffer
{
	volatile unsigned char osdID;
	volatile unsigned char color;
	volatile unsigned char alpha;
	volatile unsigned char font;
	volatile unsigned char fontsize;
	volatile unsigned char ctrl;
	volatile unsigned int posx;
	volatile unsigned int posy;
	volatile unsigned char buf[128];
}osdbuffer_t;

typedef struct osdtext
{
	volatile unsigned char osdID[32];
	volatile unsigned char color[32];
	volatile unsigned char alpha[32];
	volatile unsigned char font[32];
	volatile unsigned char fontsize[32];
	volatile unsigned char ctrl[32];
	volatile unsigned int posx[32];
	volatile unsigned int posy[32];
	volatile unsigned char buf[32][128];
}osdtext_t;


static IMGSTATUS *ipc_getimgstatus_p()
{
	return (IMGSTATUS*)IpcHandl[IPC_SHA].ptr;
}

static OSDSTATUS *ipc_getosdstatus_p()
{
	return (OSDSTATUS*)IpcHandl[IPC_OSD_SHA].ptr;
}

static UTCTRKSTATUS *ipc_getutstatus_p()
{
	return (UTCTRKSTATUS*)IpcHandl[IPC_UTCTRK_SHA].ptr;
}

static LKOSDSTATUS *ipc_getlkosdstatus_p()
{
	return (LKOSDSTATUS*)IpcHandl[IPC_LKOSD_SHA].ptr;
}
static osdtext_t *ipc_getosdtextstatus_p()
{
	return (osdtext_t*)IpcHandl[IPC_OSDTEXT_SHA].ptr;
}

static int ipc_settrack(unsigned int trackstatus, float trackposx, float trackposy)
{
	int ret=0;
	IMGSTATUS* IMGstatus = (IMGSTATUS*)IpcHandl[IPC_SHA].ptr;
	if(IMGstatus==NULL)
		return -1;
	ret = sharedMemoryLock(IpcHandl[IPC_SHA].IPCID);
	ipc_status_P();
	IMGstatus->TrkStat=trackstatus;
	IMGstatus->trkerrx=trackposx;
	IMGstatus->trkerry=trackposy;
	ipc_status_V();
	ret = sharedMemoryUnlock(IpcHandl[IPC_SHA].IPCID);
	return ret;
}

static int ipc_gettrack(unsigned int* trackstatus, float* trackposx, float* trackposy)
{
	int ret=0;
	IMGSTATUS* IMGstatus = (IMGSTATUS*)IpcHandl[IPC_SHA].ptr;
	if(IMGstatus==NULL)
		return -1;
	if(trackstatus==NULL||trackposx==NULL||trackposy==NULL)
		return -1;
	ipc_status_P();
	*trackstatus=IMGstatus->TrkStat;
	*trackposx=IMGstatus->trkerrx;
	*trackposy=IMGstatus->trkerry;
	ipc_status_V();
	return ret;
}

static void Ipc_init()
{
	IpcHandl.resize(IPC_MAX);

	IpcHandl[IPC_TOIMG_MSG].name = DEFAULTPATH;
	IpcHandl[IPC_TOIMG_MSG].Identify = IPC_TOIMG_MSG;
	IpcHandl[IPC_TOIMG_MSG].Class = IPC_Class_MSG;
	IpcHandl[IPC_TOIMG_MSG].IPCID = 0;
	IpcHandl[IPC_TOIMG_MSG].length = 0;
	IpcHandl[IPC_TOIMG_MSG].ptr = NULL;
	
	IpcHandl[IPC_FRIMG_MSG].name = DEFAULTPATH;
	IpcHandl[IPC_FRIMG_MSG].Identify = IPC_FRIMG_MSG;
	IpcHandl[IPC_FRIMG_MSG].Class = IPC_Class_MSG;
	IpcHandl[IPC_FRIMG_MSG].IPCID = 0;
	IpcHandl[IPC_FRIMG_MSG].length = 0;
	IpcHandl[IPC_FRIMG_MSG].ptr = NULL;
	
	IpcHandl[IPC_SHA].name = DEFAULTPATH;
	IpcHandl[IPC_SHA].Identify = IPC_SHA;
	IpcHandl[IPC_SHA].Class=IPC_Class_SHA;
	IpcHandl[IPC_SHA].IPCID = 0;
	IpcHandl[IPC_SHA].length = sizeof(IMGSTATUS);
	IpcHandl[IPC_SHA].RWmode = shm_rdwr;
	IpcHandl[IPC_SHA].ptr = NULL;

	IpcHandl[IPC_OSD_SHA].name = DEFAULTPATH;
	IpcHandl[IPC_OSD_SHA].Identify = IPC_OSD_SHA;
	IpcHandl[IPC_OSD_SHA].Class = IPC_Class_SHA;
	IpcHandl[IPC_OSD_SHA].IPCID = 0;
	IpcHandl[IPC_OSD_SHA].length = sizeof(OSDSTATUS);
	IpcHandl[IPC_OSD_SHA].RWmode = shm_rdwr;
	IpcHandl[IPC_OSD_SHA].ptr = NULL;

	IpcHandl[IPC_UTCTRK_SHA].name = DEFAULTPATH;
	IpcHandl[IPC_UTCTRK_SHA].Identify = IPC_UTCTRK_SHA;
	IpcHandl[IPC_UTCTRK_SHA].Class = IPC_Class_SHA;
	IpcHandl[IPC_UTCTRK_SHA].IPCID = 0;
	IpcHandl[IPC_UTCTRK_SHA].length = sizeof(UTCTRKSTATUS);
	IpcHandl[IPC_UTCTRK_SHA].RWmode = shm_rdwr;
	IpcHandl[IPC_UTCTRK_SHA].ptr = NULL;

	IpcHandl[IPC_LKOSD_SHA].name = DEFAULTPATH;
	IpcHandl[IPC_LKOSD_SHA].Identify = IPC_LKOSD_SHA;
	IpcHandl[IPC_LKOSD_SHA].Class = IPC_Class_SHA;
	IpcHandl[IPC_LKOSD_SHA].IPCID = 0;
	IpcHandl[IPC_LKOSD_SHA].length = sizeof(LKOSDSTATUS);
	IpcHandl[IPC_LKOSD_SHA].RWmode = shm_rdwr;
	IpcHandl[IPC_LKOSD_SHA].ptr = NULL;

	IpcHandl[IPC_OSDTEXT_SHA].name = DEFAULTPATH;
	IpcHandl[IPC_OSDTEXT_SHA].Identify = IPC_OSDTEXT_SHA;
	IpcHandl[IPC_OSDTEXT_SHA].Class = IPC_Class_SHA;
	IpcHandl[IPC_OSDTEXT_SHA].length = sizeof(osdtext_t);
	IpcHandl[IPC_OSDTEXT_SHA].RWmode = shm_rdwr;
	IpcHandl[IPC_OSDTEXT_SHA].ptr = NULL;
	
	IpcHandl[IPC_SEM].name = DEFAULTPATH;
	IpcHandl[IPC_SEM].Identify = IPC_SEM;
	IpcHandl[IPC_SEM].Class = IPC_Class_SEM;	
	IpcHandl[IPC_SEM].IPCID = 0;
	IpcHandl[IPC_SEM].ptr = NULL;

	return;
}



#endif /* IPC_CUSTOM_HEAD_HPP_ */

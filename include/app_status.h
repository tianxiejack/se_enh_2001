
#ifndef _GLOBAL_STATUS_H_
#define _GLOBAL_STATUS_H_

#ifdef __cplusplus
extern "C" {
#endif

//#include "Ipcctl.h"

#ifndef MTD_TARGET_NUM
#define MTD_TARGET_NUM  8
#endif

typedef enum{
	Disable = 0x00,
	Enable_Iris,
	Enable_Focus,
	up,
	down
}IrisAndFocus;

typedef struct{
	volatile unsigned int  DispColor;
	volatile unsigned char DispFont;
	volatile unsigned char DispSize;
	volatile unsigned char DispSwitch;
	volatile unsigned char DispID;
	volatile short DispPosX;
	volatile short DispPosY;
	volatile unsigned char DispAlpha;
}OSD_param;


typedef enum {
	eSen_CH0	= 0x00,
	eSen_CH1   ,
	eSen_CH2   ,
	eSen_CH3   ,
	eSen_CH4   ,
	eSen_CH5   ,
	eSen_Max   
}eSenserStat;

typedef enum {
	ePicp_top_left = 0x00,
	ePicp_top_right = 0x01,
	ePicp_bot_left = 0x02,
	ePicp_bot_right = 0x03,
}ePicpPosStat;

typedef enum Dram_SysMode
{
    INIT_MODE           = 0x00,
    CHECK_MODE      = 0x01,
    AXIS_MODE           = 0x02,
    NORMAL_MODE     = 0x03,
} eSysMode;

typedef enum Dram_zoomCtrl
{
    eZoom_Ctrl_Pause    = 0x00,
    eZoom_Ctrl_SCAL2    = 0x01,
    eZoom_Ctrl_SCAL4    = 0x02,
    eZoom_Ctrl_ADD      = 0x01,
    eZoom_Ctrl_SUB      = 0x02,
} eZoomCtrl;

typedef enum Dram_ImgAlg
{
    eImgAlg_Disable     = 0x00,
    eImgAlg_Enable      = 0x01,
} eImgAlgStat;  // use by img zoom/enh/stb/mtd/rst

typedef enum Dram_MMTSelect
{
    eMMT_No     = 0x00,
    eMMT_Next       = 0x01,
    eMMT_Prev       = 0x02,
    eMMT_Select = 0x03,
} eMMTSelect;

typedef enum Dram_DispGradeStat
{
    eDisp_hide      = 0x00,
    eDisp_show_rect     = 0x01,
    eDisp_show_text     = 0x02,
    eDisp_show_dbg      = 0x04,
} eDispGrade;

typedef enum Dram_DispGradeColor
{
    ecolor_Default  = 0x0,
    ecolor_Black = 0x1,
    ecolor_White    = 0x2,
    ecolor_Red = 0x03,
    ecolor_Yellow = 0x04,
    ecolor_Blue = 0x05,
    ecolor_Green = 0x06,
} eOSDColor;

typedef enum
{
    eTrk_Acq        = 0x00,
    eTrk_Normal = 0x01,
    eTrk_Assi       = 0x02,
    eTrk_Lost       = 0x03,
} eTrkStat;

typedef enum Dram_trkMode
{
    // mode cmd
    eTrk_mode_acq       = 0x00,
    eTrk_mode_target    = 0x01,
    eTrk_mode_mtd       = 0x02,
    eTrk_mode_sectrk    = 0x03,
    eTrk_mode_search    = 0x04,
    eTrk_mode_acqmove 	= 0x05,

    eTrk_mode_switch      = 0x100,

} eTrkMode;

typedef enum Dram_trkRefine
{
    eTrk_ref_no     = 0x00,
    eTrk_ref_left   = 0x01,
    eTrk_ref_right  = 0x02,
    eTrk_ref_up     = 0x01,
    eTrk_ref_down   = 0x02,
} eTrkRefine;

typedef enum Dram_saveMode
{
    eSave_Disable       = 0x00,
    eSave_Enable        = 0x01,
    eSave_Cancel        = 0x02,
} eSaveMode;


/** universal status **/
#if 1
typedef struct
{
	/***** new status *****/
	volatile int axisMoveStepX;
	volatile int axisMoveStepY;
	volatile int aimRectMoveStepX;
	volatile int aimRectMoveStepY;
	volatile int validChId;
	volatile int opticAxisPosX[eSen_Max];	//may be same to unitAxisX[eSen_Max]
	volatile int opticAxisPosY[eSen_Max];
	volatile int AxisPosX[eSen_Max];	
	volatile int AxisPosY[eSen_Max];
	volatile int AvtPosX[eSen_Max];	//target avt x,y for each channel
	volatile int AvtPosY[eSen_Max];
	
	volatile int AcqRectW[eSen_Max];
	volatile int AcqRectH[eSen_Max];
	volatile int AimW[eSen_Max];
	volatile int AimH[eSen_Max];
	volatile int crossAxisWidth[eSen_Max];
	volatile int crossAxisHeight[eSen_Max];
	volatile int picpCrossAxisWidth;
	volatile int picpCrossAxisHeight;
	volatile bool osdTextShow;
	volatile bool osdDrawShow;
	volatile bool crossDrawShow;
	volatile int osdTextColor;
	volatile int osdTextAlpha;
	volatile int osdTextFont;
	volatile int osdTextSize;
	volatile int osdDrawColor;

	/***** old status ,remaining tidy*****/	
	volatile unsigned int  unitVerNum;      	// 1.23=>0x0123
	volatile unsigned int  unitFaultStat;   		// bit0:tv input bit1:fr input bit2:avt21
	volatile unsigned int  unitFaultStatpri;   	// bit0:tv input bit1:fr input bit2:avt21
	volatile unsigned char  SysMode; 		// 0 --- init ; 1 ---normal  2---settiing
	volatile unsigned char  FovCtrl; 
	volatile unsigned char  FovStat;       /* 1 byte ext-input fov:0 Large fov ,1 midle fov,2 small fov,3 electric x2 fov */
	volatile float  unitFovAngle[eSen_Max];
	
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
	volatile unsigned int  ImgZoomStat[eSen_Max];   	
	volatile unsigned int  ImgEnhStat[eSen_Max];    	
	volatile unsigned int  ImgBlobDetect[eSen_Max];    
	volatile unsigned int  ImgFrezzStat[eSen_Max];    	
	volatile unsigned int  ImgVideoTrans[eSen_Max]; 
	volatile unsigned int  ImgPicp[eSen_Max];   	

	volatile unsigned int  MmtValid;    			
	volatile unsigned int  MmtPixelX;
	volatile unsigned int  MmtPixelY;
	volatile unsigned int  MmtStat[eSen_Max];    	
	volatile unsigned int  MmtSelect[eSen_Max];

	volatile unsigned char  MMTTempStat;		//ack mmt stat
	volatile unsigned char  Mmttargetnum; 			

	volatile unsigned char  MtdState[eSen_Max];	//record moving obj detect state of each channel
	volatile unsigned int  MtdSelect[eSen_Max];
	
	/***** cmd osd part *****/
	volatile unsigned int  DispGrp[eSen_Max];       	// eDispGrade
	volatile unsigned int  DispColor[eSen_Max];  	// eOSDColor or eGRPColor
	volatile unsigned char Imgalgosdrect;
	
	//don't know the usage
	volatile unsigned int  TrkCoastCount;
	volatile unsigned int  FreezeresetCount;

	//may be not useful
	volatile unsigned int  TvCollimation;   //dianshi zhunzhi   not understanding
	volatile unsigned int  FrCollimation;   //rexiang zhunzhi
	
	volatile unsigned int  	ImgMmtshow[eSen_Max];	//not sure show what
	volatile unsigned char 	MmtOffsetXY[20]; 		//not sure the func
	
} CMD_EXT;
#endif
//typedef IMGSTATUS CMD_EXT;
	
typedef struct
{
    union
    {
        unsigned char c[4];
        float         f;
        int           i;
    } un_data;
} ACK_REG;

typedef struct
{
    unsigned int  NumItems;
    unsigned int  Block[16];        // ackBlk
    ACK_REG Data[16];
} ACK_EXT;

typedef enum 
{
    MSGID_SYS_INIT  = 0x00000000,           ///< internal cmd system init.
    MSGID_SYS_RESET,
    //MSGID_AVT_RESET,

    MSGID_EXT_INPUT_SYSMODE = 0x00000010,   ///< external cmd, system work mode.
    MSGID_EXT_INPUT_SENSOR,                 ///< external cmd, switch sensor.

    MSGID_EXT_INPUT_FOVSTAT,                ///< external cmd, switch fov.
    MSGID_EXT_INPUT_CFGSAVE,                ///< external cmd, config save(here only for axis save)
    MSGID_EXT_INPUT_AXISPOS,                ///< external cmd, set pos of cross axis.
    MSGID_EXT_INPUT_TRACKALG,             ///< external cmd, set track alg.
    MSGID_EXT_INPUT_TRACK,                  ///< external cmd, start track or stop.
    MSGID_EXT_INPUT_AIMPOS,                 ///< external cmd, set pos of aim area.
    MSGID_EXT_INPUT_AIMSIZE,                ///< external cmd, set size of aim area.
    MSGID_EXT_INPUT_COAST,              ///< external cmd, coast set or cannel
    MSGID_EXT_INPUT_FOVSELECT,        //FOVSELECT
    MSGID_EXT_INPUT_SEARCHMOD,        //FOVSELECT
    MSGID_EXT_INPUT_FOVCMD,        //FOVSELECT
    

    // img control
    MSGID_EXT_INPUT_ENPICP = 0x00000020,        ///< external cmd, open picture close.
    MSGID_EXT_INPUT_ENZOOM,                 ///< external cmd, zoom near or far.
    MSGID_EXT_INPUT_ENPICPZOOM,             ///< external cmd, picp zoom near or far.
    MSGID_EXT_INPUT_ENENHAN,                ///< external cmd, open image enhan or close.
    MSGID_EXT_INPUT_ENMTD,                  ///< external cmd, open mtd or close.
    MSGID_EXT_INPUT_MTD_SELECT,              ///< external cmd, select mtd target.
    MSGID_EXT_INPUT_ENSTB,                  ///< external cmd, open image stb or close.
    MSGID_EXT_INPUT_ENRST,                  ///< external cmd, open image rst or close.
    MSGID_EXT_INPUT_RST_THETA,              ///< external cmd, open image rst or close.
    MSGID_EXT_INPUT_ENBDT,              ///< external cmd, open image bdt or close.
    MSGID_EXT_INPUT_ENFREZZ,              ///< external cmd, open image bdt or close.
    MSGID_EXT_INPUT_PICPCROP,              ///< external cmd, open image bdt or close.
    MSGID_EXT_INPUT_VIDEOEN,              ///< external cmd, open image bdt or close.
    MSGID_EXT_INPUT_MMTSHOW,              ///< external cmd, open image bdt or close.
    MSGID_EXT_INPUT_MMTSHOWUPDATE,              ///< external cmd, open image bdt or close.

    // osd control
    MSGID_EXT_INPUT_DISPGRADE = 0x00000030,  ///< external cmd, osd show or hide
    MSGID_EXT_INPUT_DISPCOLOR,              ///< external cmd, osd color
    MSGID_EXT_INPUT_COLOR,                 ///< external cmd, switch input video color.
    MSGID_EXT_INPUT_ALGOSDRECT,

    // video control
    MSGID_EXT_INPUT_VIDEOCTRL,              ///< external cmd, video record or replay.
    MSGID_EXT_UPDATE_OSD,
    MSGID_EXT_UPDATE_ALG,
    MSGID_EXT_UPDATE_CAMERA,
    MSGID_EXT_MVDETECT,
    MSGID_EXT_MVDETECTSELECT
}MSG_PROC_ID;

typedef struct
{
	unsigned int  CaptureWidth  [eSen_Max];    // for width
	unsigned int  CaptureHeight [eSen_Max];    // for height
	unsigned int  CaptureType   [eSen_Max];	   //date type
	unsigned int  DispalyWidth   [eSen_Max];    // dispaly width
	unsigned int  DispalyHeight  [eSen_Max];    // display height
}CMD_SYS;


#ifdef __cplusplus
}
#endif

#endif



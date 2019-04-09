
#ifndef _GLOBAL_STATUS_H_
#define _GLOBAL_STATUS_H_

#include "ipc_custom_head.hpp"

#ifdef __cplusplus
extern "C" {
#endif


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
typedef IMGSTATUS CMD_EXT;
	
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
    MSGID_EXT_MVDETECTSELECT,
	MSGID_EXT_PATTERNDETECT,
    //
    MSGID_EXT_INPUT_SCENETRK,
	
	//mtd param 
	MSGID_EXT_MVDETECTAERA,
	MSGID_EXT_MVDETECTUPDATE,

	//
	MSGID_EXT_SETCURPOS
}MSG_PROC_ID;

typedef struct
{
	unsigned int  CaptureWidth  [eSen_Max];    // for width
	unsigned int  CaptureHeight [eSen_Max];    // for height
	unsigned int  CaptureType   [eSen_Max];	   //date type
	unsigned int  DispalyWidth   [eSen_Max];    // dispaly width
	unsigned int  DispalyHeight  [eSen_Max];    // display height
}CMD_SYS;


typedef struct{
	unsigned int panPos;
	unsigned int tilPos;
	unsigned int zoom;
}LinkagePos_t;



#ifdef __cplusplus
}
#endif

#endif



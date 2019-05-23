
#ifndef _CONFIG_TABLE_H_
#define _CONFIG_TABLE_H_

#define AIM_WIDTH					64
#define AIM_HEIGHT					64

#define AVT_TRK_AIM_SIZE			2

enum devvideo{
	video_gaoqing0=0,
	video_gaoqing,
	video_gaoqing2,
	video_gaoqing3,
	video_pal,
	MAX_CHAN,
};

#define MAIN_CHID					video_gaoqing0
#define PAL_VIRCHID					0

#define VIDEO_DIS_WIDTH		1920
#define VIDEO_DIS_HEIGHT		1080

#define BALL_CHID			video_gaoqing
#define GUN_CHID			video_gaoqing0

#define min_width_ratio 0.2
#define max_width_ratio 0.8
#define min_height_ratio 0.2
#define max_height_ratio 0.8

extern int vcapWH[5][2];
extern int vdisWH[5][2];

////////////////////////////////////////////////////
// sys config table part
#define SYSCONFIG_VERSION     100
#define CFGID_FEILD_MAX	16
#define CFGID_BLOCK_MAX	128
#define CFGID_USEROSD_MAX	32
#define USEROSD_LENGTH	128

#define CFGID_BUILD( blkId, feildId )   ( ( (( blkId) << 4 ) & 0xFF0 ) | ( feildId ) )
#define CFGID_blkId( cfgId )            ( ( (cfgId) >> 4 ) & 0xFF )
#define CFGID_feildId( cfgId )          ( cfgId & 0xF )

typedef enum 
{
	CFGID_JOS_BKID = 0,
	CFGID_PTZ_BKID = 1,
	CFGID_RTS_BKID = 2,
	CFGID_TRK_BKID = 3,		// 3block
	CFGID_OSD_BKID = 6,	// part1 16block
	CFGID_INPUT1_BKID = 22,	// 7block
	CFGID_OSD2_BKID = 29,	// part2 16block
	CFGID_OUTPUT_BKID = 50,
	CFGID_SYS_BKID = 51,
	CFGID_MTD_BKID = 53,	// 2block
	CFGID_INPUT2_BKID = 55,	// 7block
	CFGID_INPUT3_BKID = 62,	// 7block
	CFGID_INPUT4_BKID = 69,	// 7block
	CFGID_INPUT5_BKID = 76,	// 7block
	CFGID_PID_BKID = 100,	// 2block
	CFGID_PLATFM_BKID = 102,

	CFGID_BKID_MAX = 128
}CFGID_BKID;

typedef enum 
{
	CFGID_JOS_BASE = CFGID_BUILD( CFGID_JOS_BKID, 0 ),
	CFGID_JOS_type = CFGID_BUILD( CFGID_JOS_BKID, 0 ),
}CFGID_JOS;

typedef enum 
{
	CFGID_PTZ_BASE = CFGID_BUILD( CFGID_PTZ_BKID, 0 ),
	CFGID_PTZ_comm = CFGID_BUILD( CFGID_PTZ_BKID, 0 ),
}CFGID_PTZ;

typedef enum 
{
	CFGID_RTS_BASE = CFGID_BUILD( CFGID_RTS_BKID, 0 ),
	CFGID_RTS_mainch = CFGID_BUILD( CFGID_RTS_BKID, 0 ),
	CFGID_RTS_mainch2 = CFGID_BUILD( CFGID_RTS_BKID, 1 ),
	CFGID_RTS_trken = CFGID_BUILD( CFGID_RTS_BKID, 3 ),
	CFGID_RTS_trkmode = CFGID_BUILD( CFGID_RTS_BKID, 4 ),
	CFGID_RTS_sectrkstat = CFGID_BUILD( CFGID_RTS_BKID, 5 ),
	CFGID_RTS_trkstat = CFGID_BUILD( CFGID_RTS_BKID, 6 ),
	CFGID_RTS_trkerrx = CFGID_BUILD( CFGID_RTS_BKID, 7 ),
	CFGID_RTS_trkerry = CFGID_BUILD( CFGID_RTS_BKID, 8 ),
	CFGID_RTS_mtden = CFGID_BUILD( CFGID_RTS_BKID, 11 ),
	CFGID_RTS_mtddet = CFGID_BUILD( CFGID_RTS_BKID, 12 ),	// MtdDetectStat
}CFGID_RTS;

typedef enum 
{
	CFGID_TRK_BASE = CFGID_BUILD( CFGID_TRK_BKID, 0 ),
	CFGID_TRK_assitime = CFGID_BUILD( CFGID_TRK_BKID, 15 ),
}CFGID_TRK;

typedef enum 
{
	CFGID_OUTPUT_BASE = CFGID_BUILD( CFGID_OUTPUT_BKID, 0 ),
	CFGID_OUTPUT1_res = CFGID_BUILD( CFGID_OUTPUT_BKID, 4 ),	// HDMI resolution
	CFGID_OUTPUT2_res = CFGID_BUILD( CFGID_OUTPUT_BKID, 5 ),	// SDI resolution
}CFGID_OUTPUT;

typedef enum 
{
	CFGID_SYS_BASE = CFGID_BUILD( CFGID_SYS_BKID, 0 ),
	CFGID_SYSOSD_biten = CFGID_BUILD( CFGID_SYS_BKID, 1 ),		// bit enable
	//CFGID_USROSD_biten = CFGID_BUILD( CFGID_SYS_BKID, 2 ),		// bit enable
	//CFGID_SYSOSD_color = CFGID_BUILD( CFGID_SYS_BKID, 3 ),
}CFGID_SYS;

typedef enum 
{
	CFGID_MTD_BASE = CFGID_BUILD( CFGID_MTD_BKID, 0 ),
	CFGID_MTD_areabox = CFGID_BUILD( CFGID_MTD_BKID, 0 ),
	CFGID_MTD_detnum = CFGID_BUILD( CFGID_MTD_BKID, 1 ),	// detectNum
	CFGID_MTD_upspd = CFGID_BUILD( CFGID_MTD_BKID, 2 ),	// tmpUpdateSpeed
	CFGID_MTD_maxpixel = CFGID_BUILD( CFGID_MTD_BKID, 3 ),
	CFGID_MTD_minpixel = CFGID_BUILD( CFGID_MTD_BKID, 4 ),
	CFGID_MTD_detspd = CFGID_BUILD( CFGID_MTD_BKID, 6 ),		// detectSpeed
	CFGID_MTD_priority = CFGID_BUILD( CFGID_MTD_BKID, 8 ),
}CFGID_MTD;

typedef enum 
{
	CFGID_PID_BASE = CFGID_BUILD( CFGID_PID_BKID, 0 ),
}CFGID_PID;

typedef enum 
{
	CFGID_PLATFM_BASE = CFGID_BUILD( CFGID_PLATFM_BKID, 0 ),
}CFGID_PLATFM;

// user osd part1 ID 0-15
#define CFGID_OSD_BASE(ID)	CFGID_BUILD( CFGID_OSD_BKID+ID, 0 )
#define CFGID_OSD_SHOWEN(ID)	CFGID_BUILD( CFGID_OSD_BKID+ID, 0 )
#define CFGID_OSD_POSX(ID)		CFGID_BUILD( CFGID_OSD_BKID+ID, 1 )
#define CFGID_OSD_POSY(ID)		CFGID_BUILD( CFGID_OSD_BKID+ID, 2 )
#define CFGID_OSD_CONTENT(ID)	CFGID_BUILD( CFGID_OSD_BKID+ID, 4 )
#define CFGID_OSD_COLOR(ID)	CFGID_BUILD( CFGID_OSD_BKID+ID, 5 )
#define CFGID_OSD_ALPHA(ID)	CFGID_BUILD( CFGID_OSD_BKID+ID, 6 )
#define CFGID_OSD_FONT(ID)		CFGID_BUILD( CFGID_OSD_BKID+ID, 7 )
#define CFGID_OSD_SIZE(ID)		CFGID_BUILD( CFGID_OSD_BKID+ID, 8 )
// user osd part2 ID 16-31
#define CFGID_OSD2_BASE(ID)	CFGID_BUILD( CFGID_OSD2_BKID+(ID-16), 0 )
#define CFGID_OSD2_SHOWEN(ID)	CFGID_BUILD( CFGID_OSD2_BKID+(ID-16), 0 )
#define CFGID_OSD2_POSX(ID)	CFGID_BUILD( CFGID_OSD2_BKID+(ID-16), 1 )
#define CFGID_OSD2_POSY(ID)	CFGID_BUILD( CFGID_OSD2_BKID+(ID-16), 2 )
#define CFGID_OSD2_CONTENT(ID)	CFGID_BUILD( CFGID_OSD2_BKID+(ID-16), 4 )
#define CFGID_OSD2_COLOR(ID)	CFGID_BUILD( CFGID_OSD2_BKID+(ID-16), 5 )
#define CFGID_OSD2_ALPHA(ID)	CFGID_BUILD( CFGID_OSD2_BKID+(ID-16), 6 )
#define CFGID_OSD2_FONT(ID)		CFGID_BUILD( CFGID_OSD2_BKID+(ID-16), 7 )
#define CFGID_OSD2_SIZE(ID)		CFGID_BUILD( CFGID_OSD2_BKID+(ID-16), 8 )

// input part1 BKID = CFGID_INPUT##CHID##_BKID (5 channels CFGID_INPUT1_BKID ~ CFGID_INPUT5_BKID)
#define CFGID_INPUT_BASE(BKID)	CFGID_BUILD( BKID, 0 )
#define CFGID_INPUT_CHNAME(BKID)	CFGID_BUILD( BKID, 1 )
#define CFGID_INPUT_CHVALID(BKID)	CFGID_BUILD( BKID, 3 )
#define CFGID_INPUT_CHRES(BKID)	CFGID_BUILD( BKID, 4 )	// resolution
#define CFGID_INPUT_FOVTYPE(BKID)	CFGID_BUILD( BKID, 5 )	// lv 0-n
#define CFGID_INPUT_AIMTYPE(BKID)	CFGID_BUILD( BKID, 7 )	// 0-2
#define CFGID_INPUT_SENISIVITY(BKID)	CFGID_BUILD( BKID, 9 )
#define CFGID_INPUT_DETX(BKID)	CFGID_BUILD( BKID, 10 )
#define CFGID_INPUT_DETY(BKID)	CFGID_BUILD( BKID, 11 )
#define CFGID_INPUT_DETW(BKID)	CFGID_BUILD( BKID, 12 )
#define CFGID_INPUT_DETH(BKID)	CFGID_BUILD( BKID, 13 )
// input part2 BKID = CFGID_INPUT##CHID##_BKID (5 channels CFGID_INPUT1_BKID ~ CFGID_INPUT5_BKID)
#define CFGID_INPUT_AIMLV(BKID)	CFGID_BUILD( BKID+6, 0 )
#define CFGID_INPUT_FIXAIMW(BKID)	CFGID_BUILD( BKID+6, 2 )
#define CFGID_INPUT_FIXAIMH(BKID)	CFGID_BUILD( BKID+6, 3 )
#define CFGID_INPUT_SWAIMW(BKID, SWLV)	CFGID_BUILD( BKID+6, 2+(SWLV*2) )	// SWLV 1-3
#define CFGID_INPUT_SWAIMH(BKID, SWLV)	CFGID_BUILD( BKID+6, 3+(SWLV*2) )	// SWLV 1-3

// sys config table part end
////////////////////////////////////////////////////

/*************** universal algconfig part ***************/
/* mtd.priority:
	1.distance to the center max
	2. ... close
	3. brightness max
	4. brightness min
	5. aera max
	6. aera min
*/
typedef struct{
	// int configBlock[CFGID_MTD_BASE][16];
	int areaSetBox;
	int detectNum;
	int tmpUpdateSpeed;
	int tmpMaxPixel;
	int tmpMinPixel;
	int reserved1[1];
	int detectSpeed;
	int TrkMaxTime;
	int priority;
	int reserved2[7];

	// int configBlock[CFGID_MTD_BASE+0x10][16];
	int reserved3[4];
	int output;
	int outputPolarity;
	int alarm_delay;
	int reserved4[9];

	// not in configBlock part
	int sensitivityThreshold;
	int detectArea_X;
	int detectArea_Y;
	int detectArea_wide;
	int detectArea_high;
}ALG_CONFIG_Mtd;

typedef struct{
	// int configBlock[CFGID_TRK_BASE][16];
	float occlusion_thred;//9--0
	float retry_acq_thred;
	float up_factor;
	float res_distance;
	float res_area;
	float gapframe;
	float enhEnable;
	float cliplimit;
	float dictEnable;
	float moveX;
	float moveY;
	float moveX2;
	float moveY2;
	float segPixelX;
	float segPixelY;
	int losttime;	// ms

	// int configBlock[CFGID_TRK_BASE+0x10][16];
	float	ScalerLarge;//10--0
	float	ScalerMid;
	float	ScalerSmall;
	float	Scatter;
	float	ratio;
	float	FilterEnable;
	float	BigSecEnable;
	float	SalientThred;
	float	ScalerEnable;
	float	DynamicRatioEnable;
	float	acqSize_width;
	float	acqSize_height;
	float	TrkAim43_Enable;
	float	SceneMVEnable;
	float	BackTrackEnable;
	float	bAveTrkPos; //10--15

	// int configBlock[CFGID_TRK_BASE+0x20][16];
	float	fTau; //11--0
	float	buildFrms;
	float	LostFrmThred;
	float	histMvThred;
	float	detectFrms;
	float	stillFrms;
	float	stillThred;
	float	bKalmanFilter;
	float	xMVThred;
	float	yMVThred;
	float	xStillThred;
	float	yStillThred;
	float	slopeThred;
	float	kalmanHistThred;
	float	kalmanCoefQ;
	float	kalmanCoefR; //11--15
}ALG_CONFIG_Trk;

typedef struct{
	int Enhmod_0; //12--0
	float Enhparm_1;
}ALG_CONFIG_Enh;

typedef struct{
	int Mmtdparm_2;
	int Mmtdparm_3;
	int Mmtdparm_4;
	int Mmtdparm_5;
	int Mmtdparm_6;
	float Mmtdparm_7;
	int Mmtdparm_8; //12--8
}ALG_CONFIG_Mmt;

typedef struct{
	int ctrl;	// show or hide
	int posx;
	int posy;
	int type;
	int bufID;
	int color;
	int alpha;
	int font;
	int fontsize;
	int reserved2[7];
}OSD_CONFIG_ITEM;

/*************** universal osdconfig part ***************/
typedef struct{
	OSD_CONFIG_ITEM items[CFGID_USEROSD_MAX];
	wchar_t disBuf[CFGID_USEROSD_MAX][33];
}OSD_CONFIG_USER;

/*************** universal status part ***************/
#define CAMERACHMAX	5
typedef struct
{
	/***** new status *****/
	volatile int axisMoveStepX;
	volatile int axisMoveStepY;
	volatile int aimRectMoveStepX;
	volatile int aimRectMoveStepY;
	volatile int opticAxisPosX[CAMERACHMAX];	//may be same to unitAxisX[eSen_Max]
	volatile int opticAxisPosY[CAMERACHMAX];
	volatile int AxisPosX[CAMERACHMAX];	
	volatile int AxisPosY[CAMERACHMAX];
	volatile int AvtPosX[CAMERACHMAX];	//target avt x,y for each channel
	volatile int AvtPosY[CAMERACHMAX];
	
	volatile int AcqRectW[CAMERACHMAX];
	volatile int AcqRectH[CAMERACHMAX];
	volatile int AimW[CAMERACHMAX];
	volatile int AimH[CAMERACHMAX];
	volatile int crossAxisWidth[CAMERACHMAX];
	volatile int crossAxisHeight[CAMERACHMAX];
	volatile int picpCrossAxisWidth;
	volatile int picpCrossAxisHeight;

	volatile int ipcballComAddress;
	volatile int ipcballComBaud;

	/***** old status ,remaining tidy*****/	
	volatile unsigned int  unitVerNum;      	// 1.23=>0x0123
	volatile unsigned int  unitFaultStat;   		// bit0:tv input bit1:fr input bit2:avt21
	volatile unsigned int  unitFaultStatpri;   	// bit0:tv input bit1:fr input bit2:avt21
	volatile unsigned char  SysMode; 		// 0 --- init ; 1 ---normal  2---settiing
	volatile unsigned char  FovCtrl; 
	volatile unsigned char  FovStat;       /* 1 byte ext-input fov:0 Large fov ,1 midle fov,2 small fov,3 electric x2 fov */
	volatile float  unitFovAngle[CAMERACHMAX];
	
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

	volatile float  trkerrx;
	volatile float  trkerry;	

	/***** cmd stat part *****/
	volatile unsigned int  SensorStat; 
	volatile unsigned int  SensorStatpri;       		
	volatile unsigned int  changeSensorFlag;
	volatile unsigned int  PicpSensorStat;  		
	volatile unsigned int  PicpSensorStatpri; 		
	volatile unsigned int  PicpPosStat;			
	volatile unsigned int  ImgZoomStat[CAMERACHMAX];   	
	volatile unsigned int  ImgEnhStat[CAMERACHMAX];    	
	volatile unsigned int  ImgBlobDetect[CAMERACHMAX];    
	volatile unsigned int  ImgFrezzStat[CAMERACHMAX];
	volatile unsigned int  ImgVideoTrans[CAMERACHMAX]; 
	volatile unsigned int  ImgPicp[CAMERACHMAX];   	

	volatile unsigned int  MmtValid;    			
	volatile unsigned int  MmtPixelX;
	volatile unsigned int  MmtPixelY;
	volatile unsigned int  MmtStat[CAMERACHMAX];    	
	volatile unsigned int  MmtSelect[CAMERACHMAX];

	volatile unsigned char  MMTTempStat;		//ack mmt stat
	volatile unsigned char  Mmttargetnum; 			

	volatile unsigned char  MtdState[CAMERACHMAX];	//record moving obj detect state of each channel
	volatile unsigned int  MtdSelect[CAMERACHMAX];
	//volatile unsigned int  MtdSetRigion;
	volatile unsigned int  MtdDetectStat;

	/***** cmd osd part *****/
	//don't know the usage
	volatile unsigned int  TrkCoastCount;
	volatile unsigned int  FreezeresetCount;

	//may be not useful
	volatile unsigned int  TvCollimation;   //dianshi zhunzhi   not understanding
	volatile unsigned int  FrCollimation;   //rexiang zhunzhi
	
	volatile unsigned int  	ImgMmtshow[CAMERACHMAX];	//not sure show what
	volatile unsigned char 	MmtOffsetXY[20]; 		//not sure the func
	
} IMGSTATUS;

typedef struct{
	volatile int dir;
	volatile int alpha;
}CMD_triangle;

typedef struct {
	int workMode;
	int IrisAndFocusAndExit;
	CMD_triangle cmd_triangle;
	
	int osdDrawShow;
	int osdUserShow;
	int osdDrawColor;

	int algOsdRect;
	int m_crossOsd;
	int m_boxOsd;
	int m_workOsd;
	int m_chidIDOsd;
	int m_chidNameOsd;
	int m_sceneBoxOsd;
}OSDSTATUS;
/*************** universal status end ***************/

#endif

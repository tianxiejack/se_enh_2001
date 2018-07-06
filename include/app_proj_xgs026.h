

#ifndef __APP_PROJ_XGS026_H_
#define __APP_PROJ_XGS026_H_

#define MTD_TARGET_NUM  3

typedef enum
{
    PowerOn   =0xaa ,
    CheckStart =0x01,
    SensorStat,
    CmdTrack,
    CmdMmt,
    CmdGun,
    TrkWaveDoor,
    WaveDoorPosition,
    CMDehance,

    
    CmdPicp,
    CmdZoom,
    FrColl,
    TvColl,
    AimScal,
    TargetPal,

    ReadProcId ,
    CmdFov,

    CmdSerTrk ,
    CmdSerTrkXY ,

    SysInit ,
    SoftVer,
    SysMode,
    FovSelect,
    UpdateTvS,
    UpdateFlir,
    UpdateLrf,
    CmdVcp,
    CmdSaveCross,
    CmdUpdateMeau,
    CmdShowLv,
    CmdShowColor,

    ChangeCrossPos,
    TarCoordinate,
    SysInfoDisp,
    AngleStat,
    DistantStat,
    PTZalgvideo,
    CmdSmallest ,
}extCmdid;


typedef enum
{
    AckPowerOn  =	0X01,
    AckCheck,

    AckPeroidChek,  
    ACK,
    NAK,

    AckSensor   = 0x06,
    AckTrkState = 0x07,
    AckTrkType   = 0x08,
    AckTrkErr,

    AckMtdStat  = 0x0a,
    AckMtdInfo,
    AckGun,
    AckWaveDoor,
    Ackenhance,

    AckPicp=0x0f,
    AckZoom =	0x10,
    AckFrColl,
    AckTvColl,
    AckTarPal,

    AckSerTrk = 0x14,
    AckProId    =   0x15,
    AckWaveDoorPos  = 0x16,
    AckWaveDoorScal     = 0x17,
    AckFov  = 0x18,

    AckTvCollErr = 0x19,
    AckSmallest ,
}extAck;


typedef struct _ret_stat_ack{
	unsigned char SFCC;			// 1 Sync Flag    (0xCC)
	unsigned char SysId;   		// 2 Subsystem ID (0x06)
	unsigned char PckLen;    	// 3 Packet Len   (6-18)
	unsigned char CmdId;
	unsigned char Stuts;
	unsigned char ECode;
	unsigned char ChSum;
}RET_STAT_ACK;


typedef struct _ret_disp_ack{
	unsigned char SFCC;			// 1 Sync Flag    (0xCC)
	unsigned char SysId;   		// 2 Subsystem ID (0x06)
	unsigned char PckLen;    	// 3 Packet Len   (6-18)
	unsigned char CmdId;
	unsigned char Data;
	unsigned char ChSum;
}RET_CTRL_ACK, RET_DISP_ACK;

typedef struct _ret_tvcoll_ack{
	unsigned char SFCC;			// 1 Sync Flag    (0xCC)
	unsigned char SysId;   		// 2 Subsystem ID (0x06)
	unsigned char PckLen;    	// 3 Packet Len   (6-18)
	unsigned char CmdId;
	unsigned char xErrLsb;
	unsigned char xErrMsb;
	unsigned char yErrLsb;
	unsigned char yErrMsb;
	unsigned char ChSum;
}RET_TVCOLL_ACK;


typedef struct _ret_err_ack{
	unsigned char SFCC;			// 1 Sync Flag    (0xCC)
	unsigned char SysId;   		// 2 Subsystem ID (0x06)
	unsigned char PckLen;    	// 3 Packet Len   (6-18)
	unsigned char CmdId;
	unsigned char xErrLsb;
	unsigned char xErrMsb;
	unsigned char yErrLsb;
	unsigned char yErrMsb;
	unsigned char ChSum;
}RET_ERR_ACK;

typedef struct _ret_track_offset{
	unsigned char SFCC;			// 1 Sync Flag    (0xCC)
	unsigned char SysId;   		// 2 Subsystem ID (0x06)
	unsigned char PckLen;    	// 3 Packet Len 0x09
	unsigned char CmdId;		// 0x09
	unsigned short offsetX;		// [0-1024]
	unsigned short offsetY;		// [0-768]
	unsigned char ChSum;
}RET_TRACE_OFFSET;

typedef struct _ret_mmt_info{
	unsigned char SFCC;			// 1 Sync Flag    (0xCC)
	unsigned char SysId;   		// 2 Subsystem ID (0x06)
	unsigned char PckLen;    	// 3 Packet Len   (6-18)
	unsigned char CmdId;
	unsigned char MMTCount;		// max 3
	unsigned char offsetXY[20];	// xy(x1y1x2y2x3y3) is [0,0]~[1024,768]
	unsigned char ChSum;
}RET_MMT_INFO;


typedef struct _proc_id_ack{
	unsigned char SFCC;			// 1 Sync Flag    (0xCC)
	unsigned char SysId;   		// 2 Subsystem ID (0x06)
	unsigned char PckLen;    	// 3 Packet Len   (6-18)
	unsigned char CmdId;
	unsigned char IDInfoH;
	unsigned char IDInfoL; 
	unsigned char ID_year;
	unsigned char ID_month;
	unsigned char ID_day;
	unsigned char ChSum;
}PROC_ID_ACK;


typedef struct _ret_tri_ack{
	unsigned char SFCC;			// 1 Sync Flag    (0xCC)
	unsigned char SysId;   		// 2 Subsystem ID (0x06)
	unsigned char PckLen;    	// 3 Packet Len   (6-18)
	unsigned char CmdId;
	unsigned char ChSum;
}RET_TRI_ACK;
typedef struct sysstatus
{
	int errx;
	int erry;
	unsigned char MtdNum;	
}sysInfo;

typedef struct _EXT_Comm_Head 
{
	unsigned char SFCC;			// 1 Sync Flag    (0xCC)
	unsigned char SysId;   		// 2 Subsystem ID (0x06)
	unsigned char PckLen;    	// 3 Packet Len   (6-18)
}extCommHead_t;


typedef struct _ret_enhance_ack{
	unsigned char SFCC;			// 1 Sync Flag    (0xCC)
	unsigned char SysId;   		// 2 Subsystem ID (0x06)
	unsigned char PckLen;    	// 3 Packet Len   (6-18)
	unsigned char CmdId;
	unsigned char Data;
	unsigned char ChSum;
}RET_Enhance;


typedef struct _ret_picp_ack{
	unsigned char SFCC;			// 1 Sync Flag    (0xCC)
	unsigned char SysId;   		// 2 Subsystem ID (0x06)
	unsigned char PckLen;    	// 3 Packet Len   (6-18)
	unsigned char CmdId;
	unsigned char Data;
	unsigned char ChSum;
}RET_Picp;

typedef struct  _SysInfo_
{
    char SysSub;
    char SysStat;
}SysInfo_t;

static int vcapWH[2][2] = {{720, 576}, {640, 512}};

static int vdisWH[2][2] = {{720, 576}, {320, 256}};		// main and picp

static double FOVw[2/*eSen_Max*/][5][2] =
{
	// TV
	{
	{24.0, 19.2},
	{9.6, 7.68}, 
	{3.3, 2.64},
	{1.1, 0.88},
	{0.55, 0.44},
	},
	// FR
	{
	  {40.0, 32.0},
	  {10.0, 8.00}, 
	  {3.3, 2.64},
	  {1.2, 0.96},
	  {0.6, 0.48},
	},
};


static int trkWinWH[2/*eSen_Max*/][5][2] = 
{
	// TV
	{
	{16, 16},
	{32, 32}, 
	{48, 48},
	{64, 64},
	{80, 80},
	},
	// FR
	{
	    {8, 8},
	    {16, 16},
	    {24, 24}, 
	    {32, 32},
	    {40, 40},
	},
};

static int trkWinWHZoom[2/*eSen_Max*/][5][2] = 
{
	// TV
	{
	{16, 16},
	{16, 16}, 
	{32, 32},
	{48, 48},
	{64, 64},
	},
	// FR
	{
	    {8, 8},
	    {16, 16},
	    {24, 24}, 
	    {32, 32},
	    {40, 40},
	},
};

static int trkWinWH43[2][5][2] = 
{
	// TV
	{
		{16, 12},
		{32, 24}, 
		{48, 36},
		{64, 48},
		{80, 60},
	},
	// FR
	{
		{8, 6},
		{16, 12},
		{24, 18}, 
		{32, 24},
		{40, 30},
	},
};


static int fovSize[2][5] = 
{
	{
		2400,
		960,
		330,
		0,
		0
	},
	
	{
		4000,
		2400,
		1000,
		330,
		120
	}

};

static char gWorkStr[11][10] =
{     
	{'I','N','I','T','\0',},
	{'S','T','O','W','\0',},
	{'C','A','G','E','\0',},
	{'M','A','N','U','A','L','\0'},
	{'S','C','A','N','\0',},
	{'T','R','A','C','K','\0',},
	{'G','P','C',' ','T','R','A','C','K','\0'},
	{'S','L','A','V','E','\0',},
	{'S','E','R','V','I','C','E','\0'},
	{'S','T','E','P','S','C','A','N','\0'},
	{'\0'},
};
typedef enum
{
    WINID_TV_CROSSAXIS = 0,
    WINID_FR_CROSSAXIS = 1,
    WINID_TV_AIMAREA,
    WINID_FR_AIMAREA,
    WINID_TV_MMT_0,
    WINID_TV_MMT_1,
    WINID_TV_MMT_2,
    WINID_TV_MMT_3,
    WINID_TV_MMT_4,
    WINID_TV_MMT_5,
    WINID_TV_MMT_6,
    WINID_TV_MMT_7,
    WINID_TV_MMT_8,
    WINID_TV_MMT_9,

    WINID_FR_MMT_0,
    WINID_FR_MMT_1,
    WINID_FR_MMT_2,
    WINID_FR_MMT_3,
    WINID_FR_MMT_4,
    WINID_FR_MMT_5,
    WINID_FR_MMT_6,
    WINID_FR_MMT_7,
    WINID_FR_MMT_8,
    WINID_FR_MMT_9,

    WINID_TV_HORI_MENU, //FOR OC25 MENU RECT
    WINID_FR_HORI_MENU,
    WINID_TV_VERT_MENU,//FOR 0C25 FOV MENU RECT
    WINID_FR_VERT_MENU,

    WINID_TV_SECONDTRK_AXIS,//FOR 0C25 SECONDTRK
    WINID_FR_SECONDTRK_AXIS,
    
    WINID_TV_SECONDTRK_GUIDE,//FOR 0C25 SECONDTRK
    WINID_FR_SECONDTRK_GUIDE,
        
    WINID_GRAPHIC_MAX,
} osdCR_graph_winId;

typedef enum
{
	WINID_TV_SOFTVER = 0,
	WINID_FR_SOFTVER = 1,
	WINID_TV_SENSANGLE,
	WINID_FR_SENSANGLE,
	WINID_TV_WORKMOD,
	WINID_FR_WORKMOD,
	WINID_TV_UPDATESTAT,//FOR OC25  TV/IR/LRF STAT
	WINID_FR_UPDATESTAT,
	WINID_TV_AXIS,
	WINID_FR_AXIS,
	WINID_TV_VCP_SYS, // FOR OC25 VCP&SYSTEM
	WINID_FR_VCP_SYS,
	WINID_TV_TRK_TBE,
	WINID_FR_TRK_TBE,
	WINID_TV_TARGET_COORD,//FOR OC25 LON/LAT/ALT
	WINID_FR_TARGET_COORD,
	WINID_TV_FOV_CHOOSE_0, //FOR OC25 FOV CHOOSE
	WINID_FR_FOV_CHOOSE_0,
	WINID_TV_FOV_CHOOSE_1,
	WINID_FR_FOV_CHOOSE_1,
	WINID_TV_UPDATE_MENU_0,//FOR OC25 UPDATE MENU INFO
	WINID_FR_UPDATE_MENU_0,
	WINID_TV_UPDATE_MENU_1,
	WINID_FR_UPDATE_MENU_1,
	WINID_TV_SYSINFO_LOOP_0, //FOR OC25 SYSINFO LOOP
	WINID_FR_SYSINFO_LOOP_0,
	WINID_TV_SYSINFO_LOOP_1,
	WINID_FR_SYSINFO_LOOP_1,
	
	WINID_TEXT_MAX,
} osdCR_text_winId;

typedef enum
{
    eTextId_eCenterPos  = 0x00,
    eTextId_sysTime     = 0x01,
    eTextId_fov,
    eTextId_enh,
    eTextId_workMode,
    eTextId_laserDistance,
    eTextId_laserStat,
    eTextId_laserCode,
    eTextId_axisX,
    eTextId_axisY,
    eTextId_debugX,
    eTextId_debugY,
    eTextId_max
} osdCR_TextId;

#define WINID_MAX (WINID_GRAPHIC_MAX +WINID_TEXT_MAX)
typedef struct
{
    int blk_id;
    int sens_id;
    int wind_id;
    int graph_id;
}osd_id_t;
int APP_text_genContext(int chId, int winId, char * str, int strBufLen, short *textLen);
#endif

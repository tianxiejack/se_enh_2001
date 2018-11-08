
#ifndef CONFIG_ABLE_H__
#define CONFIG_ABLE_H__

#define APP_TRACKER_MODE	1
#define APP_LINKAGE_MODE	0

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

#define MAIN_CHID					video_gaoqing
#define PAL_VIRCHID					0

#define VIDEO_DIS_WIDTH		1920
#define VIDEO_DIS_HEIGHT		1080

extern int vcapWH[5][2];
extern int vdisWH[5][2];

#if 0
typedef struct {
	int MAIN_Sensor;	//13--0
	int Timedisp_9;
	bool OSD_text_show;
	int OSD_text_color;
	int OSD_text_alpha;
	int OSD_text_font;
	int OSD_text_size;
	bool OSD_draw_show;
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
}OSD_Param;


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

}UTC_Trk_Param;
#endif

#endif

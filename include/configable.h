
#ifndef CONFIG_ABLE_H__
#define CONFIG_ABLE_H__

#define AIM_WIDTH					64
#define AIM_HEIGHT					64

#define AVT_TRK_AIM_SIZE			2

enum devvideo{
	video_gaoqing0=0,
	video_gaoqing,
	video_gaoqing2,
	video_gaoqing3,
	MAX_CHAN,
	video_pal,
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


#endif

#ifndef _TARGET
#define _TARGET 1

#include "opencv2/core/core.hpp"

using namespace cv;
using namespace std;

#define MAX_TGT_NUM 12

#define SEARCH_TARGET_NUMBER 5  //MAX 30 ÿ���߶���Ѱ��Ŀ����
#define MAX_TARGET_NUMBER 5//�ܵ����Ŀ�������10
#define MAX_SCALER  4	//min = MIN_SCALER+1, max = 5; 1: CIF, 2:QCIF,3:Q2CIF,4:Q3CIF,5:Q4CIF
#define MIN_SCALER  2	//min = 1,           max = 4; 1: CIF, 2:QCIF,3:Q2CIF,4:Q3CIF,5:Q4CIF
#define TARGET_XSIZE		64 //48	//  �����ܱ�16��
#define	TARGET_YSIZE		32 //16	//  �����ܱ�16��
#define TARGET_INTERRUPT_TIME 25	
#define TARGET_CAPTURE_TIME	  5 // TARGET_CAPTURE_TIME must < TARGET_INTERRUPT_TIME
#define TRACK_SPEED_COEF	0.1		//0.02
#define PRED_SPEED_COEF		0.1	//0.01
#define EDGE_DETECT_THRED	60 //��Ե������ֵ

#define  MEM_ILLEGAL		NULL

#define CHANNAL_NUM 2

typedef struct
{
	unsigned char coef[4][4];
	unsigned char shift;
}gaussDown;

typedef struct
{
	unsigned char ftv;
	int AreaRate;
	int Const;
	int x;
	int y;
	int num;
}FPOINT;
typedef struct
{
	int total;
	FPOINT fp[SEARCH_TARGET_NUMBER];
}FEAT;


typedef struct
{	
	int valid;				//0: invalid, 1: valid
	int live;
	int mode;				// < 50: tracking capture, 100-199: interrupt track
	int manual;				//0:auto detect target, 1:manual detect target
	float cur_x;				//target center x position
	float cur_y;				//target center y position need * 2
	int arearate;
	int constract;
	float xspeed;				//x axe move speed
	float yspeed;				//y axe move speed
}TARGET;


typedef struct
{	
	int startx;
	int starty;
	int endx;
	int endy;
	int valid;	
}TARGETDRAW;

typedef struct{
	int valid;
	Rect Box;
}TARGETBOX;
typedef struct
{
	int state;		//0: nothing to do, 1: detect targets, 2: track targets
    	int i_width;	//ͼ��?��λΪ���أ�
    	int i_height;	//ͼ��ߣ���λΪ���أ�
	unsigned char *Img[6];	//ԭ�ߴ��С��ͼ��
	unsigned char *FtImg[5];//����ͼ��ԭ�ߴ��С��ͼ��
	unsigned char *sobel;
	unsigned char *midSobel[5];
	FEAT ft[6];	//0:total, 1:CIF, 2:QCIF,3:Q2CIF,4:Q3CIF,5:Q4CIF
	
	TARGET tg[MAX_TARGET_NUMBER];		//max 30 target
	
	gaussDown gaussCoefDown;

	
	
}target_t,*ALGMTD_HANDLE;



extern target_t tg[CHANNAL_NUM];

class CMultitarget
{
public:
	CMultitarget();
	~CMultitarget();

public:
	int OpenTarget(target_t *t, int VideoWidth, int VideoHeight);
	int CloseTarget(target_t *t);

	int Process(Mat image, TARGETBOX *tgBox/*vector<Rect> &Boxs*/, bool isInTrack,int chId);
	void MediumFliter(unsigned char *in, unsigned char *out, int w, int h);

	void GaussFliter(unsigned char *in, unsigned char *out, int w, int h);
	void DownSample(target_t *t, unsigned char *in, unsigned char *out, int w, int h);

	void IMG_sobel(unsigned char *in_data, unsigned char *out_data, int w, int h);     
	void CalLightArea(unsigned char *img, unsigned char *val, int w, int h);
	void FindMaxArea(target_t *t, unsigned char *img, int w, int h, int imagesize);
	void WriteCross(FEAT *ftp, unsigned char *img, int w, int h, int imagesize);
	void GetMaxFtp(target_t *t);
	void AutoDetectTarget(unsigned char *img,int chid);
	int GetTargetNumber(target_t *t);
	int InsertTarget(target_t *t, TARGET *target, int Index);
	int DeletTarget(target_t *t, int Index);
	int	DeletFp(FEAT *ft, int index);
	void FilterMultiTarget(target_t *t);

	int SetTarget(bool bStart,int chid);
	int GetTargetPosX(int iTagId,int chid);
	int GetTargetPosY(int iTagId,int chid);
	void calAreaConst(FPOINT *fp, unsigned char *pImg, int w, int h);
	int calEdgePoint(unsigned char *pSob, int w, int h, int x0, int y0);

	void ClearAllTgt(TARGETBOX *tgBox/*vector<Rect> &Boxes*/,int chid);

	int initMTD(Mat frame,int channalid);
public:

};


#endif

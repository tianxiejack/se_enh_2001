
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.hpp>
#include <opencv2/opencv.hpp>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "osa.h"
#include "gst_capture.h"
#include "encTrans.hpp"

#define clip(rslt, a, b)  (rslt>b)?(b):((rslt<a)?a:rslt)

typedef enum{
	CFG_Enable = 0,
	CFG_TransPrm,
	CFG_EncPrm,
	CFG_keyFrame,
	CFG_Max
}EncTrans_CFG;

typedef struct _enctran_trans_param
{
	CAPTURE_SRC srcType;
	//cv::Size imgSize;
	int width;
	int height;
	bool bRtp;
	char destIpaddr[32];
	int destPort;
}ENCTRAN_transPrm;

typedef struct _enctran_enc_param
{
	//int width;
	//int height;
	int fps;
	int bitrate;
	int minQP;
	int maxQP;
	int minQI;
	int maxQI;
	int minQB;
	int maxQB;
}ENCTRAN_encPrm;

typedef struct
{
	int m_nChannels;
	//int m_curTransMask;
	volatile bool m_enable[ENT_CHN_MAX];
	ENCTRAN_transPrm m_transPrm[ENT_CHN_MAX];
	ENCTRAN_encPrm m_encPrm[ENT_CHN_MAX];
	//OSA_BufHndl *m_bufQue[ENT_CHN_MAX];
	//OSA_SemHndl *m_bufSem[ENT_CHN_MAX];
	//OSA_SemHndl *m_semScheduler[ENT_CHN_MAX];
	GstCapture_data m_gstCapture_data[ENT_CHN_MAX];
	RecordHandle * m_record_handle[ENT_CHN_MAX];

}CEncTrans;

static char strFormat[16] = "I420";//"YUY2"//"GRAY8"
static CEncTrans *enctran = NULL;

////////////////////////////////////////////////////////
static int createEncoder(int chId)
{
}

static int deleteEncoder(int chId)
{}

static void EncTrans_pushData(int chId, char *pbuffer, int datasize)
{}

////////////////////////////////////
int EncTrans_create(int nChannels, cv::Size imgSize[])
{}

int EncTrans_destroy(void)
{}

int EncTrans_dynamic_config(EncTrans_CFG type, int iPrm, void* pPrm)
{}

////////////////////////////////////
int EncTrans_screen_rtpout(bool bRtp, int rmIpaddr, int rmPort)
{}

int EncTrans_appcap_rtpout(int chId, bool bRtp, int rmIpaddr, int rmPort)
{}

int EncTrans_appcap_frame(int chId, char *pbuffer, int datasize)
{}



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
	CEncTrans *pPrm = enctran;
	if(pPrm == NULL || chId < 0 || chId >= pPrm->m_nChannels || pPrm->m_record_handle[chId] != NULL)
		return 0;

	memset(&pPrm->m_gstCapture_data[chId], 0, sizeof(GstCapture_data));
	pPrm->m_gstCapture_data[chId].width = pPrm->m_transPrm[chId].width;
	pPrm->m_gstCapture_data[chId].height = pPrm->m_transPrm[chId].height;
	pPrm->m_gstCapture_data[chId].framerate = pPrm->m_encPrm[chId].fps;
	pPrm->m_gstCapture_data[chId].bitrate = pPrm->m_encPrm[chId].bitrate;
	pPrm->m_gstCapture_data[chId].ip_port = pPrm->m_transPrm[chId].destPort;
	pPrm->m_gstCapture_data[chId].filp_method = FLIP_METHOD_VERTICAL_FLIP;
	pPrm->m_gstCapture_data[chId].capture_src = pPrm->m_transPrm[chId].srcType;
	//m_gstCapture_data[chId].format = "YUY2";
	pPrm->m_gstCapture_data[chId].format = strFormat;//"I420";
	if( pPrm->m_transPrm[chId].bRtp)
		pPrm->m_gstCapture_data[chId].ip_addr = &pPrm->m_transPrm[chId].destIpaddr[0];
	else
		pPrm->m_gstCapture_data[chId].ip_addr = NULL;
	//pPrm->m_gstCapture_data[chId].sd_cb=;
	for(int i=0;i<ENC_QP_PARAMS_COUNT;i++)
		pPrm->m_gstCapture_data[chId].Q_PIB[i]=-1;
	//pPrm->m_gstCapture_data[chId].notify = m_semScheduler[chId];

	pPrm->m_record_handle[chId] = gstCaptureInit(pPrm->m_gstCapture_data[chId]);

	return (pPrm->m_record_handle[chId] == NULL) ? -1 : 0;
}

static int deleteEncoder(int chId)
{
	CEncTrans *pPrm = enctran;
	if(pPrm == NULL || chId < 0 || chId >= pPrm->m_nChannels || pPrm->m_record_handle[chId] == NULL)
		return 0;

	gstCaptureUninit(pPrm->m_record_handle[chId]);
	pPrm->m_record_handle[chId] = NULL;

	return 0;
}

static void EncTrans_pushData(int chId, char *pbuffer, int datasize)
{
	//OSA_assert(pixFmt == V4L2_PIX_FMT_YUV420M);
	CEncTrans *pPrm = enctran;
	if(pPrm == NULL || chId < 0 || chId >= pPrm->m_nChannels || pPrm->m_record_handle[chId] == NULL)
		return ;

	if(pPrm->m_enable[chId])
		gstCapturePushData(pPrm->m_record_handle[chId], pbuffer, datasize);
	return ;
}

////////////////////////////////////
int EncTrans_create(int nChannels, cv::Size imgSize[])
{
	if(enctran != NULL)
		return 0;

	//enctran = new CEncTrans;
	//enctran->create();
	enctran = (CEncTrans *)OSA_memAlloc(sizeof(CEncTrans));
	OSA_assert(enctran != NULL);
	memset(enctran, 0, sizeof(CEncTrans));	// default all disable
	enctran->m_nChannels = clip(nChannels, 0, ENT_CHN_MAX);
	for(int chId=0; chId < enctran->m_nChannels; chId++)
	{
		enctran->m_transPrm[chId].width = imgSize[chId].width;
		enctran->m_transPrm[chId].height = imgSize[chId].height;
		enctran->m_encPrm[chId].fps = 30;
		enctran->m_encPrm[chId].bitrate = 5600000;
		//printf("encTrans init ch%d (%d x %d) \r\n", chId, enctran->m_transPrm[chId].width, enctran->m_transPrm[chId].height);
	}

	return 0;
}

int EncTrans_destroy(void)
{
	if(enctran == NULL)
		return 0;

	for(int chId=0; chId < enctran->m_nChannels; chId++)
	{
		deleteEncoder(chId);
	}
	OSA_memFree(enctran);
	enctran = NULL;

	return 0;
}

int EncTrans_dynamic_config(EncTrans_CFG type, int iPrm, void* pPrm)
{
	int ret = OSA_SOK;
	bool bEnable;

	if(enctran == NULL)
		return 0;

	if(type == CFG_Enable){
		if(iPrm >= enctran->m_nChannels || iPrm < 0)
			return -1;
		if(pPrm == NULL)
			return -2;
		bEnable = *(bool*)pPrm;
		if(enctran->m_enable[iPrm] != bEnable)
		{
			enctran->m_enable[iPrm] = bEnable;
			if(enctran->m_enable[iPrm])
				createEncoder(iPrm);
			else
				deleteEncoder(iPrm);
		}
	}
	if(type == CFG_TransPrm){
		if(iPrm >= enctran->m_nChannels || iPrm < 0)
			return -1;
		if(pPrm == NULL)
			return -2;
		memcpy(&enctran->m_transPrm[iPrm], pPrm, sizeof(ENCTRAN_transPrm));
		if(enctran->m_enable[iPrm] && (enctran->m_record_handle[iPrm] != NULL))
		{
			ChangeUDP_remote(enctran->m_record_handle[iPrm], 
								enctran->m_transPrm[iPrm].destIpaddr, 
								enctran->m_transPrm[iPrm].destPort);
			OSA_printf("%d %s ch%d change remote %s:%d ", OSA_getCurTimeInMsec(), __func__, 
						iPrm, enctran->m_transPrm[iPrm].destIpaddr,
						enctran->m_transPrm[iPrm].destPort);
		}
	}
	if(type == CFG_EncPrm){
		if(iPrm >= enctran->m_nChannels || iPrm < 0)
			return -1;
		if(pPrm == NULL)
			return -2;
		memcpy(&enctran->m_encPrm[iPrm], pPrm, sizeof(ENCTRAN_encPrm));
		if(enctran->m_enable[iPrm] && (enctran->m_record_handle[iPrm] != NULL))
		{
			//OSA_mutexLock(&m_mutex);
			ChangeBitRate(enctran->m_record_handle[iPrm], enctran->m_encPrm[iPrm].bitrate);
			ChangeQP_range(enctran->m_record_handle[iPrm],
					enctran->m_encPrm[iPrm].minQP, enctran->m_encPrm[iPrm].maxQP,
					enctran->m_encPrm[iPrm].minQI, enctran->m_encPrm[iPrm].maxQI,
					enctran->m_encPrm[iPrm].minQB, enctran->m_encPrm[iPrm].maxQB);
			OSA_printf("%d %s ch%d %dbps QP %d-%d QI %d-%d QB %d-%d", OSA_getCurTimeInMsec(), __func__, 
					iPrm, enctran->m_encPrm[iPrm].bitrate,
					enctran->m_encPrm[iPrm].minQP, enctran->m_encPrm[iPrm].maxQP,
					enctran->m_encPrm[iPrm].minQI, enctran->m_encPrm[iPrm].maxQI,
					enctran->m_encPrm[iPrm].minQB, enctran->m_encPrm[iPrm].maxQB);
			//OSA_mutexUnlock(&m_mutex);
		}
	}
	if(type == CFG_keyFrame){
		if(iPrm >= enctran->m_nChannels || iPrm < 0)
			return -1;
		if(enctran->m_enable[iPrm] && (enctran->m_record_handle[iPrm] != NULL))
			ChangeBitRate(enctran->m_record_handle[iPrm], enctran->m_encPrm[iPrm].bitrate);
	}

	return 0;
}

////////////////////////////////////
int EncTrans_screen_rtpout(bool bRtp, int rmIpaddr, int rmPort)
{
	bool bRun;
	int chId = 0;
	ENCTRAN_transPrm dyprm;
	struct in_addr in;
	if(enctran == NULL)
		return -1;

	memcpy(&dyprm, &enctran->m_transPrm[chId], sizeof(ENCTRAN_transPrm));
	dyprm.srcType = XIMAGESRC;
	dyprm.bRtp = false;
	if(bRtp && (rmIpaddr != 0))
	{
		dyprm.destPort = clip(rmPort, 11000, 12000);
		in.s_addr = htonl(rmIpaddr);
		strcpy(dyprm.destIpaddr, inet_ntoa(in));
		dyprm.bRtp = true;
	}
	bRun = dyprm.bRtp;

	EncTrans_dynamic_config(CFG_TransPrm, chId, &dyprm);
	EncTrans_dynamic_config(CFG_Enable, chId, &bRun);

	return 0;
}

int EncTrans_appcap_rtpout(int chId, bool bRtp, int rmIpaddr, int rmPort)
{
	bool bRun;
	ENCTRAN_transPrm dyprm;
	struct in_addr in;
	if(enctran == NULL)
		return -1;
	if(chId < 0 || chId >= enctran->m_nChannels)
		return -1;

	memcpy(&dyprm, &enctran->m_transPrm[chId], sizeof(ENCTRAN_transPrm));
	dyprm.srcType = APPSRC;
	dyprm.bRtp = false;
	if(bRtp && (rmIpaddr != 0))
	{
		dyprm.destPort = clip(rmPort, 11000, 12000+enctran->m_nChannels);
		in.s_addr = htonl(rmIpaddr);
		strcpy(dyprm.destIpaddr, inet_ntoa(in));
		dyprm.bRtp = true;
	}
	bRun = dyprm.bRtp;

	EncTrans_dynamic_config(CFG_TransPrm, chId, &dyprm);
	EncTrans_dynamic_config(CFG_Enable, chId, &bRun);

	return 0;
}

int EncTrans_appcap_frame(int chId, char *pbuffer, int datasize)
{
	if(pbuffer == NULL || datasize  <= 0)
		return -1;

	EncTrans_pushData(chId, pbuffer, datasize);
	return 0;
}


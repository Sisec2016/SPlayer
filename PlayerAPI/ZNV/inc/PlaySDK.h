/*********************************************************************
* ��Ȩ���� (C)2008, ������������ά������޹�˾��
*
* �ļ����ƣ�ZteScreen.h
* �ļ���ʶ�� 
* ����ժҪ�� 8000������ؽӿڶ���
* ����˵���� 
* ��ǰ�汾�� 
* ������ڣ� 
*
* �޸ļ�¼1��
* �޸����ڣ�
* �� �� �ţ�
* �� �� �ˣ�
* �޸����ݣ�
* �޸ļ�¼2����
**********************************************************************/
#ifndef _PLAY_SDK_H
#define _PLAY_SDK_H

#if defined( PLAY_SDK_EXPORTS)
	#define PLAY_API  extern "C"__declspec(dllexport) 
#else 
	#define PLAY_API  extern "C" __declspec(dllimport) 
#endif

enum KEY_FRAME_FIND_TYPE
{
	BY_FRAMENUM,	//����ǰ֡��Ų���
	BY_FRAMTIME,	//����ǰ֡ʱ���(���ʱ��)����
};

typedef struct
{
	int	iFilePos;		//��֡���ļ��е���ʼλ
	int	iFrameNum;		//��֡֡���
	__int64	iFrameTime;	//��֡ʱ���(���ʱ��)
}FRAME_POS,*PFRAME_POS;

PLAY_API int Play_GetPort(int *nPort);
PLAY_API int Play_FreePort(int nPort);


PLAY_API int Play_OpenFile(int nPort,int hwnd,char *sFileName);
PLAY_API int Play_CloseFile(int nPort);
PLAY_API int Play_Play(int nPort);
PLAY_API int Play_Stop(int nPort);
PLAY_API int Play_Pause(int nPort);
PLAY_API int Play_Speed(int nPort,int nSpeed); //{-4,-2,1,2,4,8}
PLAY_API int Play_StopSound(int nPort);
PLAY_API int Play_PlaySound(int nPort);
PLAY_API unsigned int Play_GetFileTime(int nPort); //��λ��
PLAY_API unsigned int Play_GetPlayedTime(int nPort); //��λ��
PLAY_API unsigned int Play_GetPlayedPos(int nPort); //��λ1-100%
PLAY_API int Play_SetPlayedTime(int nPort,unsigned int lSecond); //��λ��
PLAY_API int Play_SetPlayedPos(int nPort,unsigned int lpos); //��λ1-100%
PLAY_API int Play_CapturePicture(int nPort,char *fileName);



//��ƵYUV����
typedef void (* Play_VideoYUVCallBack)(int nPort, const char* pYUVBuf, int nLen, long nWidth,long nHeight,unsigned __int64 nStamp,long nType,long nReceived);

PLAY_API int Play_OpenStream(int nPort,int hwnd,char *pFileHeadBuf,unsigned long nSize);
PLAY_API int Play_InputData(int nPort,char * pBuf,unsigned long nSize);
/*	���浱ǰ��/�ļ�������ps��avi��Ƶ�ļ�
	char *fileName(Ҫ������ļ�·��+�ļ��� ��E:\Video.ps)
    int fileFmt(�ļ����ͣ�0: PS����1: AVI����
    int mux��0: ��ϣ�1: ֻ����Ƶ��2: ֻ����Ƶ��
    int videoType��Ĭ�� - 1 ��������չ��תMPEG4��H265��
    int audioType��Ĭ�� - 1 ��������չ��תPCM��AAC)
*/
PLAY_API int Play_startSaveFile(int nPort, char *fileName, int fileFmt, int mux, int videoType, int audioType);
//ֹͣ����
PLAY_API void Play_stopSaveFile(int nPort);
PLAY_API int Play_CloseStream(int nPort);
PLAY_API int Play_SetYUVCallBack(int nPort, Play_VideoYUVCallBack fnHandler,long dwYUVCallbackParam);


//��ȡ��ǰ��Ƶ�ĳ��� Ҫ���û���λ������Ҫ����Ƶ�ĳ��߱���Ϊ׼
PLAY_API int Play_GetVideoInfo(int nPort,int *pwidth,int *pheight);
// �������½�ԭʼͼ������ڴ��ڵ���ʾ����,dRation(��Чȡֵ��ΧΪ0.25��0.5)
PLAY_API int Play_SetSrcImageRation(int nPort,const double dRation);
//ʹ�ܶ�λ
PLAY_API int Play_SetThreeDFlag(int nPort,bool bZoom);
//��λ��������
PLAY_API int Play_SetThreeDRect(int nPort,int dstx,int dsty,int dstw,int dsth);

//Ҫʹ������֡������ʱ������ӿڣ�������� openFileEx ���ļ�
PLAY_API int Play_openFileEx(int nPort, int hWnd, const char * sFileName);
//���õ�ǰ����ʱ��㣬��ȷ�����룬���ʱ��
PLAY_API int Play_setPlayTimeEx(int nPort, __int64 iMilliSecond);
//��ȡ��ǰ����ʱ��㣬��ȷ�����룬���ʱ��
PLAY_API int Play_getPlayTimeEx(int nPort, __int64 *iMilliSecond);
//���õ�ǰ����֡���
PLAY_API int Play_setCurrentFrameNum(int nPort, int iFrameIndex);
//��ȡ��ǰ����֡���
PLAY_API int Play_getCurrentFrameNum(int nPort, int *iFrameIndex);
//��ȡ�ļ���֡��
PLAY_API int Play_getFileTotalFrames(int nPort, int *iFrames);
//��ȡ��ǰ����λ��ǰ��һ���ؼ�֡
PLAY_API int Play_getPreKeyFramePos(int nPort, __int64 iValue, KEY_FRAME_FIND_TYPE iType, PFRAME_POS pFramePos);
//��ȡ��ǰ����λ�ú��һ���ؼ�֡
PLAY_API int Play_getNextKeyFramePos(int nPort, __int64 iValue, KEY_FRAME_FIND_TYPE iType, PFRAME_POS pFramePos);

//���õ�֡����
PLAY_API void Play_SetStepPlay(int nPort);
//�ļ����Ž����ص��ӿ�
typedef void (*FileEndCallBack)(long lParam);
PLAY_API void Play_SetFileEndCallBack(int nPort,FileEndCallBack callback,long lPara);
//��ȡ֡��
PLAY_API void Play_GetFrameRate(int nPort, long *lFrameRate);


#endif

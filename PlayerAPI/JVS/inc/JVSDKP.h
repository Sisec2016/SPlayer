/*////////////////////////////////////////////////////////////////////////////////
// JVSDKP.h
// JVSDKP ��̬���ӿ�ĵ���ͷ�ļ���Ӧ�ó���Ӧ������ͷ�ļ�
// JVSDKP����ʵ�ֶԲɼ�����ѹ�����ݵĽ������ʾ��Ϊ�ϲ�Ӧ�ó����ṩ���ο���
// ��SDK����C��д����C++��ʹ��ʱ��Ҫextern "C"{}
// ������ά���ͿƼ����޹�˾
// �����£�2009-05-16
//////////////////////////////////////////////////////////////////////////////////*/


#ifndef		_JVSDK_H
#define		_JVSDK_H


#define JVSDKP_API extern "C" __declspec(dllexport)


//SDK֪ͨ�¼���lParam
#define	WM_LPARAM_BITMAP	0x01		//ץͼ
#define	WM_LPARAM_STOP		0x02		//�ط�ֹͣ

//֡����
#define JVS_TYPE_P			0x00
#define JVS_TYPE_IDR		0x01
#define JVS_TYPE_AUD		0x02
#define JVS_TYPE_SKIP		0x03
#define JVS_TYPE_D			0x04

typedef struct _JVS_HEADER
{
	unsigned int	i_flags;
	unsigned int	i_frame_width;
	unsigned int	i_frame_height;
	unsigned int	i_total_frames;
} JVS_HEADER, *PJVS_HEADER;


//��ʼ��/�ͷ�SDK
JVSDKP_API	BOOL	__stdcall	JVSP_InitSDK(BOOL bDecodeOnly);
JVSDKP_API	void	__stdcall	JVSP_ReleaseSDK();
JVSDKP_API	void	__stdcall	JVSP_RegisterNotify(HWND hWnd, DWORD dwNotifyMsg);

//��/�رղ�����
JVSDKP_API	int		__stdcall	JVSP_CreatePlayer( BOOL bPreviewRGB );	//ʧ�ܷ���-1������Ϊ���������
JVSDKP_API	BOOL	__stdcall	JVSP_ReleasePlayer (int nPlayer);

JVSDKP_API	BOOL	__stdcall	JVSP_OpenFile ( int nPlayer, char * szFilename );

//����
JVSDKP_API	BOOL	__stdcall	JVSP_Play ( int nPlayer );
JVSDKP_API	void	__stdcall	JVSP_Pause ( int nPlayer );
JVSDKP_API	BOOL	__stdcall	JVSP_Stop ( int nPlayer );
JVSDKP_API	BOOL	__stdcall	JVSP_ZoomIn(int nPlayer, BOOL bZoomIn, int x, int y);
JVSDKP_API	BOOL	__stdcall	JVSP_RefreshWindow(int nPlayer);
JVSDKP_API	void	__stdcall	JVSP_StepForward(int nPlayer);	//������ǰ


//////////////////////////////////////////ץͼ////////////////////////////////
JVSDKP_API	BOOL	__stdcall	JVSP_GetBitmapSize(int nPlayer, DWORD * dwWidth, DWORD * dwHeight);
JVSDKP_API	BOOL	__stdcall	JVSP_GetBitmap(int nPlayer, PUCHAR pBuffer);


//�����趨
JVSDKP_API	void	__stdcall	JVSP_SetFPS(int nPlayer, BOOL bFastPlay, ULONG ulFrameRate);
JVSDKP_API	BOOL	__stdcall	JVSP_SetVideoPreview(int nPlayer, HWND hWnd, RECT * pRect, BOOL bVideoPreview);
JVSDKP_API	void	__stdcall	JVSP_SetVideoCrop(int nPlayer, DWORD dwPixelCroped);

//��ȡ����
JVSDKP_API	ULONG	__stdcall	JVSP_GetTotalFrames(int nPlayer);
JVSDKP_API	ULONG	__stdcall	JVSP_GetCurFrameNum(int nPlayer);
JVSDKP_API	BOOL	__stdcall	JVSP_GetKeyFramePos(int nPlayer, ULONG * ulKeyFramePos, ULONG * ulFrameNum);
JVSDKP_API	BOOL	__stdcall	JVSP_SetAudioPreview(int nPlayer, HWND hWnd, BOOL bPreview);

JVSDKP_API	BOOL	__stdcall	JVSP_GotoKeyFrame(int nPlayer, ULONG ulFrame, BOOL bPause );

//��ģʽ
JVSDKP_API	BOOL	__stdcall	JVSP_OpenStream(int nPlayer, int nFrameWidth, int nFrameHeight);
JVSDKP_API	BOOL	__stdcall	JVSP_WriteToStreamOneFrame(int nPlayer, PUCHAR pBuffer, int nSize);
JVSDKP_API	BOOL	__stdcall	JVSP_CloseStream(int nPlayer);

//����
JVSDKP_API	BOOL	__stdcall	JVSP_PlayKeyFrame(int nPlayer, DWORD dwInterval);
//��ȡѹ���ļ�ͷ����
JVSDKP_API	BOOL	__stdcall	JVSP_GetHeaderData(int nPlayer, PJVS_HEADER pheaderData);

//DirectShow Filter
JVSDKP_API	BOOL	__stdcall	JVSP_GetDecodedFrame(int nPlayer, PUCHAR pFrameData);

//���뿨���
JVSDKP_API	int		__stdcall	JVSP_InitC501();
JVSDKP_API	BOOL	__stdcall	JVSP_BindC501(int nPlayer, int nDevice);
JVSDKP_API	void	__stdcall	JVSP_EnableC501Output(int nPlayer, BOOL bEnableOutput);


#endif
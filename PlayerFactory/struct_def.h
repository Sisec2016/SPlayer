#pragma once


struct T_ConverterParameters{
	int videoCode;
	int width;
	int height;
	int fps;
	int videoBitrate;
	int audioCode;
	int samplerate;
	int channel;
	int audioBitrate;
};

typedef struct
{
	long nPort;
	HDC hdc;
	char * pBuf;
	long nBufLen;
	long nWidth;
	long nHeight;
	unsigned long long nStamp;
	long nType;
	long nUser;
}DISPLAYCALLBCK_INFO;

typedef struct SYSTEM_TIME //����ʱ��
{
	unsigned long dwYear;   //��
	unsigned long  dwMon;    //��
	unsigned long  dwDay;    //��
	unsigned long  dwHour;   //ʱ
	unsigned long  dwMin;    //��
	unsigned long  dwSec;    //��
	unsigned long  dwMs;     //����
}SYSTEM_TIME;

/*
typedef void(CALLBACK* FileEndCallback)(long nID, void *pUser);
typedef void(CALLBACK* DisplayCallback)(DISPLAYCALLBCK_INFO *pstDisplayInfo);
*/
typedef void(__stdcall* FileEndCallback)(long nID, void *pUser);
typedef void(__stdcall* DisplayCallback)(DISPLAYCALLBCK_INFO *pstDisplayInfo);
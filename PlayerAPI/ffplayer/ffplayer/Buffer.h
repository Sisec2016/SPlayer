#pragma once
#include <stdlib.h>
#include <list>
#include <mutex>

#define FRAME_COUNT 4

typedef struct
{
	int		iFrameIndex;		///< ֡������
	short		iFrameType;		///< ֡����
	unsigned int	iFrameLen;
	unsigned __int64		iFilePos;			///< ֡���ļ���λ��
	double	iTimestamp;		///< ֡ʱ�����ֵ
//	unsigned __int64		iTimestampObsolute;	///< ����ʱ��
	int64_t 	iTimestampObsolute;	///< ����ʱ��
	bool		bNewPos;	///< ��ʶ�Ƿ����˱仯��seek),��������˱仯����ʾ����delay
	int serial;
}VIDEO_FRAME;


typedef struct T_Info//˫������
{
	//AVPacket *pk;
	VIDEO_FRAME frameInfo;
	unsigned char *Data;
	int DataLen;
	int size;
	int width;
	int height;
}*PInfo;


class CBuffer
{
public:
	CBuffer();
	~CBuffer();

public:
	static void freeBuf(PInfo *pInfo);
	static PInfo createBuf(int len);
	//void createBuffer(int len=(2800*2800*3/2), int count = FRAME_COUNT);

	void destoryBuffer();

	void insertList(PInfo pInfo);

	PInfo getInfoFromList();

	PInfo getFreeInfoFromList(int len);

	int getCurCount();

public:
//	Event m_eventUsed;
//	Event m_eventValid;
private:
	std::list<PInfo> m_List;
	std::recursive_mutex m_mutexList;
};

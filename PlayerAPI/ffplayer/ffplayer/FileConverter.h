#pragma once

#include "../../../PlayerFactory/struct_def.h"


extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#include "libavutil/time.h"
#include "libavutil/common.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}


#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")

#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "swresample.lib")


#include "SDL.h"
#include "SDL_video.h"
#include "SDL_render.h"
#pragma comment(lib, "sdl2.lib")

enum state{
	FILECUT,
	FILECONVERTER,

};
class CFileConverter
{
public:
	CFileConverter();
	~CFileConverter();
public:
/*	BOOL FileCutStart(const char* srcFileName, const char* destFileName, unsigned __int64 startTime, unsigned __int64 endTime);
	BOOL FileCutClose();
	int FileCutProcess();
	BOOL FileCutPause(DWORD nPause);*/

	BOOL FileConvertStart(const char* srcFileName, const char* destFileName, unsigned __int64 startTime, unsigned __int64 endTime, bool bConvert, T_ConverterParameters *pConvertPara);
	BOOL FileConvertClose();
	int FileConvertProcess();
	BOOL FileConvertPause(DWORD nPause);
private:
	static void GetTotalFrameThread(void *pPara);
	static void ConvertThread(void *pPara);
	void GetTotalFrameThreadRun();	
	void ConvertThreadRun();

	bool OpenSrcFile(char *pSrcFile);
	bool OpenDstFile(char *pDstFile);
	void CloseSrcFile();
	void CloseDstFile();

	int PushVideoFrame(AVPacket &inPacket);
	int PushAudioFrame(AVPacket &inPacket);

	/* Add a video output stream. */
	AVStream* AddVideoStream(AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id);
	/* Add a audio output stream. */
	AVStream* AddAudioStream(AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id);
	// �򿪱����
	int OpenCodec(AVCodec *codec, AVCodecContext *cc);
	// ����Ƶת��context
	AVCodecContext* OpenAudioTranscode(enum AVCodecID audioCodec);
	// ��Ƶת��
	int TranscodeAudio(AVCodecContext* codecContext, const char* buff, unsigned long dwSize);
	// ��Ƶ����
	int EncodeAudio(AVFormatContext* formatContext, AVCodecContext* codecContext, AVFrame* frame);

	// ���������
	int PrintError(int errorCode);

private:
	int m_eState;
	SDL_Thread *m_getTotalFrameThread;
	SDL_Thread *m_convertThread;
	bool m_bCanSeek;
	int m_videoIndex;
	int m_audioIndex;
	bool m_bReadFrameFinish;
	int m_frameTotalCount;
	bool m_bStop;
	int m_progress;
	unsigned long long m_startTime;
	unsigned long long m_stopTime;

	int m_audioInputBuffSize; //��Ƶ���뻺���С PCM
	char* m_audioInputBuff; //��Ƶ���뻺��

	char* m_audioOutputSamples; //��Ƶ������� AAC
	int m_samples_size;//��Ƶ��������С AAC

	SwrContext *m_swr_ctx;

	int m_nRequireSize; //��Ҫ�����ݴ�С PCM
	
	AVFormatContext* m_formatContext;
	AVStream* m_videoStream;
	AVStream* m_audioStream;

	AVFormatContext* m_srcFormatContext;
	AVCodecContext  *m_pSrcVideoCodecCtx;;
	AVFrame         *m_pVideoFrame;
	
	AVCodecContext  *m_pSrcAudioCodecCtx;
	AVFrame* m_audioDecodedFrame;
	AVCodecContext* m_transcodeCodec; //ת��context;
	AVFrame* m_toEncframe;

	bool m_bWriteHeader; //�Ƿ���дͷ

	T_ConverterParameters m_srcFileInfo;
	T_ConverterParameters m_dstConvertPara;

	unsigned char *m_extradata;
	int m_extradata_size;
	bool m_bVideoValid; //��Ƶ����
	bool m_bAudioValid; //��Ƶ����

};


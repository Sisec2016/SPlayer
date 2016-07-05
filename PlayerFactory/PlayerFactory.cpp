// PlayerFactory.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"

#include "PlayerFactory.h"

#include<list>
#include "IPlayer.h"


#include <atlconv.h>
#include <stdio.h>
#include <mutex>
#include <tchar.h>

using namespace std;


struct Factory_Info{
	int fid;//Factory_ID
	IPlayerFactory *pFactory;
};

struct Player_Info{
	int pid;// ������ID
	IPlayer *pPlayer;
};

typedef IPlayerFactory* (*FN_GetPlayerFactory)();//������ȡ��������������ָ��

std::list<Factory_Info *> s_FactoryList;//�����������Ĺ������б�
std::list<Player_Info *> s_PlayerList;//�������б�

std::recursive_mutex s_mutexFactorys;
std::recursive_mutex s_mutexPlayers;

static int nPlayerIndex = 0;

Player_Info * FindPlayer(LONG pID)//���Ҷ�Ӧ������
{
	std::list<Player_Info*>::iterator iter = s_PlayerList.begin();
	while (iter != s_PlayerList.end())
	{
		Player_Info *pPlayerInfo = (Player_Info*)*iter;
		if (pID == pPlayerInfo->pid)
		{
			return pPlayerInfo;
		}
		iter++;
	}
	return NULL;
}

Factory_Info * FindFactory(LONG fID)//���Ҷ�Ӧ����������
{
	std::list<Factory_Info *>::iterator iter = s_FactoryList.begin();
	while (iter != s_FactoryList.end())
	{
		Factory_Info *pFactoryInfo = (Factory_Info*)*iter;

		if (fID == pFactoryInfo->fid)
		{
			return pFactoryInfo;
		}
		iter++;
	}
	return NULL;
}

BOOL GetCreatePlayerAPI(TCHAR *pAPIFile)
{
	HMODULE hDllLib = ::LoadLibrary(pAPIFile);

     if (hDllLib)
    {
            //��ȡ��̬���ӿ���ĺ�����ַ��
		FN_GetPlayerFactory fpFun = (FN_GetPlayerFactory)GetProcAddress(hDllLib, "GetPlayerFactory");
		if (fpFun)//��ȡ�������ɹ�
		{
			IPlayerFactory *pFactory = fpFun();
			if (pFactory&&pFactory->Init())//�����ɹ����ҳ�ʼ���ɹ�
			{
				Factory_Info *pFactoryInfo = new Factory_Info();

				pFactoryInfo->fid = s_FactoryList.size() + 1;
				pFactoryInfo->pFactory = pFactory;
				s_FactoryList.push_back(pFactoryInfo);
			}
		

		}
		return true;
	}
	  return false;
}

void FindFactoryAPI(TCHAR *cAPIPath)
{
	USES_CONVERSION;
	//�򿪲���playAPI��̬��
	WIN32_FIND_DATA findFileData;
	//HANDLE	hFindFile = ::FindFirstFile(cAPIPath, &findFileData);
	TCHAR cAPIPathFile[1024];
	memset(cAPIPathFile, 0, 1024);
	swprintf_s(cAPIPathFile, 1024, A2T("%s\\*.dll"), cAPIPath);
	HANDLE	hFindFile = ::FindFirstFile(cAPIPathFile, &findFileData);
	if (INVALID_HANDLE_VALUE == hFindFile)
	{
		return;
	}
	do
	{
		if (strcmp(T2A(findFileData.cFileName), ".") == 0 || strcmp(T2A(findFileData.cFileName), "..") == 0)
		{
			continue;
		}

		if (strstr(T2A(findFileData.cFileName), "PlayerAPI"))//�Ƚ��Ƿ���PlayAPI�Ķ�̬��
		{
			size_t size;
			char *pEnvBuf;
			_dupenv_s(&pEnvBuf, &size, "path");//��ȡϵͳ��������

			char env[2048];
			sprintf_s(env, "%s%s;%s", "path=", T2A(cAPIPath), pEnvBuf);
			_putenv(env);//���û�������

			TCHAR cAPIFile[1024];
			swprintf_s(cAPIFile, 1024, A2T("%s\\%s"), cAPIPath, findFileData.cFileName);
			GetCreatePlayerAPI(cAPIFile);
			break;
		}

	} while (FindNextFile(hFindFile, &findFileData));
	FindClose(hFindFile);
}

BOOL InitPlayerFactory()
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexFactorys);

	//��ȡ����·��
	TCHAR pathBuf[MAX_PATH];
//	::GetCurrentDirectory(MAX_PATH, pathBuf);
	GetModuleFileName(NULL, pathBuf, 256);
	TCHAR *cp = _tcsrchr(pathBuf, _T('\\'));
	if (cp != NULL)
		*(cp ) = _T('\0');
//	TCHAR pathNameBuf[MAX_PATH];
//	strncpy()

USES_CONVERSION;
	TCHAR cFactoryRootPath[1024];
	memset(cFactoryRootPath, 0, 1024);
	swprintf_s(cFactoryRootPath, 1024, A2T("%s\\%s"), pathBuf, A2T("factorys"));
	
	TCHAR cFactoryRootPathFile[1024];
	memset(cFactoryRootPathFile, 0, 1024);
	swprintf_s(cFactoryRootPathFile, 1024, A2T("%s\\*.*"), cFactoryRootPath);

	WIN32_FIND_DATA findPathData;
	HANDLE	hFindPath = ::FindFirstFile(cFactoryRootPathFile, &findPathData);

	if (INVALID_HANDLE_VALUE == hFindPath)
	{
		return false;
	}
	do
	{
		if (strcmp(T2A(findPathData.cFileName), ".") == 0 || strcmp(T2A(findPathData.cFileName), "..") == 0)
		{
			continue;
		}

		if (findPathData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)//�ļ�������
		{			
			TCHAR cAPIPath[1024];
			swprintf_s(cAPIPath, 1024, A2T("%s\\%s"), cFactoryRootPath, findPathData.cFileName);
			FindFactoryAPI(cAPIPath);
		}

	} while (::FindNextFile(hFindPath, &findPathData));

	FindClose(hFindPath);
		
	return true;
}

BOOL ReleasePlayerFactory()
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexFactorys);
	std::list<Factory_Info *>::iterator iter = s_FactoryList.begin();
	while (iter != s_FactoryList.end())
	{
		Factory_Info *pFactoryInfo = (Factory_Info*)*iter;
		pFactoryInfo->pFactory->Release();
		iter++;
	}
	return true;
}

BOOL GetPlayerInterface(LONG *nID)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	nPlayerIndex++;//����������Ҫ��ֹ�ظ�����
	std::list<Player_Info*>::iterator iter = s_PlayerList.begin();
	while (iter != s_PlayerList.end())
	{
		Player_Info *pPlayerInfo = (Player_Info*)*iter;
		if (nPlayerIndex == pPlayerInfo->pid)
		{
			return GetPlayerInterface(nID);
		}
		iter++;
	}
	*nID = nPlayerIndex;
	return true;
}

BOOL FreePlayerInterface(LONG nID)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	std::list<Player_Info*>::iterator iter = s_PlayerList.begin();
	while (iter != s_PlayerList.end())
	{
		Player_Info *pPlayerInfo = (Player_Info*)*iter;
		if (nID == pPlayerInfo->pid)
		{		
			delete pPlayerInfo->pPlayer;
			delete pPlayerInfo;
			s_PlayerList.erase(iter);
			return true;
		}
		iter++;
	}
	return false;
}

IPlayer *CreatePlayer(LONG nID, Factory_Info *pFactoryInfo)
{
	IPlayer *pPlayer = NULL;
	if (pFactoryInfo)
	{
		pPlayer = pFactoryInfo->pFactory->CreatePlayer();
		if (pPlayer)
		{
			Player_Info *pPlayer_Info = new Player_Info;//����һ��������
			pPlayer_Info->pid = nID;
			pPlayer_Info->pPlayer = pPlayer;
			s_PlayerList.push_back(pPlayer_Info);//���벥�����б���PIDΪΨһ��ʶ�����š���ͣ�������������PID���Ҷ�Ӧ�Ĳ�����
		}
	}
	return pPlayer;
}

BOOL Player_OpenFile(LONG nID, char *pFile, HWND hwnd, int iFileType)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexFactorys);
	Factory_Info *pFactoryInfo = FindFactory(iFileType);
	IPlayer *pPlayer = CreatePlayer(nID, pFactoryInfo);
	if (pPlayer)
	{	
		return pPlayer->OpenFile(pFile,hwnd);
	}

	return false;
}

BOOL Player_Play(LONG nID)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->Play();
	}
	return false;
}

BOOL Player_Pause(LONG nID, DWORD nPause)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->Pause(nPause);
	}
	return false;
}

BOOL Player_Stop(LONG nID)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->Stop();
	}
	return false;
}

BOOL Player_Fast(LONG nID)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->Fast();
	}
	return false;
}

BOOL Player_FastNoDelay(LONG nID)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->FastNoDelay();
	}
	return false;
}

BOOL Player_OneByOne(LONG nID)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->OneByOne();
	}
	return false;
}

BOOL Player_OneByOneBack(LONG nID)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->OneByOneBack();
	}
	return false;
}

BOOL Player_PlaySoundShare(LONG nID)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->PlaySoundShare();
	}
	return false;
}

BOOL Player_StopSoundShare(LONG nID)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->StopSoundShare();
	}
	return false;
}

BOOL Player_SetVolume(LONG nID, DWORD nVolume)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->SetVolume(nVolume);
	}
	return false;
}

DWORD Player_GetVolume(LONG nID)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->GetVolume();
	}
	return 0;
}

BOOL Player_Slow(LONG nID)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->Slow();
	}
	return false;
}

BOOL   Player_SetPlayPos(LONG nID, float fRelativePos)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->SetPlayPos(fRelativePos);
	}
	return false;
}

float  Player_GetPlayPos(LONG nID)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->GetPlayPos();
	}
	return 0;
}
DWORD  Player_GetFileTime(LONG nID)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->GetFileTime();
	}
	return 0;
}

DWORD  Player_GetPlayedTime(LONG nID)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->GetPlayedTime();
	}
	return 0;
}

BOOL  Player_SetPlayedTimeEx(LONG nID, DWORD nTime)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->SetPlayedTimeEx(nTime);
	}
	return 0;
}

BOOL  Player_GetPictureSize(LONG nID, LONG *pWidth, LONG *pHeight)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->GetPictureSize(pWidth, pHeight);
	}
	return false;
}

BOOL Player_CapturePic(LONG nID, char *pSaveFile, int iType)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->CapturePic(pSaveFile, iType);
	}
	return false;
}

BOOL Player_SetColor(LONG nID, DWORD nRegionNum, int nBrightness, int nContrast, int nSaturation, int nHue)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->SetColor(nRegionNum, nBrightness, nContrast, nSaturation, nHue);
	}
	return false;
}

BOOL Player_GetColor(LONG nID, DWORD nRegionNum, int *pBrightness, int *pContrast, int *pSaturation, int *pHue)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->GetColor(nRegionNum, pBrightness, pContrast, pSaturation, pHue);
	}
	return false;
}

BOOL Player_SetFileEndCallback(LONG nID, void(CALLBACK*FileEndCallback)(LONG nID, void *pUser), void *pUser)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->SetFileEndCallback(nID,FileEndCallback, pUser);
	}
	return false;
}

BOOL Player_SetDisPlayCallback(LONG nID, DisplayCallback displayCallback, void *pUser)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->SetDisplayCallback(nID,displayCallback, pUser);
	}
	return false;
}

BOOL Player_GetSystemTime(LONG nID, unsigned long long *systemTime)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->GetSystemTime(systemTime);
	}
	return false;
}

BOOL  Player_FileCutStart(LONG nID, const char* srcFileName, const char* destFileName, unsigned __int64 startTime, unsigned __int64 endTime, BOOL bFast)
{
	int iFileType;
	AnalyzeFileType((char*)srcFileName, &iFileType);

	std::lock_guard<std::recursive_mutex> lock(s_mutexFactorys);
	Factory_Info *pFactoryInfo = FindFactory(iFileType);
	IPlayer *pPlayer = CreatePlayer(nID, pFactoryInfo);
	if (pPlayer)
	{
		return pPlayer->FileCutStart(srcFileName, destFileName, startTime, endTime);
	}
			
	return false;
}

BOOL  Player_FileCutClose(LONG nID)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->FileCutClose();
	}
	return false;
}

int  Player_FileCutProcess(LONG nID)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->FileCutProcess();
	}
	return 0;
}
/*
BOOL IsSupportFastCut(char *pFileName)//�ж��ļ������Ƿ�֧�ֿ��ټ���
{
	int iFileType;
	AnalyzeFileType(pFileName, &iFileType);
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Factory_Info *pFactoryInfo = FindFactory(iFileType);
	if (pFactoryInfo)
	{
		return pFactoryInfo->pFactory->IsSupportFastCut();
	}

	return false;
}
*/
BOOL IsSupport22Renader(int iFileType)//�ж��ļ������Ƿ�֧�ֿ��ټ���
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Factory_Info *pFactoryInfo = FindFactory(iFileType);
	if (pFactoryInfo)
	{
		return pFactoryInfo->pFactory->IsSupport22Renader();
	}

	return false;
}

BOOL Player_FileConvertStart(LONG nID, const char* srcFileName, const char* destFileName, unsigned __int64 startTime, unsigned __int64 endTime, bool bConvert, T_ConverterParameters *pConvertPara)
{
	int fID = FindFFmpegFactoryID();
	if (fID==-1)
	{
		return false;
	}
/*	int fID;
	AnalyzeFileType((char*)srcFileName, &fID);*/

	std::lock_guard<std::recursive_mutex> lock(s_mutexFactorys);
	Factory_Info *pFactoryInfo = FindFactory(fID);
	IPlayer *pPlayer = CreatePlayer(nID, pFactoryInfo);

	if (pPlayer)
	{
		return pPlayer->FileConvertStart(srcFileName, destFileName, startTime, endTime, bConvert, pConvertPara);
	}
	return 0;
}

BOOL Player_FileConvertClose(LONG nID)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->FileConvertClose();
	}
	return 0;
}

int Player_FileConvertProcess(LONG nID)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->FileConvertProcess();
	}
	return 0;
}

BOOL Player_FileConvertPause(LONG nID, DWORD nPause)
{
	std::lock_guard<std::recursive_mutex> lock(s_mutexPlayers);
	Player_Info *pPlayerInfo = FindPlayer(nID);
	if (pPlayerInfo)
	{
		return pPlayerInfo->pPlayer->FileConvertPause(nPause);
	}
	return 0;
}

int  FindFFmpegFactoryID()
{
	std::list<Factory_Info *>::iterator iter = s_FactoryList.begin();
	while (iter != s_FactoryList.end())
	{
		Factory_Info *pFactoryInfo = (Factory_Info*)*iter;

		if (strstr(pFactoryInfo->pFactory->Name(), "StandardFile"))
		{
			return pFactoryInfo->fid;
		}
		iter++;
	}
	return -1;
}

BOOL AnalyzeFileType(char *pFileName, int *pfileType)
{
	*pfileType = 0;//����ͨ����
	int sStandFileFID = -1;//ͨ�ñ�׼�ļ��Ĺ���ID
	std::list<Factory_Info *>::iterator iter = s_FactoryList.begin();
	while (iter != s_FactoryList.end())
	{
		Factory_Info *pFactoryInfo = (Factory_Info*)*iter;

		if (strstr(pFactoryInfo->pFactory->Name(), "StandardFile"))
		{
			sStandFileFID = pFactoryInfo->fid;
		}
		else if (pFactoryInfo->pFactory->IsBelongThis(pFileName))
		{
			*pfileType=pFactoryInfo->fid;
			return true;
		}
		iter++;
	}
	if (*pfileType == 0)//˵��û���ҵ�ƥ�����ͣ�����Ϊ��׼
	{
		*pfileType = sStandFileFID;
	}
	return false;
}
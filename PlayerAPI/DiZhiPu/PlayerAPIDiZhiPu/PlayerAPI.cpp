// PlayerAPI.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"

#include "PlayerAPI.h"

#include "PlayerDiZhiPu.h"

IPlayerFactory * GetPlayerFactory()
{
	CPlayerFactoryDiZhiPu *pPlayerFacory = new CPlayerFactoryDiZhiPu();
	return pPlayerFacory;
}
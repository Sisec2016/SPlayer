// PlayerAPI.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"

#include "PlayerAPI.h"

#include "PlayerDL.h"

IPlayerFactory * GetPlayerFactory()
{
	CPlayerFactoryDL *pPlayerFacory = new CPlayerFactoryDL();
	return pPlayerFacory;
}
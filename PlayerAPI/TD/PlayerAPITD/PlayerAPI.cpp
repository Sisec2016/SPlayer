// PlayerAPI.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"

#include "PlayerAPI.h"

#include "PlayerTD.h"

IPlayerFactory * GetPlayerFactory()
{
	CPlayerFactoryTD *pPlayerFacory = new CPlayerFactoryTD();
	return pPlayerFacory;
}
// PlayerAPI.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"

#include "PlayerAPI.h"

#include "PlayerHB.h"

IPlayerFactory * GetPlayerFactory()
{
	CPlayerFactoryHB *pPlayerFacory = new CPlayerFactoryHB();
	return pPlayerFacory;
}

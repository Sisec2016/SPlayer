// PlayerAPI.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"

#include "PlayerAPI.h"

#include "PlayerHie.h"

IPlayerFactory * GetPlayerFactory()
{
	CPlayerFactoryHie *pPlayerFacory = new CPlayerFactoryHie();
	return pPlayerFacory;
}
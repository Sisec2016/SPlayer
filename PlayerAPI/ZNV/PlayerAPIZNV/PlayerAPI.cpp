// PlayerAPI.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"

#include "PlayerAPI.h"

#include "PlayerZNV.h"

IPlayerFactory * GetPlayerFactory()
{
	IPlayerFactory *pPlayerFacory = new CPlayerFactoryZNV();
	return pPlayerFacory;
}

// PlayerAPI.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"

#include "PlayerAPI.h"

#include "PlayerJVS.h"

IPlayerFactory * GetPlayerFactory()
{
	IPlayerFactory *pPlayerFacory = new CPlayerFactoryZhongWei();
	return pPlayerFacory;
}

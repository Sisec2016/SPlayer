// PlayerAPI.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"

#include "PlayerAPI.h"

#include "PlayerJunMingShi.h"

IPlayerFactory * GetPlayerFactory()
{
	IPlayerFactory *pPlayerFacory = new CPlayerFactoryJunMingShi();
	return pPlayerFacory;
}


// PlayerAPI.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"

#include "PlayerAPI.h"

#include "PlayerDH.h"

IPlayerFactory * GetPlayerFactory()
{
	IPlayerFactory *pPlayerFactory = new CPlayerFactoryDH();
	return pPlayerFactory;
}

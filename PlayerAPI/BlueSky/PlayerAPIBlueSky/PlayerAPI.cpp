// PlayerAPI.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"

#include "PlayerAPI.h"

#include "PlayerBlueSky.h"

IPlayerFactory * GetPlayerFactory()
{
	IPlayerFactory *pPlayerFactory = new CPlayerFactoryBlueSky();
	return pPlayerFactory;
}

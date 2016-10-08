#ifndef _NATTYPES_H
#define _NATTYPES_H


#define IN
#define OUT
#define INOUT

#define DEV_ID_LEN		64


/*NAT���ͽṹ*/
typedef enum
{
	NAT_TYPE_UNKONWN = 0,
    NAT_TYPE_FULL_CONE = 1,   //��ȫԲ׶��.һ����ȫԲ׶���NAT
    NAT_TYPE_RESTRICTED_CONE, //�����Ƶ�Բ׶��.
    NAT_TYPE_PORT_RESTRICTED_CONE, //�˿������Ƶ�Բ׶��NAT.
    NAT_TYPE_SYMMETRIC_CONE,  //�ԳƵ�NAT
	NAT_TYPE_HOST             // ����Ҫnat

}NATTYPE;


/*�豸����*/
typedef enum
{
	EDEVICE_TYPE_NVR = 1,   
	EDEVICE_TYPE_DVR,
	EDEVICE_TYPE_IPC, 
	EDEVICE_TYPE_Bplayer,
	EDEVICE_TYPE_SDK

}DEVTYPE;

/*NAT���ӵ�����*/
typedef enum
{
	NAT_CON_TURN_UDP = 1, /*Turn ����*/
	NAT_CON_TURN_TCP,
	NAT_CON_STUN_UDP,      /*Stun����*/
	NAT_CON_STUN_TCP,
	NAT_CON_TURN_REAL_TCP
}NATCONTYPE;


//��ȡ�����豸��Ϣ
typedef struct 
{
	char devId[DEV_ID_LEN];		/*�豸Ψһ��*/
	int lastHeartTime;	/*�������ʱ��*/
	NATTYPE natType;   /*�豸nat����*/
}DevInfo;

/*���������ص������豸*/
typedef struct 
{
	int count;
	DevInfo devInfo[0];
}DevListResponse;


typedef struct _NatSockaddr_in_
{
	short family;          /*AF_INET����ַ�壩PF_INET��Э���壩*/
	unsigned short port;   /*�˿ں�,��Ҫ��htons()����ת�����������ݸ�ʽ������)*/
	unsigned int naddr;           /* ���������ֽ�˳��洢IP��ַ */
	unsigned char sin_zero[8]; /* ��չ��Ԥ�� */
}NatSockaddr_in;


/*
 ֱ�����ÿͻ������ݽ��յĻص�����
*/
typedef int (*pfnNatClientRecvCallback)(int natSessionId, void* data, int len, void* context);


#endif
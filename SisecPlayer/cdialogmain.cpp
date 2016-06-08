#include "cdialogmain.h"
#include "qfiledialog.h"
#include "qmessagebox.h"

#include <QDomComment>
#include <QTextStream>
#include <qmenu.h>
#include <qdatetime.h>

#include <qapplication.h>
#include <qdesktopwidget.h>
#include <QThread>

#include "cdialogconfig.h"
#include "cdialogconverter.h"
#include "cdialogcut.h"
#include "cdialoghelp.h"
#include "cupdatedlg.h"
#include "cdialogmessagebox.h"

#include "public_def.h"

//#include "ukey/Sage_USB_API.h"
//#pragma comment(lib,"ukey/Sage_USB_API.lib")
#include "ukey/Verifier.h"
#pragma comment(lib,"ukey/Verifier.lib")

#define FILEPLAYLISTXML "playlist.xml"
#define ROOTTEXT  "playFileList"
#define NOTETEXT "fileName"
#define DX 180

UkeyThread::UkeyThread():m_stopFlag(false)
{

}

UkeyThread::~UkeyThread()
{

}

bool UkeyThread::VerifyUkey()
{	
	return IsPresent();
}

void UkeyThread::run()
{
	while (!m_stopFlag)
	{
		QThread::msleep(3000);//ÿ3����һ��
		if (!VerifyUkey())
		{
			emit ukeyDown();			
		}
		else
		{
			
			emit ukeyUp();
		}
	}
}

void UkeyThread::Stop()
{
	m_stopFlag = true;
}

UkeyThread CDialogMain::s_ukeyThread;

CDialogMain::CDialogMain(QWidget *parent)
: NoFlameDlg(parent), m_pConvertDlg(NULL), m_pCutDlg(NULL)
{
	ui.setupUi(this);

	InitPlayerFactory();

	for (int i = 0; i< NUM; i++)
	{
		m_PlayWnd[i] = new CWidgetPlayWnd(i,this);
		m_PlayCtrl[i] = new CFormPlayCtrl(i, m_PlayWnd[i], this);
	}

	m_AudioDubCtrl = new CFormPlayCtrl(NUM, NULL,this);
	m_AudioDubCtrl->SetAudioAttrible();
	m_AudioDubCtrl->hide();
	m_PlayCtrl[0]->SetAudioCtrl(m_AudioDubCtrl);

	m_bHide = false;
	m_iScreenWidth = 0;
	m_iScreenHeigth = 0;
	m_bFullScreen = false;

	SetPlayMode(SINGLE_MODE);
	SetPlayCtrlPos();
	SetBtnHideListPos();
	ReadPlayFileList();//��ȡ�����б�
	CDialogConfig::ReadConfigXml();//��������Ϣ��������ͼ·����

	IniSysMenu();//��ʼ���в˵�
	IniWndMenu();
	IniPlayListMenu();
	
	ui.BtnFullScreen->hide();
	ui.frame_toolBar->raise();
	ui.frameBackGround->raise();
	ui.frameComboBox->raise();
	ui.BtnHideList->raise();

	
	connect(&s_ukeyThread, SIGNAL(ukeyDown()), this, SLOT(OnUkeyDown()));
	connect(&s_ukeyThread, SIGNAL(ukeyUp()), this, SLOT(OnUkeyUp()));
	connect(&m_ukeyDownMsgdlg, SIGNAL(accepted()), this, SLOT(OnTerminated()));
	connect(m_configAct, SIGNAL(triggered()), this, SLOT(OnConfigTriggered()));
	connect(m_converterAct, SIGNAL(triggered()), this, SLOT(OnConvertTriggered()));
	connect(m_cutAct, SIGNAL(triggered()), this, SLOT(OnCutTriggered()));
	connect(m_updateAct, SIGNAL(triggered()), this, SLOT(OnUpdateTriggered()));
	connect(m_helpAct, SIGNAL(triggered()), this, SLOT(OnHelpTriggered()));
	connect(m_WndActGroup, SIGNAL(triggered(QAction*)), this, SLOT(OnWndModeTriggered(QAction*)));
	connect(m_renderActGroup, SIGNAL(triggered(QAction*)), this, SLOT(OnRenderModeTriggered(QAction*)));
	connect(m_ActGroup, SIGNAL(triggered(QAction*)), this, SLOT(OnPlayModeTriggered(QAction*)));
	connect(ui.BtnMore, SIGNAL(clicked()), this, SLOT(OnBtnMoreClick()));
	connect(ui.BtnFullScreen, SIGNAL(clicked()), this, SLOT(OnBtnFullScreenClick()));
	connect(ui.BtnClose, SIGNAL(clicked()), this, SLOT(OnBtnCloseClick()));
	connect(ui.BtnMax, SIGNAL(clicked()), this, SLOT(OnBtnMaxClick()));
	connect(ui.BtnMin, SIGNAL(clicked()), this, SLOT(OnBtnMinClick()));
//	connect(ui.BtnSingle, SIGNAL(clicked()), this, SLOT(OnBtnSingleClick()));
	connect(ui.BtnDiv4Time, SIGNAL(clicked()), this, SLOT(OnBtnDiv4TimeClick()));
	connect(ui.BtnFileAdd, SIGNAL(clicked()), this, SLOT(OnBtnFileAddClick()));
	connect(ui.comboBox, SIGNAL(activated(int)), this, SLOT(OnActivated(int))); 
	connect(ui.BtnFileOpen, SIGNAL(clicked()), this, SLOT(OnBtnFileOpenClick()));
	connect(ui.BtnFileDel, SIGNAL(clicked()), this, SLOT(OnBtnFileDelClick()));
	connect(ui.BtnFileDelAll, SIGNAL(clicked()), this, SLOT(OnBtnFileDelAllClick()));
	connect(ui.BtnHideList, SIGNAL(clicked()), this, SLOT(OnBtnHideListClick()));
	connect(ui.listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(OnListWidgetItemDbClick(QListWidgetItem*)));
	//connect(ui.listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(OnListWidgetItemDbClick(QListWidgetItem*)));
	
	installEventFilter(this);

	ui.BtnWndMode->installEventFilter(this);
	ui.BtnDiv4Time->installEventFilter(this);

	ui.BtnHideList->installEventFilter(this);

	ui.BtnMore->installEventFilter(this);
	ui.BtnMin->installEventFilter(this);
	ui.BtnMax->installEventFilter(this);

	setAcceptDrops(true);

	OnBtnHideListClick();//Ĭ�����ز����б�
#ifdef UKey
	s_ukeyThread.start();
#endif
}

CDialogMain::~CDialogMain()
{
	for (int i = 0; i< NUM; i++)
	{
		delete m_PlayCtrl[i];
		delete m_PlayWnd[i];
	}

}

//����ʱ���
bool CDialogMain::VerifyUkey()
{
	//if (!s_ukeyThread.VerifyUkey())
	int threadID = (int)QThread::currentThreadId();
	if (!Verifiy(threadID))
	{
		QMessageBox::information(NULL, QStringLiteral("��ʾ"), QStringLiteral("δ��⵽��Ч��UKey,�޷����У�"));
		return false;
	}
	return true;
}

void CDialogMain::OnTerminated()
{
	OnBtnCloseClick();
	exit(0);//�˳�Ӧ�ó���
}

void CDialogMain::OnUkeyDown()
{
	if (this->isEnabled())//
	{
		for (int i = 0; i < NUM; i++)
		{
			if (m_PlayCtrl[i]->IsPlaying())
			{
				m_PlayCtrl[i]->Pause(1);
			}
		}

		if (m_pConvertDlg)
		{
			((CDialogConverter*)m_pConvertDlg)->UkeyDownStop();
		}
		if (m_pCutDlg )
		{
			((CDialogCut*)m_pCutDlg)->UkeyDownStop();
		}

		this->setEnabled(false);
		//m_ukeyDownMsgdlg.setWindowFlags(Qt::FramelessWindowHint);
		m_ukeyDownMsgdlg.setWindowTitle(QStringLiteral("��ʾ"));
		m_ukeyDownMsgdlg.setText(QStringLiteral("δ��⵽��Ч��UKey,�رմ˶Ի�����˳�Ӧ�ó���"));

	//	m_ukeyDownMsgdlg.addButton(QStringLiteral("�˳�Ӧ�ó���"), QMessageBox::AcceptRole);//QMessageBox::YesRole
	//	int flag = m_ukeyDownMsgdlg.exec();//flag����0����QMessageBox::NoButton��ʲôԭ��
	//	if (QMessageBox::NoButton == m_ukeyDownMsgdlg.exec())
		if (QMessageBox::Ok == m_ukeyDownMsgdlg.exec())
	//	if (QMessageBox::Accepted == m_ukeyDownMsgdlg.exec())//���У���ӵİ�ť����
		{
			emit m_ukeyDownMsgdlg.accepted();//���ֶ��ӷ����źţ�Ҫ��connect(&m_ukeyDownMsgdlg, SIGNAL(accepted()), this, SLOT(OnBtnCloseClick()))��Ч
		}
	}

}

void CDialogMain::OnUkeyUp()
{
	if (!this->isEnabled())
	{	
		m_ukeyDownMsgdlg.reject();
		this->setEnabled(true);
		for (int i = 0; i < NUM; i++)
		{
			if (m_PlayCtrl[i]->IsPlaying())
			{
				m_PlayCtrl[i]->Pause(0);
			}
		}
	}
}

void  CDialogMain::IniSysMenu()
{
	QMenu *pMenu = new QMenu(QStringLiteral("menu"), ui.BtnMore);
	m_renderIniAct = new QAction(QStringLiteral("ԭʼ����"), this);
	m_rendeStrechAct = new QAction(QStringLiteral("��������"), this);
	m_render4_3Act = new QAction(QStringLiteral("4:3"), this);
	m_rendeStrech16_9Act = new QAction(QStringLiteral("16:9"), this);
	m_configAct = new QAction(QIcon(":/Skin/configure.png"), QStringLiteral("����"), this);
	m_converterAct = new QAction( QStringLiteral("ת��"), this);
	m_cutAct = new QAction( QStringLiteral("����"), this);
	m_updateAct = new QAction(QIcon(":/Skin/Update.png"), QStringLiteral("����"), this);
	m_helpAct = new QAction(QIcon(":/Skin/Help.png"), QStringLiteral("����"), this);
	
	//QMenu *pDiaplayMenu = pMenu->addMenu(QIcon(":/Skin/configure.png"), QStringLiteral("��ʾ����"));
	QMenu *pDiaplayMenu = pMenu->addMenu( QStringLiteral("��ʾ����"));
	pDiaplayMenu->addAction(m_renderIniAct);
	pDiaplayMenu->addAction(m_rendeStrechAct);
	pDiaplayMenu->addAction(m_render4_3Act);
	pDiaplayMenu->addAction(m_rendeStrech16_9Act);
	m_renderIniAct->setCheckable(true);
	m_rendeStrechAct->setCheckable(true);
	m_render4_3Act->setCheckable(true);
	m_rendeStrech16_9Act->setCheckable(true);
	m_renderIniAct->setChecked(true);
	m_renderMode = RenderIni;
	m_renderActGroup = new QActionGroup(this);
	m_renderActGroup->addAction(m_renderIniAct);
	m_renderActGroup->addAction(m_rendeStrechAct);
	m_renderActGroup->addAction(m_render4_3Act);
	m_renderActGroup->addAction(m_rendeStrech16_9Act);

	QMenu *pConvertMenu = pMenu->addMenu(QStringLiteral("ת��/����"));
	pConvertMenu->addAction(m_converterAct);
	pConvertMenu->addAction(m_cutAct);

	pMenu->addAction(m_configAct);
	pMenu->addAction(m_updateAct);
	pMenu->addAction(m_helpAct);
	ui.BtnMore->setMenu(pMenu);
}

void  CDialogMain::IniWndMenu()
{
	QMenu *pWndMenu = new QMenu(QStringLiteral("WndModeMenu"), ui.BtnPlayList);
/*	m_Wnd1Act = new QAction(QIcon(":/Skin/single.png"), QStringLiteral("1"), this);
	m_Wnd4Act = new QAction(QIcon(":/Skin/grid4.png"), QStringLiteral("4"), this);
	m_Wnd9Act = new QAction(QIcon(":/Skin/grid9.png"), QStringLiteral("9"), this);
	m_Wnd16Act = new QAction(QIcon(":/Skin/grid16.png"), QStringLiteral("16"), this);*/

	m_Wnd1Act = new QAction( QStringLiteral("1"), this);
	m_Wnd4Act = new QAction(QStringLiteral("4"), this);
	m_Wnd9Act = new QAction(QStringLiteral("9"), this);
	m_Wnd16Act = new QAction(QStringLiteral("16"), this);
	m_Wnd1Act->setCheckable(true);
	m_Wnd4Act->setCheckable(true);
	m_Wnd9Act->setCheckable(true);
	m_Wnd16Act->setCheckable(true);

	pWndMenu->addAction(m_Wnd1Act);
	m_Wnd1Act->setChecked(true);
	m_WndMode = WND1;
	pWndMenu->addAction(m_Wnd4Act);
	pWndMenu->addAction(m_Wnd9Act);
	pWndMenu->addAction(m_Wnd16Act);
	ui.BtnWndMode->setMenu(pWndMenu);

	m_WndActGroup = new QActionGroup(this);
	m_WndActGroup->addAction(m_Wnd1Act);
	m_WndActGroup->addAction(m_Wnd4Act);
	m_WndActGroup->addAction(m_Wnd9Act);
	m_WndActGroup->addAction(m_Wnd16Act);
}

void  CDialogMain::IniPlayListMenu()
{
	QMenu *pPlayListMenu = new QMenu(QStringLiteral("PlayModeMenu"), ui.BtnPlayList);
	m_singlePlayAct = new QAction(QStringLiteral("��������"), this);
	m_listPlayAct = new QAction(QStringLiteral("˳�򲥷�"), this);
	m_singleRepeatAct = new QAction(QStringLiteral("����ѭ��"), this);
	m_listRepeatAct = new QAction(QStringLiteral("˳��ѭ��"), this);
	m_singlePlayAct->setCheckable(true);
	m_listPlayAct->setCheckable(true);
	m_singleRepeatAct->setCheckable(true);
	m_listRepeatAct->setCheckable(true);

	pPlayListMenu->addAction(m_listPlayAct);
	m_listPlayAct->setChecked(true);
	m_playListMode = LIST;
	pPlayListMenu->addAction(m_singlePlayAct);
	pPlayListMenu->addAction(m_singleRepeatAct);
	pPlayListMenu->addAction(m_listRepeatAct);
	ui.BtnPlayList->setMenu(pPlayListMenu);

	m_ActGroup = new QActionGroup(this);
	m_ActGroup->addAction(m_singlePlayAct);
	m_ActGroup->addAction(m_listPlayAct);
	m_ActGroup->addAction(m_singleRepeatAct);
	m_ActGroup->addAction(m_listRepeatAct);
}

void  CDialogMain::OnConfigTriggered()
{
	CDialogConfig dlg(this);
	dlg.exec();
}

void  CDialogMain::OnConvertTriggered()
{
	CDialogConverter dlg(this);
	int iScreenWidth = QApplication::desktop()->width();
	int iScreenHeigth = QApplication::desktop()->height();
	QRect rc = dlg.geometry();
	dlg.move((iScreenWidth - rc.width()) / 2, (iScreenHeigth - rc.height()) / 2);
	m_pConvertDlg = &dlg;
	dlg.exec();
	m_pConvertDlg = NULL;
}

void  CDialogMain::OnCutTriggered()
{
	CDialogCut dlg(this);
	int iScreenWidth = QApplication::desktop()->width();
	int iScreenHeigth = QApplication::desktop()->height();
	QRect rc = dlg.geometry();
	dlg.move((iScreenWidth - rc.width()) / 2, (iScreenHeigth - rc.height()) / 2);
	m_pCutDlg = &dlg;
	dlg.exec();
	m_pCutDlg = NULL;
}


void  CDialogMain::OnUpdateTriggered()
{
	CUpdateDlg dlg(this);
	dlg.CheckNewVersion();
	dlg.exec();
}


void  CDialogMain::OnHelpTriggered()
{
	CDialogHelp dlg(this);
	dlg.exec();
}

 void  CDialogMain::OnWndModeTriggered(QAction*act)
{
	if (m_PlayMode == TIME_MODE)
	{
		bool bPlaying = false;
		for (int i = 0; i < m_curWndNum; i++)
		{
			if (m_PlayCtrl[i]->IsPlaying())
			{
				bPlaying = true;
			}
		}
		if (bPlaying)
		{
			if (QMessageBox::No == QMessageBox::question(this, QStringLiteral("��ʾ"),
				QStringLiteral("�Ƿ��˳���ǰ���ֶβ��š���"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No))
			{
				return;
			}
			StopAll();
		}
	}

	ui.BtnDiv4Time->setStyleSheet("QPushButton{border-image: url(:/Skin/divPlay.png)0 0 0 0 repeat}QPushButton:hover{border-image: url(:/Skin/divPlay.png)28 0 0 0 repeat}QPushButton:pressed{border-image: url(:/Skin/divPlay.png)56 0 0 0 repeat}QPushButton:disabled{border-image: url(: / Skin/divPlay.png)84 0 0 0 repeat}");
	act->setChecked(true);
	if (act == m_Wnd1Act)
	{
		m_WndMode = WND1;
	}
	else if (act == m_Wnd4Act)
	{
		m_WndMode = WND4;
	}
	else if (act == m_Wnd9Act)
	{
		m_WndMode = WND9;
	}
	else if (act == m_Wnd16Act)
	{
		m_WndMode = WND16;
	}

	SetPlayMode(SINGLE_MODE);	
}

void  CDialogMain::OnRenderModeTriggered(QAction*act)
{
	act->setChecked(true);
	if (act == m_renderIniAct)
	{
		m_renderMode = RenderIni;
	}
	else if (act == m_rendeStrechAct)
	{
		m_renderMode = RenderStrech;
	}
	else if (act == m_render4_3Act)
	{
		m_renderMode = Render4_3;
	}
	else if (act == m_rendeStrech16_9Act)
	{
		m_renderMode = Render16_9;
	}

	for (int i = 0; i < NUM; i++)
	{
		SetRenderMode(i);
	}
}

void  CDialogMain::OnPlayModeTriggered(QAction*act)
{
	act->setChecked(true);
	if (act == m_singlePlayAct)
	{
		m_playListMode = SINGLE;
	}
	else if(act == m_listPlayAct)
	{
		m_playListMode = LIST;
	}
	else if (act == m_singleRepeatAct)
	{
		m_playListMode = SINGLE_REPEAT;
	}
	else if (act == m_listRepeatAct)
	{
		m_playListMode = LIST_REPEAT;
	}
}

void CDialogMain::OnBtnMaxClick()
{
	m_bMax = !m_bMax;
	if (m_bMax)
	{
		//m_rcFrameSystemMenu=ui.frame_sysmenu->geometry();
		m_rcFrameSysBar = ui.frameSysBar->geometry();
		m_rcFrameFileList = ui.frame_fileList->geometry();
		m_rcFileList = ui.listWidget->geometry();
		m_rcFrameToolBar = ui.frame_toolBar->geometry();

		//this->showMaximized();
		this->showFullScreen();

		m_iScreenWidth = QApplication::desktop()->width();
		m_iScreenHeigth = QApplication::desktop()->height();

		ui.frameSysBar->setGeometry(m_rcFrameSysBar.left(), m_rcFrameSysBar.top(), m_iScreenWidth, m_rcFrameSysBar.height());
		ui.frame_toolBar->move(m_rcFrameToolBar.left(), m_iScreenHeigth - 50);
		
		ui.frame_fileList->setGeometry(m_iScreenWidth - 200, m_rcFrameFileList.top(), m_rcFrameFileList.width(), m_iScreenHeigth - 35);
	//�����frame_fileListλ��
		ui.listWidget->setGeometry(m_rcFileList.left(), m_rcFileList.top(), m_rcFileList.width(), m_iScreenHeigth - 65);
		//top ..left
		//ui.BtnMax->setStyleSheet("QPushButton{border-image: url(:/Skin/max.png)0 0 0 30 repeat}QPushButton:hover{border-image: url(:/Skin/max.png)30 0 0 30 repeat}QPushButton:pressed{border-image: url(:/Skin/max.png)60 0 0 60 repeat}QPushButton:disabled{border-image: url(:/Skin/max.png)90 0 0 90 repeat}");
		ui.BtnMax->setStyleSheet("QPushButton{border-image: url(:/Skin/max.png)0 0 0 30 repeat}QPushButton:hover{border-image: url(:/Skin/max.png)30 0 0 30 repeat}QPushButton:pressed{border-image: url(:/Skin/max.png)60 0 0 30 repeat}QPushButton:disabled{border-image: url(:/Skin/max.png)90 0 0 30 repeat}");
		ui.BtnMax->repaint();

	}
	else
	{
		this->showNormal();

		ui.frameSysBar->setGeometry(m_rcFrameSysBar);
		ui.frame_fileList->setGeometry(m_rcFrameFileList);
		ui.listWidget->setGeometry(m_rcFileList);
		ui.frame_toolBar->setGeometry(m_rcFrameToolBar);
		ui.BtnMax->setStyleSheet("QPushButton{border-image: url(:/Skin/max.png)0 0 0 0 repeat}QPushButton:hover{border-image: url(:/Skin/max.png)30 0 0 0 repeat}QPushButton:pressed{border-image: url(:/Skin/max.png)60 0 0 0 repeat}QPushButton:disabled{border-image: url(:/Skin/max.png)90 0 0 0 repeat}");
		ui.BtnMax->repaint();
		
	}

	SetPlayMode(m_PlayMode);	
	SetPlayCtrlPos();
	SetBtnHideListPos();
}

void CDialogMain::OnBtnMinClick()
{
	this->showMinimized();
}

void CDialogMain::OnBtnMoreClick()
{
	ui.BtnClose->showMenu();
}

void  CDialogMain::ReadPlayFileList()
{
	QString strPlayListXml=QApplication::applicationDirPath() + "/" + FILEPLAYLISTXML;
	QFile file(strPlayListXml);
	QDomDocument doc;
	if (!file.open(QIODevice::ReadOnly))
	{
		return;
	}

	if (!doc.setContent(&file))
	{
		file.close();
		return;
	}
	//���ļ��б�
	QDomElement root = doc.documentElement();
	if (root.isNull())
	{
		file.close();
		return;
	}

	QDomElement  note = root.firstChildElement();
	while (!note.isNull())
	{
		QString strPathName = note.text();
		int index=strPathName.lastIndexOf("/");
		QString strName = strPathName.mid(index+1);
		
		QListWidgetItem *item = new QListWidgetItem(strName);
		item->setToolTip(strPathName);
		ui.listWidget->addItem(item);

		m_strPlayFileList.append(strPathName);

		note = note.nextSiblingElement();
	}

	file.close();
	
}
void  CDialogMain::WritePlayFileList()
{
	//д�����б�
	QDomDocument doc;

	QDomNode instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
	doc.appendChild(instruction);

	QDomElement root = doc.createElement(ROOTTEXT);
	QDomText nodeText = doc.createTextNode("");
	root.appendChild(nodeText);
	doc.appendChild(root);

	int size = m_strPlayFileList.size();
	for (int i = 0; i < size; i++)
	{
		QDomElement note = doc.createElement(NOTETEXT);
		QDomText nodeText = doc.createTextNode(m_strPlayFileList.at(i));
		note.appendChild(nodeText);
		root.appendChild(note);
	}


	QString strPlayListXml = QApplication::applicationDirPath() + "/" + FILEPLAYLISTXML;
	QFile file(strPlayListXml);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
	{
		return;
	}

	QTextStream out(&file);
	out.setCodec("UTF-8");
	doc.save(out, 4, QDomNode::EncodingFromTextStream);

	file.close();
}

void  CDialogMain::OnBtnHideListClick()
{
	m_bHide = !m_bHide;

	if (m_bHide)
	{	
		ui.frame_fileList->hide();
		ui.BtnHideList->setToolTip(QStringLiteral("չ�������б�"));
	}
	else
	{
		ui.frame_fileList->show();
		ui.BtnHideList->setToolTip(QStringLiteral("���ز����б�"));
	}

	SetPlayMode(m_PlayMode);	
	SetPlayCtrlPos();
	SetBtnHideListPos();
}

void  CDialogMain::SetPlayMode(PLAY_MODE eMode)
{
	m_PlayMode = eMode;
	int left = 4;
	int top = 38;
	
	int iScreenWidth = QApplication::desktop()->width();
	int iScreenHeigth = QApplication::desktop()->height();		
	for (int i = 0; i < NUM; i++)
	{
		m_PlayWnd[i]->hide();
		m_PlayCtrl[i]->hide();
	}

	int twidth = 725;
	int theight = 552;
	if (m_bMax)
	{
		twidth = iScreenWidth - 210;
		theight = iScreenHeigth - 110;
	}
	if (m_bHide)
	{
		twidth += 200;
	}

	if (m_PlayMode == SINGLE_MODE)
	{	
		if (m_WndMode == WND1)
		{
			m_PlayWnd[0]->setGeometry(left, top, twidth, theight);
			m_PlayWnd[0]->show();
			SetRenderMode(0);
			ui.frameComboBox->move(left + twidth / 2 - 65, top + theight / 2 + 80);
			ui.frameBackGround->move(left + (twidth - 500) / 2, top + (theight - 500) / 2);
			if (!m_PlayCtrl[0]->IsPlaying())
			{
				ui.frameBackGround->show();
				ui.frameBackGround->raise();
				ui.frameComboBox->show();
				ui.frameComboBox->raise();
			}
			m_curWndNum = 1;
		}
		else if (m_WndMode == WND4)
		{
			SetWndGrid(top, left, twidth, theight, 4,2);
		}
		else if(m_WndMode == WND9)
		{
			SetWndGrid(top, left, twidth, theight, 9,3);

		}
		else if (m_WndMode == WND16)
		{
			SetWndGrid(top, left, twidth, theight, 16,4);
		}
		m_PlayCtrl[0]->show();
		m_iCurFocus = 0;
		repaint();
	}
	else if (m_PlayMode == TIME_MODE)
	{
		SetWndGrid(top, left, twidth, theight, 4,2);

		m_PlayCtrl[0]->show();
		repaint();
	}
}

void  CDialogMain::SetWndGrid(int &top, int &left, int &twidth, int &theight, int num,int grid)
{
	int wndDivLine = 2;
	int width = twidth / grid - wndDivLine;
	int height = theight / grid - wndDivLine;
	for (int i = 0; i < num; i++)
	{
		int row = i / grid;//����
		int col = i % grid;//����

		m_PlayWnd[i]->setGeometry(left + width*col + wndDivLine*col, top + height*row + wndDivLine*row, width, height);
		m_PlayWnd[i]->show();
		SetRenderMode(i);
	}
	m_curWndNum = num;
	ui.frameComboBox->hide();
	ui.frameBackGround->hide();
}

//���������� 10, 630, 700, 100);��m_playMode�޹�
void  CDialogMain::SetPlayCtrlPos()
{
	int ctrlleft = 0;
	int ctrltop =595;
	int ctrlwidth = 730;
	int ctrlheight = 70;
	if (m_bMax)
	{
		ctrltop = m_iScreenHeigth - 67;
		ctrlwidth = m_iScreenWidth - 200;

	}
	if (m_bHide)
	{
		ctrlwidth += 200;
	}
	for (int i = 0; i < NUM; i++)
	{
		m_PlayCtrl[i]->setGeometry(ctrlleft, ctrltop, ctrlwidth, ctrlheight);
		ui.frame_CtrlBar->setGeometry(ctrlleft, ctrltop, ctrlwidth, ctrlheight);
	}
}

void  CDialogMain::SetBtnHideListPos()
{
	int left1 = 700;
	int top1 = 630;
	if (m_bMax)
	{
		left1 = m_iScreenWidth - 230;
		top1 = m_iScreenHeigth - 40;
		
	}
	if (m_bHide)
	{
		left1 += 200;
	}
	
	ui.BtnHideList->move(left1,top1);
}

bool  CDialogMain::OpenAndPlayFile(CFormPlayCtrl *pPlayCtrl, QString strFileName)
{
	bool bRet = false;
	int index = pPlayCtrl->Index();
	if (m_PlayMode == SINGLE_MODE)
	{
		if (pPlayCtrl->IsPlaying())
		{
			pPlayCtrl->Stop();
		}
		pPlayCtrl->SetPlayMode(m_PlayMode);
		
		bRet = pPlayCtrl->OpenAndPlayFile(strFileName, (HWND)m_PlayWnd[index]->GetPlayWndID());
		if (bRet&&m_WndMode == WND1)
		{
			ui.frameComboBox->hide();
			ui.frameBackGround->hide();
		}

		m_PlayWnd[index]->SetTitleFileName(strFileName);
		SetRenderMode(index);
	}
	else if (m_PlayMode == TIME_MODE)
	{
		StopAll();
		index = 0;
		m_PlayCtrl[0]->SetPlayMode(m_PlayMode);
		bRet = m_PlayCtrl[0]->OpenAndPlayFile(strFileName, (HWND)m_PlayWnd[0]->GetPlayWndID());
		m_PlayWnd[0]->SetTitleFileName(strFileName);
		SetRenderMode(0);
	}
	if (!bRet)
	{
		CDialogMessageBox dlg;
		QFile file(strFileName);
		if (!file.exists())
		{
			dlg.SetText1(QStringLiteral("�ļ�·�������ڣ�"));

		}
		else
		{
			dlg.SetText1(QStringLiteral("�޷����ţ��ļ����ܱ��𻵣�"));		
		}
		dlg.SetText2(strFileName);
		dlg.exec();
		if (m_playListMode == LIST || m_playListMode == SINGLE_REPEAT || m_playListMode == LIST_REPEAT)
		{
			if (m_PlayMode == SINGLE_MODE)
			{
				QApplication::postEvent(this, new CMyEvent(CUSTOM_AutoStop_EVENT, pPlayCtrl));
			}
			else if (m_PlayMode == TIME_MODE)
			{
				QApplication::postEvent(this, new CMyEvent(CUSTOM_AutoStop_EVENT, m_PlayCtrl[0]));
			}
		}
		
	}
	return bRet;
}

void  CDialogMain::SetRenderMode(int index)
{
	int width = 0;
	int height = 0;
	bool bRet = false;
//	QThread::msleep(10);
	bRet=m_PlayCtrl[index]->GetPicSize((long*)&width,(long*) &height);
/*	if (width == 0 || height == 0)
	{
		QThread::usleep(1000);//�󻪵ĳ�����������ȡ�ɹ��ٻ�ȡһ�� Ҳ����?
		bRet = m_PlayCtrl[index]->GetPicSize((long*)&width, (long*)&height);
	}*/
	if (bRet&&width != 0 && height != 0)
	{
		QRect rc = m_PlayWnd[index]->geometry();
		float fVideo = (float)width / (float)height;
		float fWnd = (float)rc.width() / (float)rc.height();
		QRect rcVideo = rc;
		if (m_renderMode == Render4_3)
		{
			fWnd = (float)4 / (float)3;
		}
		if (m_renderMode == Render4_3)
		{
			fWnd = (float)16 / (float)9;
		}

		if ((fVideo - fWnd) >= 0.000001)//��Wnd��Ϊ׼
		{
			int videoWidth = rc.width();
			int videoHeight = (float)videoWidth / (float)fVideo;
			rcVideo.setRect(0, (rc.height() - videoHeight) / 2, videoWidth, videoHeight);
		}
		else//��Wnd��Ϊ׼
		{
			int videoHeight = rc.height();
			int videoWidth = videoHeight * fVideo;
			rcVideo.setRect((rc.width() - videoWidth) / 2, 0, videoWidth, videoHeight);
		}
		
	
		if (m_renderMode == RenderIni)
		{
			m_PlayWnd[index]->SetPlayWndPos(rcVideo);
		}
		else if (m_renderMode == RenderStrech)
		{
			rcVideo.setRect(0, 0, rc.width(), rc.height());
			m_PlayWnd[index]->SetPlayWndPos(rcVideo);
		}
		else if (m_renderMode == Render4_3)
		{
			m_PlayWnd[index]->SetPlayWndPos(rcVideo);
		}
		else if (m_renderMode == Render16_9)
		{
			m_PlayWnd[index]->SetPlayWndPos(rcVideo);
		}
	}
	else
	{
		QRect rc = m_PlayWnd[index]->geometry();
		QRect rcVideo = rc;
		rcVideo.setRect(0, 0, rc.width(), rc.height());
		m_PlayWnd[index]->SetPlayWndPos(rcVideo);
	}
}

void CDialogMain::ShellExe2()
{
	if (!this->isEnabled())//ukey������
	{
		return;
	}
	QString strShellFileXml = QApplication::applicationDirPath() + "/" + "shellFile.xml";
	QFile file(strShellFileXml);
	QDomDocument doc;
	if (!file.open(QIODevice::ReadOnly))
	{
		return;
	}

	if (!doc.setContent(&file))
	{
		file.close();
		return;
	}
	//���ļ��б�
	QDomElement root = doc.documentElement();
	if (root.isNull())
	{
		file.close();
		return;
	}

	QDomElement  note = root.firstChildElement();
	QString strPathName = "";
	while (!note.isNull())
	{
		strPathName = note.text();	
		break;
	}

	file.close();
	ShellExe(strPathName);
}

void CDialogMain::ShellExe(QString &strFile)
{
	if (strFile=="")
	{
		return;
	}
	AddToPlayListWidget(strFile);

	CFormPlayCtrl *pPlayerCtrl = NULL;
	if (m_WndMode == WND1)
	{
		pPlayerCtrl = m_PlayCtrl[0];
	}
	for (int i = 0; i < m_curWndNum; i++)
	{
		if (!m_PlayCtrl[i]->IsPlaying())
		{
			pPlayerCtrl = m_PlayCtrl[i];
			break;
		}
	}

	if (pPlayerCtrl != NULL)
	{
		OpenAndPlayFile(pPlayerCtrl,strFile);
		SetCurPlayListIndex(strFile);
	}
}

void  CDialogMain::StopAll()//�ֶβ����л����������ţ�����������л����ֶβ���
{
	for (int i = 0; i < NUM; i++)//���ﲻ����m_curWndNum�ˣ���Ϊ16���л���9����ʱ����Щ���ڻ��ڲ��ţ�ֻ��û��ʾ
	{
		if (m_PlayCtrl[i]->IsPlaying())
		{
			m_PlayCtrl[i]->Stop();
		}
	}
}

void CDialogMain::OnBtnCloseClick()
{
	s_ukeyThread.Stop();
	s_ukeyThread.wait();

	StopAll();
	ReleasePlayerFactory();
	WritePlayFileList();//д�����б�

	
	this->close();
}

void CDialogMain::OnBtnDiv4TimeClick()
{
	if (m_PlayMode == TIME_MODE)
	{
		OnWndModeTriggered(m_Wnd1Act);
		return;
	}

	bool bPlaying = false;
	for (int i = 0; i < NUM; i++)
	{
		if (m_PlayCtrl[i]->IsPlaying())
		{
			bPlaying = true;
		}
	}
	if (bPlaying)
	{
		if (QMessageBox::No == QMessageBox::question(this, QStringLiteral("��ʾ"),
			QStringLiteral("�Ƿ��˳���ǰ���в��ţ�"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No))
		{
			return;
		}
		StopAll();
	}
	SetPlayMode(TIME_MODE);
	ui.BtnDiv4Time->setStyleSheet("QPushButton{border-image: url(:/Skin/divPlay.png)0 0 0 28 repeat}QPushButton:hover{border-image: url(:/Skin/divPlay.png)28 0 0 28 repeat}QPushButton:pressed{border-image: url(:/Skin/divPlay.png)56 0 0 28 repeat}QPushButton:disabled{border-image: url(: / Skin/divPlay.png)84 0 0 28 repeat}");
}

void CDialogMain::OnBtnFileDelClick()
{
	int row = ui.listWidget->currentRow();
	if (row < 0)
	{
		return;
	}
	QString strFile = m_strPlayFileList.at(row);
	for (int i = 0; i < NUM; i++)
	{
		if (m_PlayCtrl[i]->IsPlaying())
		{
			if (strFile == m_PlayCtrl[i]->GetCurPlayFileName())
			{
				m_PlayCtrl[i]->Stop();
			}
		}
		
	}
	QListWidgetItem *item = ui.listWidget->takeItem(row);
	delete item;
	m_strPlayFileList.removeAt(row);
}

void CDialogMain::OnBtnFileDelAllClick()
{
	int size = m_strPlayFileList.size();
	if (size == 0)
	{
		return;
	}
	if (QMessageBox::No == QMessageBox::question(this, QStringLiteral("��ʾ"),
		QStringLiteral("ȷ��Ҫ��������ļ�����ֹͣ���ţ�"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No))
	{
		return;
	}
	StopAll();

	ui.listWidget->clear();
	m_strPlayFileList.clear();
}


void CDialogMain::OnActivated(int index)
{
	//CDialogDubbing dlg(this);
	if (QDialog::Accepted == m_DubDlg.exec())
	{
		QString videoFile = m_DubDlg.GetVideoFileName();
		QString AudioFile = m_DubDlg.GetAudioFileName();
		OpenAndPlayFile(m_PlayCtrl[0], videoFile);
		m_PlayCtrl[0]->OpenAudioFile(AudioFile);
	}
}

void CDialogMain::AddToPlayListWidget(QString strPathName)
{
	strPathName.replace("\\", "/");
	int index = strPathName.lastIndexOf("/");

	QString strName = strPathName.mid(index + 1);

	int size = m_strPlayFileList.size();
	if (size != 0)
	{
		for (int i = 0; i < size; i++)
		{
			if (strPathName == m_strPlayFileList.at(i))
			{
				return;
			}
		}
	}

	QListWidgetItem *item = new QListWidgetItem(strName);
	item->setToolTip(strPathName);
	ui.listWidget->addItem(item);

	m_strPlayFileList.append(strPathName);
}

void CDialogMain::OnBtnFileAddClick()
{
	QStringList fileList = QFileDialog::getOpenFileNames(this,
		QStringLiteral("��ѡ���ļ�"), "", FileFilter);
	int size = fileList.size();
	if (size != 0)
	{
		for (int i = 0; i < size; i++)
		{
			AddToPlayListWidget(fileList.at(i));

		}//ֻ���ӵ��б�������
		SetCurPlayListIndex(fileList.at(0));
	}
}

void CDialogMain::OnBtnFileOpenClick()
{
	QStringList fileList = QFileDialog::getOpenFileNames(this,
		QStringLiteral("��ѡ���ļ�"), "", FileFilter);
	int size = fileList.size();
	if (size != 0)
	{
		QString strFirstName;
		for (int i = 0; i < size; i++)
		{
			QString strPathName = fileList.at(i);
			if (i == 0)
			{
				strFirstName = strPathName;
			}
			AddToPlayListWidget(strPathName);

		}//
		OpenAndPlayFile(m_PlayCtrl[0],strFirstName);
		SetCurPlayListIndex(strFirstName);
	}
}

void CDialogMain::SetCurPlayListIndex(QString strPathName)
{
	int count = m_strPlayFileList.size();
	for (int i = 0; i < count; i++)
	{
		if (strPathName == m_strPlayFileList.at(i))
		{
			ui.listWidget->setCurrentRow(i);
			break;
		}
	}
}

void CDialogMain::dragEnterEvent(QDragEnterEvent *event)
{
	event->acceptProposedAction();
}
/*
void CDialogMain::dragMoveEvent(QDragMoveEvent *event)
{
//	event->acceptProposedAction();
}
*/
void CDialogMain::dropEvent(QDropEvent * event)
{
	const QMimeData *mimeData = event->mimeData();
	if (mimeData->hasUrls())
	{
		QList<QUrl> urlList = mimeData->urls();
		QString strFirstName;
		for (int i = 0; i < urlList.size(); ++i) 
		{
			QString url = urlList.at(i).path();
			QString strPathName = url.mid(1);
			if (i == 0)
			{
				strFirstName = strPathName;
			}
			AddToPlayListWidget(strPathName);
			SetCurPlayListIndex(strPathName);
			
		}
		QPoint pos = event->pos();
		for (int i = 0; i < m_curWndNum; i++)
		{
			QRect rc = m_PlayWnd[i]->geometry();

			if (rc.contains(pos))
			{
				OpenAndPlayFile(m_PlayCtrl[i],strFirstName);
				break;
			}
		}

	}
	
	event->acceptProposedAction();
}

void CDialogMain::OnListWidgetItemDbClick(QListWidgetItem * item)
{
	//int index = ui.listWidget->row(item);
	//OpenAndPlayFile(m_PlayCtrl[m_iCurFocus],m_strPlayFileList.at(index));
	QApplication::postEvent(this, new CMyEvent(CUSTOM_ListItemDbClick_EVENT, (QObject*)item));
}
/*
void CDialogMain::keyPressEvent(QKeyEvent * keyEvent)
{
 	if (keyEvent->key() == Qt::Key_Escape) {
		if (m_bFullScreen)
		{
			SetFullScreen(m_PlayWnd[m_iCurFocus]);
		}
		keyEvent->accept();
		return ;
	}
	else if (keyEvent->key() == Qt::Key_Enter) {
		return ;
	} 
}

void CDialogMain::keyReleaseEvent(QKeyEvent * keyEvent)
{
	keyEvent->accept();
	if (keyEvent->key() == Qt::Key_Escape) {
		if (m_bFullScreen)
		{
			SetFullScreen(m_PlayWnd[m_iCurFocus]);
		}
		return;
	}
	else if (keyEvent->key() == Qt::Key_Enter) {
		return;
	}
}
*/
bool CDialogMain::eventFilter(QObject *obj, QEvent *e)
{
	//qDebug("e->type()��%d", e->type());
	if (e->type() == QEvent::KeyPress)
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
		if (keyEvent->key() == Qt::Key_Escape) {//Key_Escape�����KeyRelease��Ӧ�Ļ���ֱ���˳���
			if (m_bFullScreen)
			{
				SetFullScreen(m_PlayWnd[m_iCurFocus]);
			}
			return true;
		}
		else if (keyEvent->key() == Qt::Key_Space) //��ť�������˵����ģ��������Ͻ���չ���ܰ�ť�����¿ո����ֻ���� QEvent::KeyPress���˵������򿪡����ε�KeyPress�󣬻����KeyRelease
		{
			return true;
		}

	//	����slider���������Ҽ��İ��µķ�Ӧ��ʹsliderλ�ò��ı䣬ͨ��SetPlaySliderPos�ı�.����
		
		else if (keyEvent->key() == Qt::Key_Left) {
			return true;
		}
		else if (keyEvent->key() == Qt::Key_Right) {
			return true;
		}

		if (!ui.listWidget->hasFocus())
		{
			if (keyEvent->key() == Qt::Key_Up) //
			{
				return true;
			}
			else if (keyEvent->key() == Qt::Key_Down) {
				return true;
			}
		}
	}
	else if (e->type() == QEvent::KeyRelease)//�����Ϊʲôû��QEvent::KeyPress�¼�������Ҫ��QEvent::KeyRelease
	{
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
		//	else if (keyEvent->key() == Qt::Key_Enter) {
		//	return true;
		//	}
		 if (keyEvent->key() == Qt::Key_Space) {
			m_PlayCtrl[m_iCurFocus]->OnBtnPauseClick();
			return true;
		}
		else if (keyEvent->key() == Qt::Key_Left) {
			m_PlayCtrl[m_iCurFocus]->SetPlaySliderPos(-1);
			return true;
		}
		else if (keyEvent->key() == Qt::Key_Right) {
			m_PlayCtrl[m_iCurFocus]->SetPlaySliderPos(1);
			return true;
		}
		if (!ui.listWidget->hasFocus())
		{
			if (keyEvent->key() == Qt::Key_Up) {
				m_PlayCtrl[m_iCurFocus]->SetVolumeSliderPos(0xffff * 0.1);
				return true;
			}
			else if (keyEvent->key() == Qt::Key_Down) {
				m_PlayCtrl[m_iCurFocus]->SetVolumeSliderPos(-0xffff * 0.1);
				return true;
			}
		}
	}
	else if (e->type() == QEvent::MouseMove)
	{
		if (m_bFullScreen)
		{
			return true;
		}
	}
	else if (e->type() == QEvent::MouseButtonDblClick)
	{
		QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
		
		QRect rc = ui.frameSysBar->geometry();
		QPoint mousePos = ui.frameSysBar->mapFromGlobal(QCursor::pos());

		if (rc.contains(mousePos)){
			OnBtnMaxClick();
			return true;
		}
	}
	else if (e->type() == CUSTOM_ListItemDbClick_EVENT)
	{	
		CMyEvent *myEvent = (CMyEvent *)e;
		QListWidgetItem *pItem= (QListWidgetItem*)myEvent->obj;
		int index = ui.listWidget->row(pItem);
		OpenAndPlayFile(m_PlayCtrl[m_iCurFocus],m_strPlayFileList.at(index));
		return true;
		
	}
	else if (e->type() == CUSTOM_PLAYWNDPRESSED_EVENT)
	{	
		CMyEvent *myEvent = (CMyEvent *)e;
		CWidgetPlayWnd *pPlayWnd = (CWidgetPlayWnd*)myEvent->obj;
		if (pPlayWnd)
		{
			m_iCurFocus = pPlayWnd->Index();
			if (m_PlayCtrl[m_iCurFocus]->IsPlaying())
			{
				QString strFile = m_PlayCtrl[m_iCurFocus]->GetCurPlayFileName();
				SetCurPlayListIndex(strFile);
			}

			for (int i = 0; i<m_curWndNum; i++)
			{
				m_PlayCtrl[i]->hide();
				if (i == m_iCurFocus){				
						m_PlayCtrl[i]->show();
						this->repaint();
					
				}
			}
		}
		return true;
	}
	else if (e->type() == CUSTOM_PIC_EVENT)
	{
		CDialogConfig::GetPicParam(m_picPath, m_picType, m_picRepeatCount);
		//�ж�·���Ƿ���ڣ��������򴴽����������ɹ�����

		if (!CheckPicPath(m_picPath))
		{
			return true;
		}
		CMyEvent *myEvent = (CMyEvent *)e;
		CWidgetPlayWnd *pPlayWnd = (CWidgetPlayWnd*)myEvent->obj;
		int index = pPlayWnd->Index();
		QString strName = m_PlayCtrl[index]->GetCurPlayFileName();
		int pos1 = strName.lastIndexOf('/');
		int pos2 = strName.lastIndexOf('.');
		strName = strName.mid(pos1 + 1, pos2 - pos1 - 1);
		QDateTime dateTime = QDateTime::currentDateTime();// m_CapPicRepeatCount
		QString strFilePathName = "";
		CDialogConfig::GetPicParam(m_picPath, m_picType, m_picRepeatCount);
		QString strPicType = ".jpg";
		if (m_picType == 0)
		{
			strPicType = ".bmp";
		}
		strFilePathName = strFilePathName.append(m_picPath).append("/").append(strName).append(dateTime.toString("_yyyyMMddhhmmss").append(strPicType));
		bool bRet = m_PlayCtrl[index]->CapturePic(strFilePathName, m_picType);
		if (bRet)
		{
			CDialogMessageBox dlg(this);
			dlg.SetText1(QStringLiteral("����ɹ���"));
			dlg.SetText2(strFilePathName);
			dlg.exec();
		}
		return true;
	}
	else if (e->type() == CUSTOM_PICREPEAT_EVENT)
	{
		
		CDialogConfig::GetPicParam(m_picPath, m_picType, m_picRepeatCount);
		//�ж�·���Ƿ���ڣ��������򴴽����������ɹ�����

		if (!CheckPicPath(m_picPath))
		{
			return true;
		}

		CMyEvent *myEvent = (CMyEvent *)e;
		CWidgetPlayWnd *pPlayWnd = (CWidgetPlayWnd*)myEvent->obj;

		int index = pPlayWnd->Index();
		QString strName = m_PlayCtrl[index]->GetCurPlayFileName();
		int pos1 = strName.lastIndexOf('/');
		int pos2 = strName.lastIndexOf('.');
		strName = strName.mid(pos1 + 1, pos2 - pos1 - 1);
		QDateTime dateTime = QDateTime::currentDateTime();// 
		QString strTime = dateTime.toString("_yyyyMMddhhmmss");	
		QString strPicType = ".jpg";
		if (m_picType == 0)
		{
			strPicType = ".bmp";
		}
		bool bRet = false;
		QString strFileList;
		for (int i = 0; i < m_picRepeatCount; i++)
		{
			QString strFilePathName = "";
			strFilePathName = strFilePathName.append(m_picPath).append("/").append(strName).append(strTime).append("_(%1_%2)").append(strPicType).arg(m_picRepeatCount).arg(i + 1);
			bRet=m_PlayCtrl[index]->CapturePic(strFilePathName, m_picType);
			strFileList.append(strFilePathName);
			strFileList.append("\n");
		}

		if (bRet)
		{
			CDialogMessageBox dlg(this);
			dlg.SetText1(QStringLiteral("����ɹ���"));
			dlg.SetText2(strFileList);
			dlg.exec();
		}
		return true;
	}
	else if (e->type() == CUSTOM_PLAYWNDDBCLICK_EVENT)
	{
		CMyEvent *myEvent = (CMyEvent *)e;
		CWidgetPlayWnd *pPlayWnd = (CWidgetPlayWnd*)myEvent->obj;
		
		SetFullScreen(pPlayWnd);
		return true;
	}
	else if (e->type() == CUSTOM_AutoStop_EVENT || e->type() == CUSTOM_ManualStop_EVENT)
	{
		CMyEvent *myEvent = (CMyEvent *)e;
		CFormPlayCtrl *pPlayerCtrl = (CFormPlayCtrl*)myEvent->obj;	
		int index = pPlayerCtrl->Index();
		if (m_bFullScreen&&index==m_iCurFocus)
		{
			SetFullScreen(m_PlayWnd[index]);
		}
		if (pPlayerCtrl->IsPlaying())
		{
			pPlayerCtrl->Stop();
		}
		
		if (m_PlayMode == SINGLE_MODE)
		{
			if (m_WndMode == WND1)
			{
				ui.frameBackGround->show();
				ui.frameBackGround->raise();
				ui.frameComboBox->show();
				ui.frameComboBox->raise();
			}

		}
		else if (m_PlayMode == TIME_MODE)
		{
			bool bPlaying = false;
			for (int i = 0; i<4; i++){
				if (m_PlayCtrl[i]->IsPlaying()){
					bPlaying = true;//�ֶβ����������ڲ��ţ����أ����������һ������
					return true;
				} 
			}
		}

		if (e->type() == CUSTOM_ManualStop_EVENT)//�ֶ�ֹͣ��������һ������
		{
			return true;
		}
		
		
	/*	if (m_playListMode == SINGLE)//��������
		{

		}*/
		if (m_playListMode == LIST)
		{
			PlayIndex(pPlayerCtrl,true);
		}
		else if (m_playListMode == SINGLE_REPEAT)
		{
			QString strFileName = pPlayerCtrl->GetCurPlayFileName();
			OpenAndPlayFile(pPlayerCtrl,strFileName);
		}
		else if (m_playListMode == LIST_REPEAT)
		{
			PlayIndex(pPlayerCtrl,true, true);
		}
		
		return true;
	}
	else if (e->type() == CUSTOM_PREVIOUS_EVENT)
	{
		CMyEvent *myEvent = (CMyEvent *)e;
		CFormPlayCtrl *pPlayerCtrl = (CFormPlayCtrl*)myEvent->obj;
		PlayIndex(pPlayerCtrl,false,false);
		return true;
	}
	else if (e->type() == CUSTOM_NEXT_EVENT)
	{
		CMyEvent *myEvent = (CMyEvent *)e;
		CFormPlayCtrl *pPlayerCtrl = (CFormPlayCtrl*)myEvent->obj;
		PlayIndex(pPlayerCtrl,true);
		return true;
	}
	else if (e->type() == CUSTOM_NOTDIV_EVENT)
	{
		QMessageBox::information(this, QStringLiteral("��ʾ"), QStringLiteral("�ļ����̣���֧�ֶַβ��ţ�"));
		return true;
	}
	else if (e->type() == CUSTOM_DIV_EVENT)
	{
		CDivEvent *divEvent = (CDivEvent *)e;
		QString strFileName = divEvent->strFileName;
		QString strDivFile = strFileName;
		m_PlayWnd[0]->SetTitleFileName(strDivFile.append(QStringLiteral("_�ֶ�%1")).arg(1));
		for (int i = 1; i < 4; i++)//NUM
		{
			m_PlayCtrl[i]->SetPlayMode(m_PlayMode);
			bool ret = m_PlayCtrl[i]->OpenAndPlayFile(strFileName, (HWND)m_PlayWnd[i]->GetPlayWndID());

			strDivFile = strFileName;
			m_PlayWnd[i]->SetTitleFileName(strDivFile.append(QStringLiteral("_�ֶ�%1")).arg(i+1));
			SetRenderMode(i);
			QThread::usleep(10);
		}
		return true;
	}
	return QWidget::eventFilter(obj, e);
}

bool CDialogMain::CheckPicPath(QString picPath)
{
	QDir dir(picPath);
	bool bRet=dir.exists();
	if (bRet)
	{
		return true;
	}
	return dir.mkpath(picPath);
}

void CDialogMain::PlayIndex(CFormPlayCtrl *pPlayerCtrl, bool bNext, bool bListRepeat)
{
	QString strFileName = pPlayerCtrl->GetCurPlayFileName();
	bool bFind = false;
	int index = 0;
	int size = m_strPlayFileList.size();
	if (size != 0)
	{
		for (int i = 0; i < size; i++)
		{
			if (strFileName.compare(m_strPlayFileList.at(i)) == 0)
			{
				bFind = true;
				if (bNext)
				{
					index = i + 1;
				}
				else
				{
					index = i - 1;
				}
				break;
			}
		}
	}
	if (bListRepeat)
	{
		if (index >= size)
		{
			index = 0;
		}
	}
	if (bFind&&index >=0 && index < size)
	{
		OpenAndPlayFile(pPlayerCtrl, m_strPlayFileList.at(index));
		ui.listWidget->setCurrentRow(index);
	}
	else
	{
		//if (m_bPlay)
		//{
		//	Stop();
		//}

	//	ui.labelFileName->setText("");
	//	m_strCurPlayFileName = "";
	}
	
}

void CDialogMain::SetPlayCtrlVisible(bool bShow)
{
	if (!m_bFullScreen)
	{
		return;
	}

	if (bShow)
	{
		ui.frame_CtrlBar->raise();
		m_PlayCtrl[m_iCurFocus]->raise();
		m_PlayCtrl[m_iCurFocus]->show();
		ui.frame_CtrlBar->show();
	}
	else
	{
		m_PlayCtrl[m_iCurFocus]->hide();
		ui.frame_CtrlBar->hide();

	}
}

bool CDialogMain::IsPlayCtrlVisible(int x,int y)
{
	if (!m_bFullScreen)
	{
		return false;
	}
	QRect rc = ui.frame_CtrlBar->geometry();
	
	if (rc.contains(x,y))
	{
		SetPlayCtrlVisible(true);
		return true;
	}
	else
	{
		SetPlayCtrlVisible(false);
		return false;

	}
}

void CDialogMain::SetFullScreen(QWidget *pPlayWnd)
{
	int index = ((CWidgetPlayWnd*)pPlayWnd)->Index();
	if (!m_PlayCtrl[index]->IsPlaying())
	{
		return;
	}
	m_bFullScreen = !m_bFullScreen;
	if (m_bFullScreen)
	{
		m_rcPlayWndNormal = pPlayWnd->geometry();
		m_rcPlayCtrlNormal = m_PlayCtrl[index]->geometry();

		int width = QApplication::desktop()->width();
		int height = QApplication::desktop()->height();
		this->showFullScreen();
		pPlayWnd->setGeometry(0, 0, width, height);
		
		ui.frameSysBar->hide();
	
		ui.frame_toolBar->hide();
		ui.BtnHideList->hide();
		ui.frameBackGround->hide();
		ui.frameComboBox->hide();
	
		ui.frame_fileList->lower();
		pPlayWnd->raise();
		
		m_PlayCtrl[index]->setGeometry(0, height - 70, width, 70);
		ui.frame_CtrlBar->setGeometry(0, height - 70, width, 70);
	//	ui.frame_CtrlBar->raise();
	//	m_PlayCtrl[index]->raise();
		m_PlayCtrl[index]->hide();
		ui.frame_CtrlBar->hide();


	}
	else
	{
		if (m_bMax)
		{
			this->showFullScreen();
		}
		else
		{
			this->showNormal();
		}

		pPlayWnd->setGeometry(m_rcPlayWndNormal);
		m_PlayCtrl[index]->setGeometry(m_rcPlayCtrlNormal);
		ui.frame_CtrlBar->setGeometry(m_rcPlayCtrlNormal);
		m_PlayCtrl[index]->lower();
		ui.frame_CtrlBar->lower();
		
		ui.frameSysBar->show();
		ui.frame_CtrlBar->show();
		ui.frame_toolBar->show();
		ui.BtnHideList->show();

		m_PlayCtrl[index]->show();

	}

	SetRenderMode(index);
}

void CDialogMain::OnBtnFullScreenClick()
{
	SetFullScreen(m_PlayWnd[m_iCurFocus]);
}

void CDialogMain::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	//painter.fillRect(this->rect(), QColor(0, 0, 0));
	if (m_PlayMode == SINGLE_MODE&&m_WndMode == WND1)
	{
			QRect rc = m_PlayWnd[0]->geometry();
			rc.adjust(-1, -1, 1, 1);
			painter.setPen(QColor(50, 50, 20));
			painter.drawRect(rc);
	}
	else 
	{
		PaintWndRect(painter, m_curWndNum);
	}
}

void CDialogMain::PaintWndRect(QPainter &painter, int num)
{
	for (int i = 0; i < num; i++)
	{
		QRect rc = m_PlayWnd[i]->geometry();
		rc.adjust(-1, -1, 1, 1);
		painter.setPen(QColor(50, 50, 20));
		painter.drawRect(rc);
	}

	if (m_iCurFocus < NUM&&m_iCurFocus>-1)
	{
		QRect rc = m_PlayWnd[m_iCurFocus]->geometry();
		rc.adjust(-1, -1, 1, 1);
		painter.setPen(QColor(255, 255, 0));
		painter.drawRect(rc);
	}
}
#include "cdialogcut.h"
#include "qfiledialog.h"
#include "public_def.h"
#include <qthread.h>
#include <qmessagebox.h>
#include <qtextcodec.h>

#include "cdialogconfig.h"

CDialogCut::CDialogCut(QWidget *parent)
: NoFlameDlg(parent), m_nPort(-1)
{
	ui.setupUi(this);

	CDialogConfig::ReadConfigXml();
	ui.lineEditDstPath->setText(CDialogConfig::m_strCutPath);
	m_strDstPath = CDialogConfig::m_strCutPath;

	m_PlayWnd = new CWidgetPlayWnd(0, this);
	m_PlayWndTest = new CWidgetPlayWnd(1, this);
	m_PlayCtrl = new CFormPlayCtrl(0, m_PlayWnd, this);
	m_PlayCtrlTest = new CFormPlayCtrl(1, m_PlayWndTest, this);
	m_PlayWndTest->hide();
	m_PlayCtrlTest->hide();

	m_getProgressTimer = new QTimer(this);

	m_PlayWnd->SetPreViewMode();
	m_PlayWnd->setGeometry(3, 33,930, 440);
	m_PlayCtrl->setGeometry(3, 33 + 440 , 930, 70);
	m_PlayCtrl->HideNotUsedButton(); 

	ui.frame_2->lower();
	ui.frameComboBox->raise();
	ui.progressBar->setDisabled(1);

	connect(ui.BtnFileOpen, SIGNAL(clicked()), this, SLOT(OnBtnFileOpenClick()));

	connect(ui.BtnClose, SIGNAL(clicked()), this, SLOT(OnBtnCloseClick()));
	connect(ui.BtnMin, SIGNAL(clicked()), this, SLOT(OnBtnMinClick()));

	connect(m_getProgressTimer, SIGNAL(timeout()), this, SLOT(TimeGetProgress()));
	connect(ui.BtnStartCut, SIGNAL(clicked()), this, SLOT(OnBtnStartCutClick()));
	connect(ui.BtnCutStop, SIGNAL(clicked()), this, SLOT(OnBtnCutStopClick()));

	connect(ui.BtnFilePath, SIGNAL(clicked()), this, SLOT(OnBtnFilePathClick()));
	connect(ui.BtnTimeStart, SIGNAL(clicked()), this, SLOT(OnBtnTimeStartClick()));
	connect(ui.BtnTimeStop, SIGNAL(clicked()), this, SLOT(OnBtnTimeStopClick()));

	installEventFilter(this);
}

CDialogCut::~CDialogCut()
{
	delete m_PlayWnd;
	delete m_PlayCtrl;
	delete m_PlayWndTest;
	delete m_PlayCtrlTest;
	delete m_getProgressTimer;
}

void CDialogCut::UkeyDownStop()
{
	if (m_PlayCtrl->IsPlaying())
	{
		m_PlayCtrl->Pause(1);
	}
	if (!ui.BtnStartCut->isEnabled())
	{	
		OnBtnCutStopClick();	
	}
}


void CDialogCut::OnBtnCloseClick()
{
	if (!ui.BtnStartCut->isEnabled())
	{
		QMessageBox::information(this, QStringLiteral("��ʾ"), QStringLiteral("����ֹͣ����"));
		return;
	}
	CDialogConfig::m_strCutPath = m_strDstPath; 
	CDialogConfig::WriteConfigXml();

	m_PlayCtrl->Stop();
	this->accept();
}

void CDialogCut::OnBtnMinClick()
{
	this->showMinimized();
}

void  CDialogCut::TimeGetProgress()
{
	if (m_nPort != -1)
	{
		int progress = Player_FileConvertProcess(m_nPort);
		ui.progressBar->setValue(progress);
		if (progress == 100)
		{
			StopTask();

			bool bRet = m_PlayCtrlTest->OpenAndPlayFile(m_strDstPathName, NULL);
			if (bRet)
			{
				m_PlayCtrlTest->StopAudio();
				ui.labelState->setText(QStringLiteral("�ɹ�"));
			}
			else
			{
				ui.progressBar->setValue(0);
				ui.labelState->setText(QStringLiteral("ʧ��(���ļ���֧��)"));
			}
			m_PlayCtrlTest->Stop();
		}
		/*
		else if (progress == -1)//����
		{
			StopTask();
			ui.labelState->setText(QStringLiteral("����"));
		}*/

	}
}

void  CDialogCut::OnBtnCutStopClick()
{
	ui.labelState->setText(QStringLiteral("ֹͣ��"));
	StopTask();
}

void  CDialogCut::StopTask()
{
	if (m_getProgressTimer->isActive())
	{
		m_getProgressTimer->stop();
	}
	
	Player_FileConvertClose(m_nPort);
	FreePlayerInterface(m_nPort);
	

	SetCtrlEnabled(true);
}

void  CDialogCut::OnBtnStartCutClick()
{
	if (!m_PlayCtrl->IsPlaying())
	{
		return;
	}
	QTime t1 = ui.timeEditStart->time();
	unsigned long long dStartTime = t1.hour() * 3600 + t1.minute()* 60 + t1.second();

	QTime  t2 = ui.timeEditStop->time();
	unsigned long long dStopTime = t2.hour() * 3600 + t2.minute() * 60 + t2.second();

	if (dStartTime >= dStopTime)
	{
		QMessageBox::information(this, QStringLiteral("��ʾ"), QStringLiteral("��ʼʱ�䲻�ܴ��ڻ���ڽ���ʱ�䣡"));
		return ;
	}

	int dTotalTime = m_PlayCtrl->GetFileTime();
	if (dStopTime>dTotalTime)
	{
		QMessageBox::information(this, QStringLiteral("��ʾ"), QStringLiteral("����ʱ�䲻�ܴ����ļ���ʱ�䣡"));
		return;
	}

	if (ui.lineEditDstPath->text().isEmpty())
	{
		QMessageBox::information(this, QStringLiteral("��ʾ"), QStringLiteral("�����뱣��·����"));
		return;
	}

	QDir dir(ui.lineEditDstPath->text());
	if (!dir.exists())
	{
		if (!dir.mkpath(ui.lineEditDstPath->text()))
		{
			QMessageBox::information(this, QStringLiteral("��ʾ"), QStringLiteral("��������·��ʧ�ܣ�"));
			return;
		}
	}

	if (GetPlayerInterface(&m_nPort))
	{
		QString strFileName=m_PlayCtrl->GetCurPlayFileName();

		char szFileName[1024];
		QTextCodec *textcode = QTextCodec::codecForName("GB18030");
		strcpy(szFileName, textcode->fromUnicode(strFileName).data());

		strFileName.replace("\\", "/");
		int index = strFileName.lastIndexOf("/");
		int index2 = strFileName.indexOf(".");
		QString strName = strFileName.mid(index + 1, index2-index-1);
		QString strDstPath = ui.lineEditDstPath->text();
		strDstPath.append("/").append(strName).append(QString("(%1-%2).avi").arg(t1.toString("hhmmss")).arg(t2.toString("hhmmss")));
		char szDstFileName[1024];
		strcpy(szDstFileName, textcode->fromUnicode(strDstPath).data());
		m_strDstPathName = strDstPath;

		bool bCut=Player_FileConvertStart(m_nPort, szFileName, szDstFileName, dStartTime, dStopTime, false, NULL);
		if (!bCut)
		{
			StopTask();
			ui.labelState->setText(QStringLiteral("ʧ��(���ļ���֧��)"));
			return;
		}
		ui.progressBar->setValue(0);
		ui.labelState->setText(QStringLiteral("���ڼ���..."));
		m_strDstPath = ui.lineEditDstPath->text();	
		m_getProgressTimer->start(1000);
		SetCtrlEnabled(false);
		
	}
}

void CDialogCut::OnBtnFileOpenClick()
{
	QString strPathName = QFileDialog::getOpenFileName(this,
		QStringLiteral("��ѡ���ļ�"), "", FileFilter);
	
	if (!strPathName.isEmpty())
	{		
		bool bRet = m_PlayCtrl->OpenAndPlayFile(strPathName, (HWND)m_PlayWnd->GetPlayWndID());
		if (bRet)
		{
			QThread::usleep(1000);
		//	m_PlayCtrl->OnBtnPauseClick();
			m_PlayWnd->SetTitleFileName(strPathName);
			ui.frameComboBox->hide();
			m_PlayCtrl->SetRenderMode();
		}
		else
		{
			m_PlayCtrl->Stop();
		}	
	}
}


bool CDialogCut::eventFilter(QObject *obj, QEvent *e)
{
	if (e->type() == CUSTOM_ManualStop_EVENT)
	{
		m_PlayCtrl->Stop();

		ui.timeEditStart->setTime(QTime(0, 0, 0)); 
		ui.timeEditStop->setTime(QTime(0, 0, 0));

		ui.frameComboBox->show();
		ui.frameComboBox->raise();
		ui.progressBar->setValue(0);
		ui.labelState->setText("");
		return true;

	}
	return QWidget::eventFilter(obj, e);
}


void CDialogCut::OnBtnFilePathClick()
{
	QString pathName = QFileDialog::getExistingDirectory(this,
		QStringLiteral("��ѡ���ļ���"));
	if (pathName != "")
	{
		ui.lineEditDstPath->setText(pathName);
	}
}

void CDialogCut::OnBtnTimeStartClick()
{
	DWORD dTime=m_PlayCtrl->GetCurPlayTime();

	//ui.timeEditStart->setTimeSpec(Qt::TimeSpec spec)
	int hours = dTime / 3600;
	int mins = dTime / 60 % 60;
	int secs = dTime % 60;
	QTime time(hours,mins,secs);
	ui.timeEditStart->setTime(time);
}

void CDialogCut::OnBtnTimeStopClick()
{
	DWORD dTime = m_PlayCtrl->GetCurPlayTime();
	int hours = dTime / 3600;
	int mins = dTime / 60 % 60;
	int secs = dTime % 60;
	QTime time(hours, mins, secs);
	ui.timeEditStop->setTime(time);
}
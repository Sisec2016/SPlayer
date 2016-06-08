#ifndef CWIDGETPLAYWND_H
#define CWIDGETPLAYWND_H

#include <QWidget>
#include "ui_cwidgetplaywnd.h"
#include <qpainter.h>
#include <time.h>

enum COUSTOM_EVENT{
	CUSTOM_AutoStop_EVENT= 5000 + 1,//�ļ��������Զ��ص�
	CUSTOM_ListItemDbClick_EVENT,//˫�������б��¼���ʹ��postEvent,��ֹ���ڷֶη���ʱ��˫�����ţ����ܻῨ��
	CUSTOM_ManualStop_EVENT,//�ֶ�ֹͣ�ط�
	CUSTOM_PLAYWNDPRESSED_EVENT,//���Ŵ��ڵ�����¼�
	CUSTOM_PLAYWNDDBCLICK_EVENT,//���Ŵ���˫���¼�
	CUSTOM_PIC_EVENT,//ץͼ
	CUSTOM_PICREPEAT_EVENT,//����
	CUSTOM_NEXT_EVENT,//���Ŵ���˫���¼�
	CUSTOM_PREVIOUS_EVENT,//���Ŵ���˫���¼�
	CUSTOM_DIV_EVENT,//ʱ�䳤�ȿɷ���
	CUSTOM_NOTDIV_EVENT,//ʱ�䳤��̫��
	CUSTOM__SETSPEED_EVENT,//���������¼���֪ͨ����
};

class CMyEvent :public QEvent
{
public:
	CMyEvent(int type, QObject *pObj) :QEvent((QEvent::Type)type), obj(pObj){}
	~CMyEvent(){}

	QObject *obj;
};

class CDivEvent :public QEvent
{
public:
	CDivEvent(int type, QString strFile) :QEvent((QEvent::Type)type), strFileName(strFile){}
	~CDivEvent(){}

	QString strFileName;
};

class CWidgetPlayWnd : public QWidget
{
	Q_OBJECT

public:
	CWidgetPlayWnd(int index,QWidget *parent = 0);
	~CWidgetPlayWnd();
signals:
	void ShowColorDlg();
public:
	virtual void paintEvent(QPaintEvent *event);
	virtual void resizeEvent(QResizeEvent * event);
	virtual void mousePressEvent(QMouseEvent * event);
	virtual void mouseMoveEvent(QMouseEvent * event);
	virtual void leaveEvent(QEvent * event);
	virtual void mouseDoubleClickEvent(QMouseEvent * event);
	virtual void wheelEvent(QWheelEvent * event);
public:
	void setCorrectTime(unsigned long long timeStamp)
	{
		if (timeStamp == 0)
		{
			ui.labelCorrectTime->setText("");
			return;
		}
		struct tm tTime = { 0 };
		localtime_s(&tTime, (const time_t*)&(timeStamp));
		/*������Ҳ��
		time_t tt = timeStamp;
		struct tm *tTime2 = localtime(&tt);*/
		char strTime[20] = { 0 };
		sprintf(strTime, "%d-%02d-%02d %02d:%02d:%02d", tTime.tm_year + 1900, tTime.tm_mon + 1, tTime.tm_mday, tTime.tm_hour, tTime.tm_min, tTime.tm_sec);
	//	sprintf(desFileName, "%d", timeStamp / 1000);
		ui.labelCorrectTime->setText(QString(strTime));
		ui.labelCorrectTime->setToolTip(QString(strTime));
	}
	void SetPreViewMode(){ 
		m_previewMode = true; 
		ui.BtnFix->hide(); 
		ui.BtnPic->hide();
		ui.BtnPicRepeat->hide();
		ui.BtnFullScreen->hide();
		ui.BtnColor->hide();
	}
	void Refresh(){
		this->repaint();
		ui.widgetRender->repaint();
	}
	int Index(){ return m_index; }
	int GetPlayWndID(){ return ui.widgetRender->winId(); }
	void SetSpeedString(QString strSpeed){ ui.labelSpeed->setText(strSpeed); }
	void SetPauseString(QString strPause){ ui.labelPause->setText(strPause); }
	void SetTitleFileName(QString strFile){ 
		strFile.replace("\\", "/");
		int index = strFile.lastIndexOf("/");
		QString strName = strFile.mid(index + 1);
		ui.labelFileName->setText(strName); ui.labelFileName->setToolTip(strName);
	}
	void SetPlayWndPos(QRect &rc){
		m_listScaleRC.clear(); 
		ui.widgetRender->setGeometry(rc);
	}
	void SetShowEnabled(bool bShowEnabled){
		m_bEnabled = bShowEnabled;
		if (bShowEnabled)
		{
			if (m_bFix)
			{
				ui.widgetToolBar->show();
			}
		}
		else 
		{		
		/*	if (m_bFix)
			{
				OnBtnFixClick();
			}*/
			ui.widgetToolBar->hide();
			m_listScaleRC.clear();
			ui.labelCorrectTime->setText("");
		}
	}

protected slots:
	void OnBtnFixClick();	
	void OnBtnColorClick();
	void OnBtnPicClick();
	void OnBtnPicRepeatClick();
	void OnBtnFullScreenClick();
private:
	Ui::CWidgetPlayWnd ui;
	int m_index;
	bool m_bEnabled;
	bool m_bFix;//�̶�������
	QList<QRect> m_listScaleRC;
	bool m_previewMode;//Ԥ��ģʽ
};

#endif // CWIDGETPLAYWND_H

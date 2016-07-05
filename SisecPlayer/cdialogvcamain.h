#ifndef CDIALOGVCAMAIN_H
#define CDIALOGVCAMAIN_H

#include <QDialog>
#include "ui_cdialogvcamain.h"
#include "cdialogvca.h"
#include "cwidgetvcaprogress.h"
#include "cwidgetvcaresult.h"
#include "NoFlameDlg.h"
#include <qmessagebox.h>


struct T_FileVCA
{
	QString strFileName;//����·����
	QList<QString> strVCAResultDataList;//���·��������ʱ���¼ÿ�μ��� /vcaData/fileName/time
};


struct T_CasePoint
{
	QString strCasePointName;
	QList<T_FileVCA> FileList;
};


class CDialogVCAMain : public NoFlameDlg
{
	Q_OBJECT

public:
	CDialogVCAMain(QWidget *parent = 0);
	~CDialogVCAMain();
	virtual void closeEvent(QCloseEvent *ev);
public:
	static void ReadPlayFileListVCA();
	static void WritePlayFileListVCA();
	static void NotifyResult(int iPointVCA, int indexFile);
	void StartShow();
	bool Close();
protected slots:
	void OnBtnCloseClick();
	void OnBtnMinClick();
//	void OnBtnCaseAddClick();
//	void OnBtnCasePointAddClick();

	void OnSetup();
private:
	Ui::CDialogVCAMain ui;

	QTimer *m_timer;
	QMessageBox m_dlgInit;
public:
	CDialogVCA *m_dlgVCA;
	CWidgetVCAProgress   *m_widgetProgress;
	CWidgetVCAResult *m_widgetResult;
	static QList<T_CasePoint> ListCasePoint;
};

#endif // CDIALOGVCAMAIN_H

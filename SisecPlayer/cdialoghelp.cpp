#include "cdialoghelp.h"
#include "cupdatedlg.h"
CDialogHelp::CDialogHelp(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	
	QString strXmlUrl, strPacketUrl, strCurVersion;
	QString strXmlPath = QApplication::applicationDirPath() + "/" + "update.xml";
	if (CUpdateDlg::ReadXml(strXmlPath, strXmlUrl, strPacketUrl, strCurVersion))
	{
		ui.label_2->setText(strCurVersion);
	}
	else
	{
		ui.label_2->setText(QStringLiteral("��ȡ��ǰ�汾ʧ�ܣ�"));
	}
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(OnBtnOKClick()));
}

CDialogHelp::~CDialogHelp()
{

}

void CDialogHelp::OnBtnOKClick()
{
	this->accept();
}

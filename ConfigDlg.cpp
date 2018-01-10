// ConfigDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "BackMirServer.h"
#include "ConfigDlg.h"
#include "../CommonModule/GDefine.h"
#include "Helper.h"
#include "GameWorld/GameWorld.h"
#include "runarg.h"

// CConfigDlg 对话框

IMPLEMENT_DYNAMIC(CConfigDlg, CDialog)

CConfigDlg::CConfigDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigDlg::IDD, pParent)
{
	m_bAutoKickDoorObstacle = false;
	m_bEnableElite = false;
	m_bShowDifficultySelect = true;
	m_bUsingOldEngine = false;
}

CConfigDlg::~CConfigDlg()
{
}

void CConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CConfigDlg, CDialog)
END_MESSAGE_MAP()


// CConfigDlg 消息处理程序
static int s_nDifficultyRadioBoxIds[] = {
	IDC_RADIO1,
	IDC_RADIO2,
	IDC_RADIO3
};

BOOL CConfigDlg::OnInitDialog()
{
	BOOL bRet = __super::OnInitDialog();

	if(TRUE == bRet)
	{
		/*char szFile[MAX_PATH];
		if (NULL == GetRunArg("cfgfile") ||
			strlen(GetRunArg("cfgfile")) == 0)
		{
			sprintf(szFile, "%s\\conf\\cfg.ini", GetRootPath());
		}
		else
		{
			sprintf(szFile, "%s\\conf\\%s", GetRootPath(), GetRunArg("cfgfile"));
		}*/
		const char* szFile = RunArgGetConfigFile();
		//	load config
		int nValue = ::GetPrivateProfileInt("SETTING", "GENELITEMONS", 0, szFile);
		if(0 != nValue)
		{
			CButton *pButton = (CButton *)GetDlgItem(IDC_CHECK1);
			pButton->SetCheck(1);
		}

		nValue = ::GetPrivateProfileInt("SETTING", "AUTOKICKDOOROBSTACLE", 0, szFile);
		if(0 != nValue)
		{
			CButton *pButton = (CButton *)GetDlgItem(IDC_CHECK2);
			pButton->SetCheck(1);
		}

		nValue = ::GetPrivateProfileInt("SETTING", "USINGOLDENGINE", 0, szFile);
		if(0 != nValue)
		{
			CButton *pButton = (CButton *)GetDlgItem(IDC_CHECK3);
			pButton->SetCheck(1);
		}

		int nDiff = GameWorld::GetInstance().GetDifficultyLevel();
		if(nDiff != kDifficultyDefault)
		{
			//	disable all
			for(int i = 0 ; i < sizeof(s_nDifficultyRadioBoxIds) / sizeof(s_nDifficultyRadioBoxIds[0]); ++i)
			{
				CButton *pButton = (CButton *)GetDlgItem(s_nDifficultyRadioBoxIds[i]);
				pButton->EnableWindow(FALSE);
			}
			if(nDiff >= kDifficultyNovice &&
				nDiff <= kDifficultyNormal)
			{
				CButton *pButton = (CButton *)GetDlgItem(s_nDifficultyRadioBoxIds[nDiff - kDifficultyNovice]);
				if(pButton)
				{
					pButton->SetCheck(1);
				}
			}
			m_bShowDifficultySelect = false;
		}
		else
		{
			CButton *pButton = (CButton *)GetDlgItem(IDC_RADIO3);
			pButton->SetCheck(1);
		}
	}

	return bRet;
}

void CConfigDlg::OnOK()
{
	/*char szFile[MAX_PATH];
	if (NULL == GetRunArg("cfgfile") ||
		strlen(GetRunArg("cfgfile")) == 0)
	{
		sprintf(szFile, "%s\\conf\\cfg.ini", GetRootPath());
	}
	else
	{
		sprintf(szFile, "%s\\conf\\%s", GetRootPath(), GetRunArg("cfgfile"));
	}*/
	const char* szFile = RunArgGetConfigFile();
	//	write config
	CButton *pButton = (CButton *)GetDlgItem(IDC_CHECK1);
	if(pButton->GetCheck() != 0)
	{
		m_bEnableElite = true;
	}

	pButton = (CButton *)GetDlgItem(IDC_CHECK2);
	if(pButton->GetCheck() != 0)
	{
		m_bAutoKickDoorObstacle = true;
	}

	pButton = (CButton *)GetDlgItem(IDC_CHECK3);
	if(pButton->GetCheck() != 0)
	{
		m_bUsingOldEngine = true;
	}

	char szValue[10] = {0};
	WritePrivateProfileString("SETTING", "GENELITEMONS", itoa(m_bEnableElite ? 1 : 0, szValue, 10), szFile);
	WritePrivateProfileString("SETTING", "AUTOKICKDOOROBSTACLE", itoa(m_bAutoKickDoorObstacle ? 1 : 0, szValue, 10), szFile);
	WritePrivateProfileString("SETTING", "USINGOLDENGINE", itoa(m_bUsingOldEngine ? 1 : 0, szValue, 10), szFile);

	//	get difficulty setting
	if(m_bShowDifficultySelect)
	{
		int nSelectIndex = -1;
		for(int i = 0 ; i < sizeof(s_nDifficultyRadioBoxIds) / sizeof(s_nDifficultyRadioBoxIds[0]); ++i)
		{
			CButton *pButton = (CButton *)GetDlgItem(s_nDifficultyRadioBoxIds[i]);
			if(pButton->GetCheck() != 0)
			{
				nSelectIndex = i;
				break;
			}
		}

		//	dispatch event
		MSG msgQuery;
		msgQuery.message = WM_SETDIFFICULTY;
		msgQuery.wParam = (WPARAM)(kDifficultyNovice + nSelectIndex);
		msgQuery.lParam = 0;

		GameWorld::GetInstance().PostRunMessage(&msgQuery);
	}

	__super::OnOK();
}
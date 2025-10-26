#ifndef METIN2_TOOL_WORLDEDITOR_UTIL_HPP
#define METIN2_TOOL_WORLDEDITOR_UTIL_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "DockingBar/PageCtrl.h"
#define WORLD_EDITOR

#include <eterGrnLib/ModelInstance.h>

METIN2_BEGIN_NS

#ifndef VK_0
#define VK_0				0x30
#define VK_1				0x31
#define VK_2				0x32
#define VK_3				0x33
#define VK_4				0x34
#define VK_5				0x35
#define VK_6				0x36
#define VK_7				0x37
#define VK_8				0x38
#define VK_9				0x39
#endif

#ifndef VK_A
#define VK_A				0x41
#define VK_B				0x42
#define VK_C				0x43
#define VK_D				0x44
#define VK_E				0x45
#define VK_F				0x46
#define VK_G				0x47
#define VK_H				0x48
#define VK_I				0x49
#define VK_J				0x4A
#define VK_K				0x4B
#define VK_L				0x4C
#define VK_M				0x4D
#define VK_N				0x4E
#define VK_O				0x4F
#define VK_P				0x50
#define VK_Q				0x51
#define VK_R				0x52
#define VK_S				0x53
#define VK_T				0x54
#define VK_U				0x55
#define VK_V				0x56
#define VK_W				0x57
#define VK_X				0x58
#define VK_Y			    0x59
#define VK_Z				0x5A
#endif

void CreateBitmapButton(CButton * pButton, int iBitmapID, CBitmap & rBitmap);
void RepositioningWindow(CPageCtrl & rPageCtrl, int ixTemporarySpace, int iyTemporarySpace);
void CreateHighColorImageList(int iBitmapID, CImageList * pImageList);

void SetDialogFloatText(HWND hWnd, int iID, float fData);
void SetDialogIntegerText(HWND hWnd, int iID, int iData);
float GetDialogFloatText(HWND hWnd, int iID);
int GetDialogIntegerText(HWND hWnd, int iID);
void SelectComboBoxItem(CComboBox & rComboBox, uint32_t dwIndex);

extern char g_szProgramPath[PROGRAM_PATH_LENGTH+1];
extern char g_szProgramWindowPath[PROGRAM_PATH_LENGTH+1];

extern const char * GetProgramExcutingPath(const char * c_szFullFileName);

///////////////////////////////////////////////////////////////////////////////////////////////////

class CTileInstance : public CGrannyModelInstance
{
	public:
		CTileInstance(){}
		~CTileInstance(){}

		void LoadTexture();
		void RenderTile();

	protected:
		CGraphicImageInstance m_ImageInstance;
};

class COpenFileDialog
{
	public:
		COpenFileDialog(CWnd * pParentWnd = NULL, uint32_t dwFlag = 0, const char * c_pszTitle = NULL, const char * c_pszFilter = NULL, const char * c_pszDefaultPath = NULL, const char * c_pfirstName = NULL) :
			m_pWnd(pParentWnd),
			m_dwFlag(dwFlag),
			mc_pszTitle(c_pszTitle),
			mc_pszDefaultPath(c_pszDefaultPath)
		{
			strncpy(m_szFilter, c_pszFilter, MAX_PATH);

			char * p = m_szFilter;
			while ((p = strchr(p, '|')))
				*(p++) = '\0';

			CWinApp * pApplication = (CWinApp *) AfxGetApp();
			assert(pApplication);

			memset(m_szFileName, 0, sizeof(m_szFileName));
			memset(m_szPathName, 0, sizeof(m_szPathName));
			if (c_pfirstName)
			{
				strcpy(m_szPathName, c_pfirstName);
			}

			memset(&m_ofn, 0, sizeof(OPENFILENAME));

			m_ofn.lStructSize		= sizeof(OPENFILENAME);
			m_ofn.hwndOwner			= m_pWnd->GetSafeHwnd();
			m_ofn.hInstance			= pApplication->m_hInstance;
			m_ofn.lpstrFilter		= m_szFilter;
			m_ofn.nFilterIndex		= 1L;

			m_ofn.lpstrFile			= m_szPathName;
			m_ofn.nMaxFile			= sizeof(m_szPathName);
			m_ofn.lpstrFileTitle	= m_szFileName;
			m_ofn.nMaxFileTitle		= sizeof(m_szFileName);

			m_ofn.Flags				= OFN_PATHMUSTEXIST | m_dwFlag;
			m_ofn.lpstrTitle		= mc_pszTitle;
			m_ofn.lpstrInitialDir	= mc_pszDefaultPath;
			m_ofn.lpstrDefExt		= NULL;
		}

		~COpenFileDialog() {};

		virtual bool DoModal()
		{
			if (!GetOpenFileName(&m_ofn))
				return FALSE;

			return TRUE;
		}

		const char * GetPathName()
		{
			return m_szPathName;
		}

	protected:
		char			m_szPathName[MAX_PATH+1];		// Filename including path and extension
		char			m_szFileName[MAX_PATH+1];		// Filename and extension without path
		char			m_szFilter[MAX_PATH+1];			// File Filter

		CWnd *			m_pWnd;
		uint32_t			m_dwFlag;
		const char *	mc_pszTitle;
		const char *	mc_pszDefaultPath;
		OPENFILENAME	m_ofn;
};

class CSaveFileDialog : public COpenFileDialog
{
	public:
		CSaveFileDialog(CWnd * pParentWnd = NULL, uint32_t dwFlag = 0, const char * c_pszTitle = NULL, const char * c_pszFilter = NULL, const char * c_pszDefaultPath = NULL, const char * c_pfirstName = NULL) :
			COpenFileDialog(pParentWnd, dwFlag, c_pszTitle, c_pszFilter, c_pszDefaultPath,  c_pfirstName)
		{
		}

		virtual bool DoModal()
		{
			if (!GetSaveFileName(&m_ofn))
				return FALSE;

			return TRUE;
		}
};

extern void RenderBackGroundCharacter(float fx, float fy, float fz);
extern void RenderBackGroundTile();
extern void DestroyBackGroundData();

extern std::string RemoveStartString(const std::string & strOriginal, const std::string & strBegin);

METIN2_END_NS

#endif

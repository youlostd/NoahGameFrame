#ifndef METIN2_TOOL_WORLDEDITOR_STDAFX_HPP
#define METIN2_TOOL_WORLDEDITOR_STDAFX_HPP


#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif
#include <cstdint>

typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;
typedef FLOAT               *PFLOAT;

typedef int                 INT;
typedef unsigned int        UINT;
typedef unsigned int        *PUINT;


#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#include <../shared/dxgitype.h> // We need the windows sdk version of dxgitype

#include <afxwin.h>         // MFC core and standard components

//#define _AFXDLL
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
#include <afxpriv.h>    // for CDockContext
#include <afxtempl.h>   // for CArray
#include <atltypes.h>   // for CArray

#pragma warning(disable:4786)	// character 255 넘어가는거 끄기
#pragma warning(disable:4100)	// parameter 안쓰인거 경고 끄기
#pragma warning(disable:4710)	// inline 이 안됐다고 워닝나오는거 끄기

#include <il/il.h>
#define WORLD_EDITOR

#include <EterLib/StdAfx.h>
#include <EterBase/StdAfx.h>
#include <EterGrnLib/StdAfx.h>
#include <GameLib/StdAfx.h>
#include <EffectLib/StdAfx.h>
#include <MilesLib/StdAfx.h>
#include <SpeedTreeLib/StdAfx.h>

#include "Type.h"
#include "Util.h"

// UI
#include "./UI/SimpleUI.h"
#include "./UI/XBrowseForFolder.h"

// Data
#include "./DataCtrl/ActorInstanceAccessor.h"
#include "./DataCtrl/ModelInstanceAccessor.h"
#include "./DataCtrl/EffectAccessor.h"

#include "./DataCtrl/UndoBuffer.h"
#include "./DataCtrl/MapAccessorArea.h"

#include "./DataCtrl/ObjectData.h"

// Control Bar
//#include "./DockingBar/SizeCBar.h"
#include "./DockingBar/PageCtrl.h"
// Control Bar - Object

// Scene
#include "./Scene/PickingArrow.h"
#include "./Scene/SceneBase.h"
#include "./Scene/SceneMap.h"
#include "./Scene/SceneObject.h"
#include "./Scene/SceneEffect.h"
#include "./Scene/SceneFly.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif

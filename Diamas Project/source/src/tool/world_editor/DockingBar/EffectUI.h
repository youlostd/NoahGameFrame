#ifndef METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_EFFECTUI_HPP
#define METIN2_TOOL_WORLDEDITOR_DOCKINGBAR_EFFECTUI_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

METIN2_BEGIN_NS

class CBlendTypeComboBox : public CComboBox
{
// Construction
public:
	CBlendTypeComboBox();
	BOOL Create();

	void SelectBlendType(uint32_t dwIndex);
	int GetBlendType() const;

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTest)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBlendTypeComboBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTest)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

class COperationTypeComboBox : public CComboBox
{
// Construction
public:
	COperationTypeComboBox();
	BOOL Create();

	void SelectOperationType(uint32_t dwIndex);
	int GetOperationType() const;

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTest)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~COperationTypeComboBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTest)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

METIN2_END_NS

#endif

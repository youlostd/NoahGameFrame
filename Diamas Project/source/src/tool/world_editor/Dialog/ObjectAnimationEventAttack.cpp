#include "stdafx.h"
#include "..\WorldEditor.h"
#include "ObjectAnimationEventAttack.h"

METIN2_BEGIN_NS

CObjectAnimationEventAttack::CObjectAnimationEventAttack(CWnd* pParent /*=NULL*/)
	: CObjectAnimationEventBase(CObjectAnimationEventAttack::IDD, pParent)
{
	//{{AFX_DATA_INIT(CObjectAnimationEventAttack)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CObjectAnimationEventAttack::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CObjectAnimationEventAttack)
	DDX_Control(pDX, IDC_OBJECT_ANIMATION_EVENT_ATTACK_TYPE, m_ctrlAttackType);
	DDX_Control(pDX, IDC_OBJECT_ANIMATION_EVENT_HIT_TYPE, m_ctrlHitType);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CObjectAnimationEventAttack, CDialog)
	//{{AFX_MSG_MAP(CObjectAnimationEventAttack)
	ON_BN_CLICKED(IDC_OBJECT_ANIMATION_EVENT_HIT_PROCESS, OnEnableHitProcess)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CObjectAnimationEventBase normal functions

BOOL CObjectAnimationEventAttack::Create(CWnd * pParent, const CRect & c_rRect)
{
	if (!CDialog::Create(CObjectAnimationEventAttack::IDD, (CWnd*)pParent))
		return FALSE;

	SetWindowPos(NULL, c_rRect.left, c_rRect.top, c_rRect.Width(), c_rRect.Height(), TRUE);
	ShowWindow(SW_HIDE);

	m_v3Position = Vector3(0.0f, 0.0f, 0.0f);
	m_fRadius = 0.0f;
	m_dwHitType = 0;
	m_dwAttackType = 0;
	m_fInvisibleType = 0.0f;
	m_fExternalForce = 0.0f;
	m_fDuration = 0.0f;
	m_iHitLimitCount = 0;

	SetDialogFloatText(GetSafeHwnd(), IDC_OBJECT_ANIMATION_EVENT_ATTACK_POSITION_X, 0.0f);
	SetDialogFloatText(GetSafeHwnd(), IDC_OBJECT_ANIMATION_EVENT_ATTACK_POSITION_Y, 0.0f);
	SetDialogFloatText(GetSafeHwnd(), IDC_OBJECT_ANIMATION_EVENT_ATTACK_POSITION_Z, 0.0f);
	SetDialogFloatText(GetSafeHwnd(), IDC_OBJECT_ANIMATION_EVENT_ATTACK_RADIUS, 0.0f);

	m_HitTypeNameMap.insert(make_pair(NRaceData::HIT_TYPE_NONE, std::string("None") ));
	m_HitTypeNameMap.insert(make_pair(NRaceData::HIT_TYPE_GREAT, std::string("Great") ));
	m_HitTypeNameMap.insert(make_pair(NRaceData::HIT_TYPE_GOOD, std::string("Good") ));

	m_ctrlHitType.InsertString(0, "None");
	m_ctrlHitType.InsertString(1, "Great");
	m_ctrlHitType.InsertString(2, "Good");
	m_ctrlHitType.SelectString(0, "Great");

	m_AttackTypeNameMap.insert(make_pair(NRaceData::ATTACK_TYPE_SPLASH, std::string("Splash") ));
	m_AttackTypeNameMap.insert(make_pair(NRaceData::ATTACK_TYPE_SNIPE, std::string("Snipe") ));

	m_ctrlAttackType.InsertString(0, "Splash");
	m_ctrlAttackType.InsertString(1, "Snipe");
	m_ctrlAttackType.SelectString(0, "Splash");

	SetDialogFloatText(GetSafeHwnd(), IDC_OBJECT_ANIMATION_EVENT_ATTACK_INVISIBLE_TIME, 0.0f);
	SetDialogFloatText(GetSafeHwnd(), IDC_OBJECT_ANIMATION_EVENT_ATTACK_EXTERNAL_FORCE, 0.0f);
	SetDialogFloatText(GetSafeHwnd(), IDC_OBJECT_ANIMATION_EVENT_ATTACK_DURATION, 0.0f);
	SetDialogIntegerText(GetSafeHwnd(), IDC_OBJECT_ANIMATION_EVENT_ATTACK_HIT_LIMIT, 0);

	return TRUE;
}

void CObjectAnimationEventAttack::Close()
{
	m_v3Position.x = GetDialogFloatText(GetSafeHwnd(), IDC_OBJECT_ANIMATION_EVENT_ATTACK_POSITION_X);
	m_v3Position.y = GetDialogFloatText(GetSafeHwnd(), IDC_OBJECT_ANIMATION_EVENT_ATTACK_POSITION_Y);
	m_v3Position.z = GetDialogFloatText(GetSafeHwnd(), IDC_OBJECT_ANIMATION_EVENT_ATTACK_POSITION_Z);
	m_fRadius = GetDialogFloatText(GetSafeHwnd(), IDC_OBJECT_ANIMATION_EVENT_ATTACK_RADIUS);
	m_isEnableHitProcess = IsDlgButtonChecked(IDC_OBJECT_ANIMATION_EVENT_HIT_PROCESS);
	m_dwHitType = m_ctrlHitType.GetCurSel();
	m_dwAttackType = m_ctrlAttackType.GetCurSel();
	m_fInvisibleType = GetDialogFloatText(GetSafeHwnd(), IDC_OBJECT_ANIMATION_EVENT_ATTACK_INVISIBLE_TIME);
	m_fExternalForce = GetDialogFloatText(GetSafeHwnd(), IDC_OBJECT_ANIMATION_EVENT_ATTACK_EXTERNAL_FORCE);
	m_fDuration = GetDialogFloatText(GetSafeHwnd(), IDC_OBJECT_ANIMATION_EVENT_ATTACK_DURATION);
	m_iHitLimitCount = GetDialogIntegerText(GetSafeHwnd(), IDC_OBJECT_ANIMATION_EVENT_ATTACK_HIT_LIMIT);
}

void CObjectAnimationEventAttack::GetData(CRaceMotionDataAccessor::TMotionEventData * pData)
{
	CRaceMotionDataAccessor::TMotionAttackingEventData * pAttackData = (CRaceMotionDataAccessor::TMotionAttackingEventData *)pData;
	ZeroMemory(&pAttackData->AttackData, sizeof(pAttackData->AttackData));

	pAttackData->fDurationTime = m_fDuration;
	pAttackData->CollisionData.iCollisionType = NRaceData::COLLISION_TYPE_SPLASH;
	pAttackData->CollisionData.SphereDataVector.clear();
	pAttackData->CollisionData.SphereDataVector.resize(1);
	CSphereCollisionInstance & rInstance = pAttackData->CollisionData.SphereDataVector[0];
	TSphereData & rSphereData = rInstance.GetAttribute();
	rSphereData.v3Position = m_v3Position;
	rSphereData.fRadius = m_fRadius;

	pAttackData->isEnableHitProcess = m_isEnableHitProcess;

	pAttackData->AttackData.iHittingType = m_dwHitType;
	pAttackData->AttackData.iAttackType = m_dwAttackType;
	pAttackData->AttackData.fInvisibleTime = m_fInvisibleType;
	pAttackData->AttackData.fExternalForce = m_fExternalForce;
	pAttackData->AttackData.fStiffenTime = 0.0f;
	pAttackData->AttackData.iHitLimitCount = m_iHitLimitCount;
}

void CObjectAnimationEventAttack::SetData(const CRaceMotionDataAccessor::TMotionEventData * c_pData)
{
	const CRaceMotionDataAccessor::TMotionAttackingEventData * pAttackData = (const CRaceMotionDataAccessor::TMotionAttackingEventData *)c_pData;

	const CSphereCollisionInstance & c_rInstance = pAttackData->CollisionData.SphereDataVector[0];
	const TSphereData & c_rSphereData = c_rInstance.GetAttribute();

	SetDialogFloatText(GetSafeHwnd(), IDC_OBJECT_ANIMATION_EVENT_ATTACK_POSITION_X, c_rSphereData.v3Position.x);
	SetDialogFloatText(GetSafeHwnd(), IDC_OBJECT_ANIMATION_EVENT_ATTACK_POSITION_Y, c_rSphereData.v3Position.y);
	SetDialogFloatText(GetSafeHwnd(), IDC_OBJECT_ANIMATION_EVENT_ATTACK_POSITION_Z, c_rSphereData.v3Position.z);
	SetDialogFloatText(GetSafeHwnd(), IDC_OBJECT_ANIMATION_EVENT_ATTACK_RADIUS, c_rSphereData.fRadius);
	CheckDlgButton(IDC_OBJECT_ANIMATION_EVENT_HIT_PROCESS, pAttackData->isEnableHitProcess);

	if (m_HitTypeNameMap.end() != m_HitTypeNameMap.find(pAttackData->AttackData.iHittingType))
		m_ctrlHitType.SelectString(0, m_HitTypeNameMap[pAttackData->AttackData.iHittingType].c_str());
	if (m_AttackTypeNameMap.end() != m_AttackTypeNameMap.find(pAttackData->AttackData.iAttackType))
		m_ctrlAttackType.SelectString(0, m_AttackTypeNameMap[pAttackData->AttackData.iAttackType].c_str());
	SetDialogFloatText(GetSafeHwnd(), IDC_OBJECT_ANIMATION_EVENT_ATTACK_INVISIBLE_TIME, pAttackData->AttackData.fInvisibleTime);
	SetDialogFloatText(GetSafeHwnd(), IDC_OBJECT_ANIMATION_EVENT_ATTACK_EXTERNAL_FORCE, pAttackData->AttackData.fExternalForce);
	SetDialogFloatText(GetSafeHwnd(), IDC_OBJECT_ANIMATION_EVENT_ATTACK_DURATION, pAttackData->fDurationTime);
	SetDialogIntegerText(GetSafeHwnd(), IDC_OBJECT_ANIMATION_EVENT_ATTACK_HIT_LIMIT, pAttackData->AttackData.iHitLimitCount);
}

/////////////////////////////////////////////////////////////////////////////
// CObjectAnimationEventAttack message handlers

void CObjectAnimationEventAttack::OnOK()
{
}

void CObjectAnimationEventAttack::OnCancel()
{
}

void CObjectAnimationEventAttack::OnEnableHitProcess()
{
	m_isEnableHitProcess = IsDlgButtonChecked(IDC_OBJECT_ANIMATION_EVENT_HIT_PROCESS);
}

METIN2_END_NS

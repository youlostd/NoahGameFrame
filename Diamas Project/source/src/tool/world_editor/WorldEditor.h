#ifndef METIN2_TOOL_WORLDEDITOR_WORLDEDITOR_HPP
#define METIN2_TOOL_WORLDEDITOR_WORLDEDITOR_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "DataCtrl/ObjectData.h"
#include "DataCtrl/MapManagerAccessor.h"
#include "resource.h"

#include <GameLib/FlyingObjectManager.h>
#include <GameLib/GameEventManager.h>
#include <MilesLib/SoundManager.h>
#include <EffectLib/EffectManager.h>
#include <EterLib/GrpDevice.h>
#include <EterLib/GrpLightManager.h>
#include <EterLib/ResourceManager.h>
#include <EterLib/FontManager.hpp>
#include <EterLib/CullingManager.h>
#include <EterBase/StepTimer.h>
#include <EterGrnLib/GrannyState.hpp>
#include <GameLib/ClientConfig.h>

#include <pak/PakManager.hpp>


#include <storm/memory/NewAllocator.hpp>

METIN2_BEGIN_NS

class InitializationContext
{
	public:
		InitializationContext();

	protected:
		storm::NewAllocator m_allocator;


};

class CWorldEditorApp : public CWinApp, InitializationContext
{
	public:
		CWorldEditorApp();
		~CWorldEditorApp();

		void OpenLogs();

		CObjectData* GetObjectData();
		CEffectAccessor* GetEffectAccessor();
		CMapManagerAccessor* GetMapManagerAccessor();
		CSceneObject* GetSceneObject();
		CSceneEffect* GetSceneEffect();
		CSceneMap* GetSceneMap();
		CSceneFly* GetSceneFly();

		CGraphicDevice& GetGraphicDevice() { return m_GraphicDevice; }

		virtual BOOL InitInstance();
		virtual int ExitInstance();
		virtual BOOL OnIdle(LONG lCount);

		afx_msg void OnAppAbout();
		DECLARE_MESSAGE_MAP()

	protected:
		CClientConfig m_config;
		PakManager m_vfs;

		DX::StepTimer m_timer;
		CResourceManager m_resManager;
		FontManager m_fontManager;
		GrannyState m_grnState;

		CGraphicDevice m_GraphicDevice;
		CSoundManager m_SoundManager;
		CEffectManager m_EffectManager;
		CFlyingManager m_FlyingManager;
		CLightManager m_LightManager;
		CGameEventManager m_GameEventManager;

		CCullingManager m_CullingManager;

		uint32_t m_nextProcessTimestamp;
};

METIN2_END_NS

#endif

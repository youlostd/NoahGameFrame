#include "StdAfx.h"
#include "MapManagerAccessor.h"
#include "MapAccessorOutdoor.h"

METIN2_BEGIN_NS

void CMapManagerAccessor::BackupObject()
{
	m_pMapAccessor->BackupObject();
}

void CMapManagerAccessor::BackupObjectCurrent()
{
	m_pMapAccessor->BackupObjectCurrent();
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void CMapManagerAccessor::BackupTerrain()
{
	m_pMapAccessor->BackupTerrain();
}

void CMapManagerAccessor::BackupTerrainCurrent()
{
	m_pMapAccessor->BackupTerrainCurrent();
}

METIN2_END_NS

#include "StdAfx.h"
#include "ObjectData.h"

#include <SpdLog.hpp>

METIN2_BEGIN_NS

BOOL CObjectData::LoadModelScript(const char * c_szFileName)
{
	CTextFileLoader TextFileLoader;
	if (!TextFileLoader.Load(c_szFileName))
		return FALSE;

	TextFileLoader.SetTop();

	std::string strScriptType;
	if (!TextFileLoader.GetTokenString("scripttype", &strScriptType))
		return FALSE;

	if (0 == strScriptType.compare("RaceDataScript"))
	{
		if (!LoadRaceData(TextFileLoader))
			return FALSE;
		m_iModelType = MODEL_TYPE_CHARACTER;
	}
	else if (0 == strScriptType.compare("ItemDataScript"))
	{
		if (!LoadItemData(TextFileLoader))
			return FALSE;
		m_iModelType = MODEL_TYPE_ITEM;
	}
	else if (0 == strScriptType.compare("ObjectDataScript"))
	{
		if (!LoadObjectData(TextFileLoader))
			return FALSE;
		m_iModelType = MODEL_TYPE_OBJECT;
	}

	/////////////////////////////////////////////////////////////////
	if (!m_strModelName.empty())
		SetModelThing(m_strModelName.c_str());
	/////////////////////////////////////////////////////////////////

	return TRUE;
}

BOOL CObjectData::LoadRaceData(CTextFileLoader & rTextFileLoader)
{
	rTextFileLoader.GetTokenString("basemodelfilename", &m_strModelName);

	if (rTextFileLoader.SetChildNode("shapedata"))
	{
		m_strShapeDataPathName = "";
		rTextFileLoader.GetTokenString("pathname", &m_strShapeDataPathName);

		uint32_t dwShapeDataCount;
		if (!rTextFileLoader.GetTokenDoubleWord("shapedatacount", &dwShapeDataCount))
			return FALSE;

		m_ShapeData.clear();

		for (uint32_t i = 0; i < dwShapeDataCount; ++i)
		{
			if (!rTextFileLoader.SetChildNode("shapedata", i))
				return FALSE;

			TShapeData ShapeData;

			if (!rTextFileLoader.GetTokenDoubleWord("shapeindex", &ShapeData.dwIndex))
				return FALSE;
			if (!rTextFileLoader.GetTokenString("model", &ShapeData.strModelFileName))
				return FALSE;
			rTextFileLoader.GetTokenString("sourceskin", &ShapeData.strSourceSkinFileName);
			rTextFileLoader.GetTokenString("targetskin", &ShapeData.strTargetSkinFileName);

			m_ShapeData.push_back(ShapeData);

			rTextFileLoader.SetParentNode();
		}

		rTextFileLoader.SetParentNode();
	}

	if (rTextFileLoader.SetChildNode("attachingdata"))
	{
		if (!NRaceData::LoadAttachingData(rTextFileLoader, &m_AttachingDataVector))
			return FALSE;

		rTextFileLoader.SetParentNode();
	}

	return TRUE;
}

BOOL CObjectData::LoadItemData(CTextFileLoader & rTextFileLoader)
{
	rTextFileLoader.GetTokenDoubleWord("type", &m_dwEquipmentType);
	rTextFileLoader.GetTokenString("modelfilename", &m_strModelName);
	rTextFileLoader.GetTokenString("iconimagefilename", &m_strEquipmentIconImageFileName);
	rTextFileLoader.GetTokenString("dropmodelfilename", &m_strEquipmentDropModelFileName);

	if (rTextFileLoader.SetChildNode("attachingdata"))
	{
		if (!NRaceData::LoadAttachingData(rTextFileLoader, &m_AttachingDataVector))
			return FALSE;

		rTextFileLoader.SetParentNode();
	}

	return TRUE;
}

BOOL CObjectData::LoadObjectData(CTextFileLoader & rTextFileLoader)
{
	rTextFileLoader.GetTokenString("modelfilename", &m_strModelName);

	if (rTextFileLoader.SetChildNode("attachingdata"))
	{
		if (!NRaceData::LoadAttachingData(rTextFileLoader, &m_AttachingDataVector))
			return FALSE;

		rTextFileLoader.SetParentNode();
	}

	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CObjectData::SaveModelScript(const char * c_szFileName)
{
	switch (m_iModelType)
	{
		case MODEL_TYPE_CHARACTER:
			if (!SaveRaceData(c_szFileName))
				return FALSE;
			break;
		case MODEL_TYPE_ITEM:
			if (!SaveItemData(c_szFileName))
				return FALSE;
			break;
		case MODEL_TYPE_OBJECT:
			if (!SaveObjectData(c_szFileName))
				return FALSE;
			break;
	}
	return TRUE;
}

BOOL CObjectData::SaveRaceData(const char * c_szFileName)
{
	storm::File File;

	bsys::error_code ec;
	File.Open(c_szFileName, ec,
	          storm::AccessMode::kWrite,
	          storm::CreationDisposition::kCreateAlways,
	          storm::ShareMode::kNone,
	          storm::UsageHint::kSequential);

	if (ec) {
		spdlog::error("Failed to open {0} for writing with {1}",
		          c_szFileName, ec);
		return FALSE;
	}

	PrintfTabs(File, 0, "ScriptType            RaceDataScript\n");
	PrintfTabs(File, 0, "\n");

	PrintfTabs(File, 0, "BaseModelFileName     \"%s\"\n", m_strModelName.c_str());
	PrintfTabs(File, 0, "\n");

	// Shape Data
	PrintfTabs(File, 0, "Group ShapeData\n");
	PrintfTabs(File, 0, "{\n");
	PrintfTabs(File, 1, "PathName \t\t \"%s\"\n", m_strShapeDataPathName.c_str());
	PrintfTabs(File, 1, "ShapeDataCount \t\t %d\n", m_ShapeData.size());
	for (size_t i = 0; i < m_ShapeData.size(); ++i)
	{
		TShapeData & rShapeData = m_ShapeData[i];

		PrintfTabs(File, 1, "ShapeData%02d\n", i);
		PrintfTabs(File, 1, "{\n");
		PrintfTabs(File, 2, "ShapeIndex \t\t %d\n", rShapeData.dwIndex);
		PrintfTabs(File, 2, "Model \t\t \"%s\"\n", rShapeData.strModelFileName.c_str());
		if (!rShapeData.strSourceSkinFileName.empty())
		if (!rShapeData.strTargetSkinFileName.empty())
		{
			PrintfTabs(File, 2, "SourceSkin \t\t \"%s\"\n", rShapeData.strSourceSkinFileName.c_str());
			PrintfTabs(File, 2, "TargetSkin \t\t \"%s\"\n", rShapeData.strTargetSkinFileName.c_str());
		}
		PrintfTabs(File, 1, "}\n");
	}
	PrintfTabs(File, 0, "}\n");

	// Attaching Data
	PrintfTabs(File, 0, "Group AttachingData\n");
	PrintfTabs(File, 0, "{\n");
	NRaceData::SaveAttachingData(File, 1, m_AttachingDataVector);
	PrintfTabs(File, 0, "}\n");
	return TRUE;
}

BOOL CObjectData::SaveItemData(const char * c_szFileName)
{

	storm::File File;

	bsys::error_code ec;
	File.Open(c_szFileName, ec,
	          storm::AccessMode::kWrite,
	          storm::CreationDisposition::kCreateAlways,
	          storm::ShareMode::kNone,
	          storm::UsageHint::kSequential);

	if (ec) {
		spdlog::error("Failed to open {0} for writing with {1}",
		          c_szFileName, ec);
		return FALSE;
	}

	PrintfTabs(File, 0, "ScriptType            ItemDataScript\n");
	PrintfTabs(File, 0, "\n");

	PrintfTabs(File, 0, "Type                  %d\n", m_dwEquipmentType);
	PrintfTabs(File, 0, "\n");

	PrintfTabs(File, 0, "ModelFileName         \"%s\"\n", m_strModelName.c_str());
	PrintfTabs(File, 0, "IconImageFileName     \"%s\"\n", m_strEquipmentIconImageFileName.c_str());
	PrintfTabs(File, 0, "DropModelFileName     \"%s\"\n", m_strEquipmentDropModelFileName.c_str());
	PrintfTabs(File, 0, "\n");

	PrintfTabs(File, 0, "Group AttachingData\n");
	PrintfTabs(File, 0, "{\n");
	NRaceData::SaveAttachingData(File, 1, m_AttachingDataVector);
	PrintfTabs(File, 0, "}\n");
	return TRUE;
}

BOOL CObjectData::SaveObjectData(const char* c_szFileName)
{

	storm::File File;

	bsys::error_code ec;
	File.Open(c_szFileName, ec,
	          storm::AccessMode::kWrite,
	          storm::CreationDisposition::kCreateAlways,
	          storm::ShareMode::kNone,
	          storm::UsageHint::kSequential);

	if (ec) {
		spdlog::error("Failed to open {0} for writing with {1}",
		          c_szFileName, ec);
		return FALSE;
	}

	PrintfTabs(File, 0, "ScriptType            ObjectDataScript\n");

	PrintfTabs(File, 0, "ModelFileName         \"%s\"\n", m_strModelName.c_str());

	PrintfTabs(File, 0, "Group AttachingData\n");
	PrintfTabs(File, 0, "{\n");
	NRaceData::SaveAttachingData(File, 1, m_AttachingDataVector);
	PrintfTabs(File, 0, "}\n");
	return TRUE;
}

METIN2_END_NS

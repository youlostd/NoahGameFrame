#include "StdAfx.h"
#include "../eterLib/ResourceManager.h"

#include "RaceData.h"
#include "RaceMotionData.h"
#include "../EffectLib/EffectManager.h"
#include <base/GroupTextTree.hpp>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CRaceData::LoadRaceData(const char *c_szFileName)
{
    CTextFileLoader TextFileLoader;
    if (!TextFileLoader.Load(c_szFileName))
        return false;

    TextFileLoader.SetTop();

    TextFileLoader.GetTokenString("basemodelfilename", &m_strBaseModelFileName);
    TextFileLoader.GetTokenString("treefilename", &m_strTreeFileName);
    TextFileLoader.GetTokenString("attributefilename", &m_strAttributeFileName);
    TextFileLoader.GetTokenString("smokebonename", &m_strSmokeBoneName);
    TextFileLoader.GetTokenString("motionlistfilename", &m_strMotionListFileName);

    if (!m_strTreeFileName.empty())
        StringPath(m_strTreeFileName);

    CTokenVector *pSmokeTokenVector;
    if (TextFileLoader.GetTokenVector("smokefilename", &pSmokeTokenVector))
    {
        if (pSmokeTokenVector->size() % 2 != 0)
        {
            SPDLOG_ERROR("SmokeFileName ArgCount[{0}]%%2==0", pSmokeTokenVector->size());
            return false;
        }

        UINT uLineCount = pSmokeTokenVector->size() / 2;

        for (UINT uLine = 0; uLine < uLineCount; ++uLine)
        {
            int eSmoke = atoi(pSmokeTokenVector->at(uLine * 2 + 0).c_str());
            if (eSmoke < 0 || eSmoke >= SMOKE_NUM)
            {
                SPDLOG_ERROR("SmokeFileName SmokeNum[{0}] OUT OF RANGE", eSmoke);
                return false;
            }

            const auto &c_rstrEffectFileName = pSmokeTokenVector->at(uLine * 2 + 1);

            auto &rdwCRCEft = m_adwSmokeEffectID[eSmoke];
            if (!CEffectManager::Instance().RegisterEffect(c_rstrEffectFileName.c_str(), &rdwCRCEft))
            {
                SPDLOG_ERROR("CRaceData::RegisterEffect({0}) ERROR", c_rstrEffectFileName.c_str());
                rdwCRCEft = 0;
                return false;
            }
        }
    }

    if (TextFileLoader.SetChildNode("shapedata"))
    {
        std::string strPathName;
        uint32_t dwShapeDataCount = 0;
        if (TextFileLoader.GetTokenString("pathname", &strPathName) &&
            TextFileLoader.GetTokenDoubleWord("shapedatacount", &dwShapeDataCount))
        {
            for (uint32_t i = 0; i < dwShapeDataCount; ++i)
            {
                if (!TextFileLoader.SetChildNode("shapedata", i))
                {
                    continue;
                }

                /////////////////////////
                // Temporary - 이벤트를 위한 임시 기능
                TextFileLoader.GetTokenString("specialpath", &strPathName);
                /////////////////////////

                uint32_t dwShapeIndex;
                if (!TextFileLoader.GetTokenDoubleWord("shapeindex", &dwShapeIndex))
                {
                    continue;
                }

                // LOCAL_PATH_SUPPORT
                std::string strModel;
                if (TextFileLoader.GetTokenString("model", &strModel))
                {
                    SetShapeModel(dwShapeIndex, (strPathName + strModel).c_str());
                }
                else
                {
                    if (!TextFileLoader.GetTokenString("local_model", &strModel))
                        continue;

                    SetShapeModel(dwShapeIndex, strModel.c_str());
                }
                // END_OF_LOCAL_PATH_SUPPORT

                auto &shape = m_kMap_dwShapeKey_kShape[dwShapeIndex];

                TextFileLoader.GetTokenFloat("specular", &shape.specular);

                NRaceData::LoadAttachingData(TextFileLoader, &shape.m_attachingData);

                std::string strSourceSkin;
                std::string strTargetSkin;

                // LOCAL_PATH_SUPPORT
                if (TextFileLoader.GetTokenString("local_sourceskin", &strSourceSkin) &&
                    TextFileLoader.GetTokenString("local_targetskin", &strTargetSkin))
                {
                    AppendShapeSkin(dwShapeIndex, 0, strSourceSkin.c_str(), strTargetSkin.c_str());
                }
                // END_OF_LOCAL_PATH_SUPPORT

#define METIN2_SHAPESKIN(num) do { \
	if (TextFileLoader.GetTokenString("sourceskin" num, &strSourceSkin) && \
		TextFileLoader.GetTokenString("targetskin" num, &strTargetSkin)) \
	{ \
		AppendShapeSkin(dwShapeIndex, 0, (strPathName + strSourceSkin).c_str(), (strPathName + strTargetSkin).c_str()); \
	} \
} while(false);
                METIN2_SHAPESKIN("");
                METIN2_SHAPESKIN("2");
                METIN2_SHAPESKIN("3");
                METIN2_SHAPESKIN("4");
                METIN2_SHAPESKIN("5");
                METIN2_SHAPESKIN("6");
#undef METIN2_SHAPESKIN
                TextFileLoader.SetParentNode();
            }
        }

        TextFileLoader.SetParentNode();
    }

    if (TextFileLoader.SetChildNode("hairdata"))
    {
        std::string strPathName;
        uint32_t dwHairDataCount = 0;
        if (TextFileLoader.GetTokenString("pathname", &strPathName) &&
            TextFileLoader.GetTokenDoubleWord("hairdatacount", &dwHairDataCount))
        {
            for (uint32_t i = 0; i < dwHairDataCount; ++i)
            {
                if (!TextFileLoader.SetChildNode("hairdata", i))
                {
                    continue;
                }

                /////////////////////////
                // Temporary - 이벤트를 위한 임시 기능
                TextFileLoader.GetTokenString("specialpath", &strPathName);
                /////////////////////////

                uint32_t dwShapeIndex;
                if (!TextFileLoader.GetTokenDoubleWord("hairindex", &dwShapeIndex))
                {
                    continue;
                }

                std::string strModel;
                std::string strSourceSkin;
                std::string strTargetSkin;
                if (TextFileLoader.GetTokenString("model", &strModel) &&
                    TextFileLoader.GetTokenString("sourceskin", &strSourceSkin) &&
                    TextFileLoader.GetTokenString("targetskin", &strTargetSkin))
                {
                    SetHairSkin(dwShapeIndex, 0, (strPathName + strModel).c_str(),
                                (strPathName + strSourceSkin).c_str(), (strPathName + strTargetSkin).c_str());
                } else if(TextFileLoader.GetTokenString("model", &strModel)) {
                    SetHairModel(dwShapeIndex, (strPathName + strModel).c_str());
                }

                TextFileLoader.SetParentNode();
            }
        }

        TextFileLoader.SetParentNode();
    }

    if (TextFileLoader.SetChildNode("attachingdata"))
    {
        if (!NRaceData::LoadAttachingData(TextFileLoader, &m_AttachingDataVector))
            return false;

        TextFileLoader.SetParentNode();
    }

    return true;
}

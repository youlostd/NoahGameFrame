#ifndef METIN2_CLIENT_MAIN_PYTHONQUEST_H
#define METIN2_CLIENT_MAIN_PYTHONQUEST_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

class CPythonQuest : public CSingleton<CPythonQuest>
{
public:
    struct SQuestInstance
    {
        SQuestInstance()
        {
            dwIndex = 0;
            iClockValue = 0;
            iCounterValue = 0;
            iStartTime = 0;
        }

        uint32_t dwIndex;
        uint32_t c_index;

        std::string strIconFileName;
        std::string strTitle;

        std::string strClockName;
        std::string strCounterName;

        int iClockValue;
        int iCounterValue;

        int iStartTime;
    };

    typedef std::vector<SQuestInstance> TQuestInstanceContainer;

public:
    CPythonQuest();
    virtual ~CPythonQuest();

    void Clear();

    void RegisterQuestInstance(const SQuestInstance &c_rQuestInstance);
    void DeleteQuestInstance(uint32_t dwIndex);

    bool IsQuest(uint32_t dwIndex);
    void MakeQuest(uint32_t dwIndex, uint32_t c_index);

    void SetQuestTitle(uint32_t dwIndex, const char *c_szTitle);
    void SetQuestClockName(uint32_t dwIndex, const char *c_szClockName);
    void SetQuestCounterName(uint32_t dwIndex, const char *c_szCounterName);
    void SetQuestClockValue(uint32_t dwIndex, int iClockValue);
    void SetQuestCounterValue(uint32_t dwIndex, int iCounterValue);
    void SetQuestIconFileName(uint32_t dwIndex, const char *c_szIconFileName);

    int GetQuestCount();
    bool GetQuestInstancePtr(uint32_t dwArrayIndex, SQuestInstance **ppQuestInstance);

protected:
    void __Initialize();
    bool __GetQuestInstancePtr(uint32_t dwQuestIndex, SQuestInstance **ppQuestInstance);

protected:
    TQuestInstanceContainer m_QuestInstanceContainer;
};
#endif /* METIN2_CLIENT_MAIN_PYTHONQUEST_H */

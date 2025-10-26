#include "StdAfx.h"
#include "PythonQuest.h"
#include "../EterLib/ResourceManager.h"
#include "../eterBase/Timer.h"

void CPythonQuest::RegisterQuestInstance(const SQuestInstance &c_rQuestInstance)
{
    DeleteQuestInstance(c_rQuestInstance.dwIndex);
    m_QuestInstanceContainer.push_back(c_rQuestInstance);

    /////

    SQuestInstance &rQuestInstance = *m_QuestInstanceContainer.rbegin();
    rQuestInstance.iStartTime = ELTimer_GetMSec() / 1000;
}

struct FQuestInstanceCompare
{
    uint32_t dwSearchIndex;

    FQuestInstanceCompare(uint32_t dwIndex)
        : dwSearchIndex(dwIndex)
    {
    }

    bool operator ()(const CPythonQuest::SQuestInstance &rQuestInstance)
    {
        return dwSearchIndex == rQuestInstance.dwIndex;
    }
};

void CPythonQuest::DeleteQuestInstance(uint32_t dwIndex)
{
    TQuestInstanceContainer::iterator itor = std::find_if(m_QuestInstanceContainer.begin(),
                                                          m_QuestInstanceContainer.end(),
                                                          FQuestInstanceCompare(dwIndex));
    if (itor == m_QuestInstanceContainer.end())
        return;

    m_QuestInstanceContainer.erase(itor);
}

bool CPythonQuest::IsQuest(uint32_t dwIndex)
{
    TQuestInstanceContainer::iterator itor = std::find_if(m_QuestInstanceContainer.begin(),
                                                          m_QuestInstanceContainer.end(),
                                                          FQuestInstanceCompare(dwIndex));
    return itor != m_QuestInstanceContainer.end();
}

void CPythonQuest::MakeQuest(uint32_t dwIndex, uint32_t c_index)
{
    DeleteQuestInstance(dwIndex);
    m_QuestInstanceContainer.push_back(SQuestInstance());

    /////

    SQuestInstance &rQuestInstance = *m_QuestInstanceContainer.rbegin();
    rQuestInstance.dwIndex = dwIndex;
    rQuestInstance.c_index = c_index;
    rQuestInstance.iStartTime = ELTimer_GetMSec() / 1000;
}

void CPythonQuest::SetQuestTitle(uint32_t dwIndex, const char *c_szTitle)
{
    SQuestInstance *pQuestInstance;
    if (!__GetQuestInstancePtr(dwIndex, &pQuestInstance))
        return;

    pQuestInstance->strTitle = c_szTitle;
}

void CPythonQuest::SetQuestClockName(uint32_t dwIndex, const char *c_szClockName)
{
    SQuestInstance *pQuestInstance;
    if (!__GetQuestInstancePtr(dwIndex, &pQuestInstance))
        return;

    pQuestInstance->strClockName = c_szClockName;
}

void CPythonQuest::SetQuestCounterName(uint32_t dwIndex, const char *c_szCounterName)
{
    SQuestInstance *pQuestInstance;
    if (!__GetQuestInstancePtr(dwIndex, &pQuestInstance))
        return;

    pQuestInstance->strCounterName = c_szCounterName;
}

void CPythonQuest::SetQuestClockValue(uint32_t dwIndex, int iClockValue)
{
    SQuestInstance *pQuestInstance;
    if (!__GetQuestInstancePtr(dwIndex, &pQuestInstance))
        return;

    pQuestInstance->iClockValue = iClockValue;
    pQuestInstance->iStartTime = ELTimer_GetMSec() / 1000;
}

void CPythonQuest::SetQuestCounterValue(uint32_t dwIndex, int iCounterValue)
{
    SQuestInstance *pQuestInstance;
    if (!__GetQuestInstancePtr(dwIndex, &pQuestInstance))
        return;

    pQuestInstance->iCounterValue = iCounterValue;
}

void CPythonQuest::SetQuestIconFileName(uint32_t dwIndex, const char *c_szIconFileName)
{
    SQuestInstance *pQuestInstance;
    if (!__GetQuestInstancePtr(dwIndex, &pQuestInstance))
        return;

    pQuestInstance->strIconFileName = c_szIconFileName;
}

int CPythonQuest::GetQuestCount()
{
    return m_QuestInstanceContainer.size();
}

bool CPythonQuest::GetQuestInstancePtr(uint32_t dwArrayIndex, SQuestInstance **ppQuestInstance)
{
    if (dwArrayIndex >= m_QuestInstanceContainer.size())
        return false;

    *ppQuestInstance = &m_QuestInstanceContainer[dwArrayIndex];

    return true;
}

bool CPythonQuest::__GetQuestInstancePtr(uint32_t dwQuestIndex, SQuestInstance **ppQuestInstance)
{
    TQuestInstanceContainer::iterator itor = std::find_if(m_QuestInstanceContainer.begin(),
                                                          m_QuestInstanceContainer.end(),
                                                          FQuestInstanceCompare(dwQuestIndex));
    if (itor == m_QuestInstanceContainer.end())
        return false;

    *ppQuestInstance = &(*itor);

    return true;
}

void CPythonQuest::__Initialize()
{
    /*
    #ifdef _DEBUG
        for (int i = 0; i < 7; ++i)
        {
            SQuestInstance test;
            test.dwIndex = i;
            test.strIconFileName = "";
            test.strTitle = _getf("test%d", i);
            test.strClockName = "남은 시간";
            test.strCounterName = "남은 마리수";
            test.iClockValue = 1000;
            test.iCounterValue = 1000;
            test.iStartTime = 0;
            RegisterQuestInstance(test);
        }
    #endif
    */
}

void CPythonQuest::Clear()
{
    m_QuestInstanceContainer.clear();
}

CPythonQuest::CPythonQuest()
{
    __Initialize();
}

CPythonQuest::~CPythonQuest()
{
    Clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

PyObject *questGetQuestCount(PyObject *poSelf, PyObject *poArgs)
{
    return Py_BuildValue("i", CPythonQuest::Instance().GetQuestCount());
}

PyObject *questGetQuestData(PyObject *poSelf, PyObject *poArgs)
{
    int iIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
        return Py_BadArgument();

    CPythonQuest::SQuestInstance *pQuestInstance;
    if (!CPythonQuest::Instance().GetQuestInstancePtr(iIndex, &pQuestInstance))
        return Py_BuildException("Failed to find quest by index %d", iIndex);

    std::string strIconFileName;
    if (!pQuestInstance->strIconFileName.empty())
    {
        strIconFileName = "d:/ymir work/ui/game/quest/questicon/";
        strIconFileName += pQuestInstance->strIconFileName;
    }
    else
    {
        strIconFileName = "season1/icon/scroll_open.tga";
    }

    //지정되지 않은것은 시스템 퀘스트로 간주
    if (pQuestInstance->c_index == 99)
        pQuestInstance->c_index = 6;

    return Py_BuildValue("isissi", pQuestInstance->dwIndex,
                         pQuestInstance->strTitle.c_str(),
                         pQuestInstance->c_index,
                         strIconFileName.c_str(),
                         pQuestInstance->strCounterName.c_str(),
                         pQuestInstance->iCounterValue);
}

PyObject *questGetQuestIndex(PyObject *poSelf, PyObject *poArgs)
{
    int iIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
        return Py_BadArgument();

    CPythonQuest::SQuestInstance *pQuestInstance;
    if (!CPythonQuest::Instance().GetQuestInstancePtr(iIndex, &pQuestInstance))
        return Py_BuildException("Failed to find quest by index %d", iIndex);

    return Py_BuildValue("i", pQuestInstance->dwIndex);
}

PyObject *questGetQuestLastTime(PyObject *poSelf, PyObject *poArgs)
{
    int iIndex;
    if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
        return Py_BadArgument();

    CPythonQuest::SQuestInstance *pQuestInstance;
    if (!CPythonQuest::Instance().GetQuestInstancePtr(iIndex, &pQuestInstance))
        return Py_BuildException("Failed to find quest by index %d", iIndex);

    int iLastTime = 0;

    if (pQuestInstance->iClockValue >= 0)
    {
        iLastTime = (pQuestInstance->iStartTime + pQuestInstance->iClockValue) - ELTimer_GetMSec() / 1000;
    }

    // 시간 증가 처리 코드
    //	else
    //	{
    //		iLastTime = ELTimer_GetMSec() / 1000 - pQuestInstance->iStartTime;
    //	}

    return Py_BuildValue("si", pQuestInstance->strClockName.c_str(), iLastTime);
}

PyObject *questClear(PyObject *poSelf, PyObject *poArgs)
{
    CPythonQuest::Instance().Clear();
    Py_RETURN_NONE;
}

extern "C" void initquest()
{
    static PyMethodDef s_methods[] =
    {
        {"GetQuestCount", questGetQuestCount, METH_VARARGS},
        {"GetQuestData", questGetQuestData, METH_VARARGS},
        {"GetQuestIndex", questGetQuestIndex, METH_VARARGS},
        {"GetQuestLastTime", questGetQuestLastTime, METH_VARARGS},
        {"Clear", questClear, METH_VARARGS},
        {NULL, NULL, NULL},
    };

    PyObject *poModule = Py_InitModule("quest", s_methods);
    PyModule_AddIntConstant(poModule, "QUEST_MAX_NUM", 5);
    PyModule_AddIntConstant(poModule, "QUEST_CATEGORY_MAX_NUM", 7);
}

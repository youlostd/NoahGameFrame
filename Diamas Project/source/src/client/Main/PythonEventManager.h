#ifndef METIN2_CLIENT_MAIN_PYTHONEVENTMANAGER_H
#define METIN2_CLIENT_MAIN_PYTHONEVENTMANAGER_H

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include "../eterLib/parser.h"

#include <string>
#include <map>

class CPythonEventManager : public CSingleton<CPythonEventManager>
{
  public:
    typedef struct STextLine
    {
        STextLine() {}

        STextLine(const STextLine &) = delete;

        STextLine(STextLine &&o)
            : ixLocal(o.ixLocal), iyLocal(o.iyLocal), centered(o.centered), instance(std::move(o.instance))
        {
            // ctor
        }

        // If |centered| is true, ixLocal specifies the center
        // position of the text line
        int ixLocal, iyLocal;
        bool centered;
        std::unique_ptr<CGraphicTextInstance> instance;
    } TTextLine;

    typedef std::list<TTextLine> TScriptTextLineList;
    typedef std::unordered_map<int, std::string> TEventAnswerMap;

    enum
    {
        EVENT_POSITION_START = 0,
        EVENT_POSITION_END = 1,

        BOX_VISIBLE_LINE_COUNT = 10,
    };

    enum EButtonType
    {
        BUTTON_TYPE_NEXT,
        BUTTON_TYPE_DONE,
        BUTTON_TYPE_CANCEL,
    };

    enum EEventType
    {
        EVENT_TYPE_LETTER,
        EVENT_TYPE_COLOR,
        EVENT_TYPE_DELAY,
        EVENT_TYPE_ENTER,
        EVENT_TYPE_WAIT,
        EVENT_TYPE_CLEAR,
        EVENT_TYPE_QUESTION,
        EVENT_TYPE_NEXT,
        EVENT_TYPE_DONE,

        EVENT_TYPE_LEFT_IMAGE,
        EVENT_TYPE_TOP_IMAGE,
        EVENT_TYPE_BACKGROUND_IMAGE,
        EVENT_TYPE_IMAGE,

        EVENT_TYPE_ADD_MAP_SIGNAL,
        EVENT_TYPE_CLEAR_MAP_SIGNAL,
        EVENT_TYPE_SET_MESSAGE_POSITION,
        EVENT_TYPE_ADJUST_MESSAGE_POSITION,
        EVENT_TYPE_SET_CENTER_MAP_POSITION,

        EVENT_TYPE_QUEST_BUTTON,

        // HIDE_QUEST_LETTER
        EVENT_TYPE_QUEST_BUTTON_CLOSE,
        // END_OF_HIDE_QUEST_LETTER

        EVENT_TYPE_SLEEP,
        EVENT_TYPE_SET_CAMERA,
        EVENT_TYPE_BLEND_CAMERA,
        EVENT_TYPE_RESTORE_CAMERA,
        EVENT_TYPE_FADE_OUT,
        EVENT_TYPE_FADE_IN,
        EVENT_TYPE_WHITE_OUT,
        EVENT_TYPE_WHITE_IN,
        EVENT_TYPE_CLEAR_TEXT,
        EVENT_TYPE_TEXT_HORIZONTAL_ALIGN_CENTER,
        EVENT_TYPE_TITLE_IMAGE,

        EVENT_TYPE_RUN_CINEMA,
        EVENT_TYPE_DUNGEON_RESULT,

        EVENT_TYPE_ITEM_NAME,
        EVENT_TYPE_MONSTER_NAME,

        EVENT_TYPE_COLOR256,
        EVENT_TYPE_WINDOW_SIZE,

        EVENT_TYPE_INPUT,
        EVENT_TYPE_LONG_INPUT,
        EVENT_TYPE_CONFIRM_WAIT,
        EVENT_TYPE_END_CONFIRM_WAIT,

        EVENT_TYPE_INSERT_IMAGE,

        EVENT_TYPE_SELECT_ITEM,
        EVENT_TYPE_INSERT_IMAGE_TOOLTIP,
        EVENT_TYPE_INPUT_ADD_CANCEL,

        EVENT_TYPE_VIDEO,
    };

  private:
    std::map<std::string, int> EventTypeMap;

  public:
    typedef struct SEventSet
    {
        int ix, iy;
        int iWidth;
        int iyLocal;

        // State
        bool isLock;

        uint32_t lastUpdateTime;
        int64_t lLastDelayTime;

        int iCurrentLetter;

        DirectX::SimpleMath::Color CurrentColor;
        std::string strCurrentLine;

        TTextLine currentLine;
        TScriptTextLineList ScriptTextLineList;

        bool isConfirmWait;
        CGraphicTextInstance *pConfirmTimeTextLine;
        int iConfirmEndTime;

        // Group Data
        ScriptGroup ScriptGroup;

        // Static Data
        char szFileName[32 + 1];

        int iTotalLineCount;
        int iVisibleStartLine;
        int iVisibleLineCount;

        int iAdjustLine;

        long lWaitingTime;
        int iRestrictedCharacterCount;

        int nAnswer;
        int qIndex;

        bool isWaitFlag;
        bool centerTextLines;

        PyObject *poEventHandler;

        bool isQuestInfo;

        SEventSet() {}

        virtual ~SEventSet() {}
    } TEventSet;

    typedef std::vector<TEventSet *> TEventSetVector;

  public:
    CPythonEventManager();
    virtual ~CPythonEventManager();

    void Destroy();

    int RegisterEventSet(const std::string &c_szFileName);
    int RegisterEventSetFromString(const std::string &strScript, bool bIsQuestInfo = false);
    void ClearEventSeti(int iIndex);
    void __ClearEventSetp(TEventSet *pEventSet);

    void SetEventHandler(int iIndex, PyObject *poEventHandler);
    void SetRestrictedCount(int iIndex, int iCount);

    int GetEventSetLocalYPosition(int iIndex);
    void AddEventSetLocalYPosition(int iIndex, int iAddValue);
    void InsertText(int iIndex, const char *c_szText, int iX_pos = 0);

    void UpdateEventSet(int iIndex, int ix, int iy);
    void RenderEventSet(int iIndex);
    void SetEventSetWidth(int iIndex, int iWidth);
    void SetFontColor(int iIndex, float r, float g, float b);

    void Skip(int iIndex);
    bool IsWait(int iIndex);
    void EndEventProcess(int iIndex);

    void SelectAnswer(int iIndex, int iAnswer);
    void SetVisibleStartLine(int iIndex, int iStartLine);
    int GetVisibleStartLine(int iIndex);
    int GetLineCount(int iIndex);
    int GetTotalLineCount(int iIndex);
    void SetVisibleLineCount(int iIndex, int iLineCount);

    void SetInterfaceWindow(PyObject *poInterface, PyObject *from = nullptr);
    void SetLeftTimeString(const char *c_szString);

  protected:
    void __InitEventSet(TEventSet &rEventSet);
    void __InsertLine(TEventSet &rEventSet, bool isCenter = false);
    void __AddSpace(TEventSet &pEventSet, int iSpace);

    uint32_t GetEmptyEventSetSlot();

    bool CheckEventSetIndex(int iIndex) const;

    void ProcessEventSet(TEventSet *pEventSet);

    void ClearLine(TEventSet *pEventSet);
    bool GetScriptEventIndex(const char *c_szName, int *pEventPosition, int *pEventType);

    void MakeQuestion(TEventSet *pEventSet, ScriptGroup::TArgList &rArgumentList);
    void MakeNextButton(TEventSet *pEventSet, int iButtonType);
    void RenderTextLine(TEventSet &set, TTextLine &line);

  protected:
    TEventSetVector m_EventSetVector;
    bool m_isQuestConfirmWait;

  private:
    CDynamicPool<TEventSet> m_EventSetPool;
    CDynamicPool<CGraphicTextInstance> m_ScriptTextLinePool;
    PyObject *m_poInterface;
    std::string m_strLeftTimeString;
};
#endif /* METIN2_CLIENT_MAIN_PYTHONEVENTMANAGER_H */

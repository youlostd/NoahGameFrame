#include "StdAfx.h"
#include "PythonTextTail.h"
#include "PythonCharacterManager.h"

PyObject *textTailClear(PyObject *poSelf, PyObject *poArgs)
{
    CPythonTextTail::Instance().Clear();
    Py_RETURN_NONE;
}

PyObject *textTailUpdateAllTextTail(PyObject *poSelf, PyObject *poArgs)
{
    CPythonTextTail::Instance().UpdateAllTextTail();
    Py_RETURN_NONE;
}

PyObject *textTailUpdateShowingTextTail(PyObject *poSelf, PyObject *poArgs)
{
    CPythonTextTail::Instance().UpdateShowingTextTail();
    Py_RETURN_NONE;
}

PyObject *textTailRender(PyObject *poSelf, PyObject *poArgs)
{
    CPythonTextTail::Instance().Render();
    Py_RETURN_NONE;
}

PyObject *textTailRegisterCharacterTextTail(PyObject *poSelf, PyObject *poArgs)
{
    int iGuildID;
    if (!PyTuple_GetInteger(poArgs, 0, &iGuildID))
        return Py_BuildException();
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 1, &iVirtualID))
        return Py_BuildException();

    CPythonTextTail::Instance().RegisterCharacterTextTail(iGuildID, iVirtualID, DirectX::SimpleMath::Color(1.0f, 1.0f, 1.0f, 1.0f));

    Py_RETURN_NONE;
}

PyObject *textTailGetPosition(PyObject *poSelf, PyObject *poArgs)
{
    int VirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &VirtualID))
        return Py_BuildException();

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    bool isData = CPythonTextTail::Instance().GetTextTailPosition(VirtualID, &x, &y, &z);
    if (!isData)
    {
        CPythonCharacterManager &rkChrMgr = CPythonCharacterManager::Instance();
        CInstanceBase *pkInstMain = rkChrMgr.GetMainInstancePtr();
        if (pkInstMain)
        {
            const Vector3 &c_rv3Position = pkInstMain->GetGraphicThingInstanceRef().GetPosition();
            CPythonGraphic::Instance().ProjectPosition(c_rv3Position.x, c_rv3Position.y, c_rv3Position.z, &x, &y);
        }
    }

    return Py_BuildValue("fff", x, y, z);
}

PyObject *textTailIsChat(PyObject *poSelf, PyObject *poArgs)
{
    int VirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &VirtualID))
        return Py_BuildException();

    return Py_BuildValue("i", CPythonTextTail::Instance().IsChatTextTail(VirtualID));
}

PyObject *textTailRegisterChatTail(PyObject *poSelf, PyObject *poArgs)
{
    int VirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &VirtualID))
        return Py_BuildException();

    std::string szText;
    if (!PyTuple_GetString(poArgs, 1, szText))
        return Py_BuildException();

    CPythonTextTail::Instance().RegisterChatTail(VirtualID, szText.c_str());

    Py_RETURN_NONE;
}

PyObject *textTailRegisterInfoTail(PyObject *poSelf, PyObject *poArgs)
{
    int VirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &VirtualID))
        return Py_BuildException();

    std::string szText;
    if (!PyTuple_GetString(poArgs, 1, szText))
        return Py_BuildException();

    CPythonTextTail::Instance().RegisterInfoTail(VirtualID, szText.c_str());

    Py_RETURN_NONE;
}

PyObject *textTailAttachTitle(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BuildException();
    std::string szName;
    if (!PyTuple_GetString(poArgs, 1, szName))
        return Py_BuildException();
    float fr;
    if (!PyTuple_GetFloat(poArgs, 2, &fr))
        return Py_BuildException();
    float fg;
    if (!PyTuple_GetFloat(poArgs, 3, &fg))
        return Py_BuildException();
    float fb;
    if (!PyTuple_GetFloat(poArgs, 4, &fb))
        return Py_BuildException();

    CPythonTextTail::Instance().AttachTitle(iVirtualID, szName.c_str(), DirectX::SimpleMath::Color(fr, fg, fb, 1.0f));

    Py_RETURN_NONE;
}

PyObject *textTailShowCharacterTextTail(PyObject *poSelf, PyObject *poArgs)
{
    int VirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &VirtualID))
        return Py_BuildException();

    CPythonTextTail::Instance().ShowCharacterTextTail(VirtualID);
    Py_RETURN_NONE;
}

PyObject *textTailShowItemTextTail(PyObject *poSelf, PyObject *poArgs)
{
    int VirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &VirtualID))
        return Py_BuildException();

    CPythonTextTail::Instance().ShowItemTextTail(VirtualID);
    Py_RETURN_NONE;
}

PyObject *textTailArrangeTextTail(PyObject *poSelf, PyObject *poArgs)
{
    CPythonTextTail::Instance().ArrangeTextTail();
    Py_RETURN_NONE;
}

PyObject *textTailHideAllTextTail(PyObject *poSelf, PyObject *poArgs)
{
    CPythonTextTail::Instance().HideAllTextTail();
    Py_RETURN_NONE;
}

PyObject *textTailShowAllTextTail(PyObject *poSelf, PyObject *poArgs)
{
    CPythonTextTail::Instance().ShowAllTextTail();
    Py_RETURN_NONE;
}

PyObject *textTailShowAllPcTextTail(PyObject *poSelf, PyObject *poArgs)
{
    CPythonTextTail::Instance().ShowAllPCTextTail();
    Py_RETURN_NONE;
}

PyObject *textTailShowAllNPCTextTail(PyObject *poSelf, PyObject *poArgs)
{
    CPythonTextTail::Instance().ShowAllNPCTextTail();
    Py_RETURN_NONE;
}

PyObject *textTailShowAllMonsterTextTail(PyObject *poSelf, PyObject *poArgs)
{
    CPythonTextTail::Instance().ShowAllMonsterTextTail();
    Py_RETURN_NONE;
}

PyObject *textTailShowAllItemTextTail(PyObject *poSelf, PyObject *poArgs)
{
    CPythonTextTail::Instance().ShowAllItemTextTail();
    Py_RETURN_NONE;
}

PyObject *textTailPick(PyObject *poSelf, PyObject *poArgs)
{
    int ix;
    if (!PyTuple_GetInteger(poArgs, 0, &ix))
        return Py_BuildException();
    int iy;
    if (!PyTuple_GetInteger(poArgs, 1, &iy))
        return Py_BuildException();

    int iValue = CPythonTextTail::Instance().Pick(ix, iy);
    return Py_BuildValue("i", iValue);
}

PyObject *textTailSelectItemName(PyObject *poSelf, PyObject *poArgs)
{
    int iVirtualID;
    if (!PyTuple_GetInteger(poArgs, 0, &iVirtualID))
        return Py_BuildException();

    CPythonTextTail::Instance().SelectItemName(iVirtualID);
    Py_RETURN_NONE;
}

PyObject *textTailEnablePKTitle(PyObject *poSelf, PyObject *poArgs)
{
    int iFlag;
    if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
        return Py_BuildException();

    CPythonTextTail::Instance().EnablePKTitle(iFlag);
    Py_RETURN_NONE;
}

extern "C" void inittextTail()
{
    static PyMethodDef s_methods[] =
    {
        {"Clear", textTailClear, METH_VARARGS},

        {"UpdateAllTextTail", textTailUpdateAllTextTail, METH_VARARGS},
        {"UpdateShowingTextTail", textTailUpdateShowingTextTail, METH_VARARGS},
        {"Render", textTailRender, METH_VARARGS},

        {"ShowCharacterTextTail", textTailShowCharacterTextTail, METH_VARARGS},
        {"ShowItemTextTail", textTailShowItemTextTail, METH_VARARGS},

        {"GetPosition", textTailGetPosition, METH_VARARGS},
        {"IsChat", textTailIsChat, METH_VARARGS},

        {"ArrangeTextTail", textTailArrangeTextTail, METH_VARARGS},
        {"HideAllTextTail", textTailHideAllTextTail, METH_VARARGS},
        {"ShowAllTextTail", textTailShowAllTextTail, METH_VARARGS},
        {"ShowAllPCTextTail", textTailShowAllPcTextTail, METH_VARARGS},
        {"ShowAllNPCTextTail", textTailShowAllNPCTextTail, METH_VARARGS},
        {"ShowAllMonsterTextTail", textTailShowAllMonsterTextTail, METH_VARARGS},
        {"ShowAllItemTextTail", textTailShowAllItemTextTail, METH_VARARGS},

        {"Pick", textTailPick, METH_VARARGS},
        {"SelectItemName", textTailSelectItemName, METH_VARARGS},

        {"EnablePKTitle", textTailEnablePKTitle, METH_VARARGS},

        // For Test
        {"RegisterCharacterTextTail", textTailRegisterCharacterTextTail, METH_VARARGS},
        {"RegisterChatTail", textTailRegisterChatTail, METH_VARARGS},
        {"RegisterInfoTail", textTailRegisterInfoTail, METH_VARARGS},
        {"AttachTitle", textTailAttachTitle, METH_VARARGS},

        {NULL, NULL, NULL},
    };

    Py_InitModule("textTail", s_methods);
}

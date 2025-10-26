#include "StdAfx.h"
#include "../EterLib/ResourceManager.h"
#include "../ScriptLib/PythonUtils.h"

void ImageCapsuleDestroyer(PyObject *capsule)
{
    auto rawPtr = static_cast<CGraphicImageInstance *>(PyCapsule_GetPointer(capsule, nullptr));
    delete rawPtr;
}

bool PyTuple_GetImageInstance(PyObject *poArgs, int pos, CGraphicImageInstance **ppRetImageInstance)
{
    PyObject *handle;

    if (!PyTuple_GetObject(poArgs, pos, &handle))
        return false;

    if (!handle)
        return false;

    const auto rawPtr = static_cast<CGraphicImageInstance *>(PyCapsule_GetPointer(handle, nullptr));
    *ppRetImageInstance = rawPtr;
    return true;
}

bool PyTuple_GetExpandedImageInstance(PyObject *poArgs, int pos, CGraphicExpandedImageInstance **ppRetImageInstance)
{
    PyObject *handle;

    if (!PyTuple_GetObject(poArgs, pos, &handle))
        return false;

    if (!handle)
        return false;

    if (!((CGraphicImageInstance *)handle)->IsType(CGraphicExpandedImageInstance::Type()))
        return false;

    const auto rawPtr = static_cast<CGraphicExpandedImageInstance *>(PyCapsule_GetPointer(handle, nullptr));
    *ppRetImageInstance = rawPtr;

    return true;
}

PyObject *grpImageGenerate(PyObject *poSelf, PyObject *poArgs)
{
    std::string szFileName;

    if (!PyTuple_GetString(poArgs, 0, szFileName))
        return Py_BadArgument();

    if (szFileName.empty())
        Py_RETURN_NONE;

    auto r = CResourceManager::Instance().LoadResource<CGraphicImage>(szFileName);

    auto pImageInstance = std::make_unique<CGraphicImageInstance>(szFileName);
    pImageInstance->SetImagePointer(r);

    if (pImageInstance->IsEmpty())
        return Py_BuildException("Cannot load image (filename: %s)", szFileName.c_str());

    auto rawPtr = pImageInstance.release();
    const auto capsule = PyCapsule_New(rawPtr, nullptr, ImageCapsuleDestroyer);
    if (!capsule)
    {
        delete rawPtr;
        return Py_BuildException("Cannot load image: capsule failure (filename: %s)", szFileName.c_str());
    }

    return capsule;
}

PyObject *grpImageGenerateExpanded(PyObject *poSelf, PyObject *poArgs)
{
    std::string szFileName;

    if (!PyTuple_GetString(poArgs, 0, szFileName))
        return Py_BadArgument();

    if (szFileName.empty())
        Py_RETURN_NONE;

    auto r = CResourceManager::Instance().LoadResource<CGraphicImage>(szFileName);

    auto pImageInstance = std::make_unique<CGraphicExpandedImageInstance>(szFileName);
    pImageInstance->SetImagePointer(r);

    if (pImageInstance->IsEmpty())
        return Py_BuildException("Cannot load image (filename: %s)", szFileName);

    const auto capsule = PyCapsule_New(pImageInstance.get(), nullptr, ImageCapsuleDestroyer);
    if (!capsule)
    {
        pImageInstance.reset();
        return Py_BuildException("Cannot load image: capsule failure (filename: %s)", szFileName.c_str());
    }

    auto ptr = pImageInstance.release();

    return capsule;
}

PyObject *grpImageSetFileName(PyObject *poSelf, PyObject *poArgs)
{
    CGraphicImageInstance *pImageInstance;
    if (!PyTuple_GetImageInstance(poArgs, 0, &pImageInstance))
        return Py_BadArgument();

    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 1, szFileName))
        return Py_BadArgument();

    auto r = CResourceManager::Instance().LoadResource<CGraphicImage>(szFileName);
    pImageInstance->SetImagePointer(r);

    Py_RETURN_NONE;
}

PyObject *grpImageRender(PyObject *poSelf, PyObject *poArgs)
{
    CGraphicImageInstance *pImageInstance;
    if (!PyTuple_GetImageInstance(poArgs, 0, &pImageInstance))
        return Py_BadArgument();

    pImageInstance->Render();
    Py_RETURN_NONE;
}

PyObject *grpSetImagePosition(PyObject *poSelf, PyObject *poArgs)
{
    CGraphicImageInstance *pImageInstance;
    if (!PyTuple_GetImageInstance(poArgs, 0, &pImageInstance))
        return Py_BadArgument();

    float x;
    if (!PyTuple_GetFloat(poArgs, 1, &x))
        return Py_BadArgument();

    float y;
    if (!PyTuple_GetFloat(poArgs, 2, &y))
        return Py_BadArgument();

    pImageInstance->SetPosition(x, y);
    Py_RETURN_NONE;
}

PyObject *grpSetImageOrigin(PyObject *poSelf, PyObject *poArgs)
{
    CGraphicExpandedImageInstance *pImageInstance;
    if (!PyTuple_GetExpandedImageInstance(poArgs, 0, &pImageInstance))
        return Py_BadArgument();

    float x;
    if (!PyTuple_GetFloat(poArgs, 1, &x))
        return Py_BadArgument();
    float y;
    if (!PyTuple_GetFloat(poArgs, 2, &y))
        return Py_BadArgument();

    pImageInstance->SetOrigin(x, y);
    Py_RETURN_NONE;
}

PyObject *grpSetImageRotation(PyObject *poSelf, PyObject *poArgs)
{
    CGraphicExpandedImageInstance *pImageInstance;
    if (!PyTuple_GetExpandedImageInstance(poArgs, 0, &pImageInstance))
        return Py_BadArgument();

    float Degree;
    if (!PyTuple_GetFloat(poArgs, 1, &Degree))
        return Py_BadArgument();

    pImageInstance->SetRotation(Degree);
    Py_RETURN_NONE;
}

PyObject *grpSetImageScale(PyObject *poSelf, PyObject *poArgs)
{
    CGraphicExpandedImageInstance *pImageInstance;
    if (!PyTuple_GetExpandedImageInstance(poArgs, 0, &pImageInstance))
        return Py_BadArgument();

    float x;
    if (!PyTuple_GetFloat(poArgs, 1, &x))
        return Py_BadArgument();
    float y;
    if (!PyTuple_GetFloat(poArgs, 2, &y))
        return Py_BadArgument();

    pImageInstance->SetScale(x, y);
    Py_RETURN_NONE;
}

PyObject *grpSetRenderingRect(PyObject *poSelf, PyObject *poArgs)
{
    CGraphicExpandedImageInstance *pImageInstance;
    if (!PyTuple_GetExpandedImageInstance(poArgs, 0, &pImageInstance))
        return Py_BadArgument();

    float fLeft;
    if (!PyTuple_GetFloat(poArgs, 1, &fLeft))
        return Py_BadArgument();
    float fTop;
    if (!PyTuple_GetFloat(poArgs, 2, &fTop))
        return Py_BadArgument();
    float fRight;
    if (!PyTuple_GetFloat(poArgs, 3, &fRight))
        return Py_BadArgument();
    float fBottom;
    if (!PyTuple_GetFloat(poArgs, 4, &fBottom))
        return Py_BadArgument();

    pImageInstance->SetRenderingRect(fLeft, fTop, fRight, fBottom);
    Py_RETURN_NONE;
}

PyObject *grpSetImageDiffuseColor(PyObject *poSelf, PyObject *poArgs)
{
    CGraphicImageInstance *pImageInstance;
    if (!PyTuple_GetImageInstance(poArgs, 0, &pImageInstance))
        return Py_BadArgument();

    float r;
    if (!PyTuple_GetFloat(poArgs, 1, &r))
        return Py_BadArgument();
    float g;
    if (!PyTuple_GetFloat(poArgs, 2, &g))
        return Py_BadArgument();
    float b;
    if (!PyTuple_GetFloat(poArgs, 3, &b))
        return Py_BadArgument();
    float a;
    if (!PyTuple_GetFloat(poArgs, 4, &a))
        return Py_BadArgument();

    pImageInstance->SetDiffuseColor(r, g, b, a);
    Py_RETURN_NONE;
}

PyObject *grpGetWidth(PyObject *poSelf, PyObject *poArgs)
{
    CGraphicImageInstance *pImageInstance;

    if (!PyTuple_GetImageInstance(poArgs, 0, &pImageInstance))
        return Py_BadArgument();

    if (pImageInstance->IsEmpty())
        return Py_BuildException("Image is empty");

    return Py_BuildValue("i", pImageInstance->GetWidth());
}

PyObject *grpGetHeight(PyObject *poSelf, PyObject *poArgs)
{
    CGraphicImageInstance *pImageInstance;

    if (!PyTuple_GetImageInstance(poArgs, 0, &pImageInstance))
        return Py_BadArgument();

    if (pImageInstance->IsEmpty())
        return Py_BuildException("Image is empty");

    return Py_BuildValue("i", pImageInstance->GetHeight());
}

extern "C" void initgrpImage()
{
    static PyMethodDef s_methods[] =
    {
        {"Render", grpImageRender, METH_VARARGS},
        {"SetPosition", grpSetImagePosition, METH_VARARGS},
        {"Generate", grpImageGenerate, METH_VARARGS},
        {"GenerateExpanded", grpImageGenerateExpanded, METH_VARARGS},
        {"SetFileName", grpImageSetFileName, METH_VARARGS},
        {"SetOrigin", grpSetImageOrigin, METH_VARARGS},
        {"SetRotation", grpSetImageRotation, METH_VARARGS},
        {"SetScale", grpSetImageScale, METH_VARARGS},
        {"SetRenderingRect", grpSetRenderingRect, METH_VARARGS},
        {"SetDiffuseColor", grpSetImageDiffuseColor, METH_VARARGS},
        {"GetWidth", grpGetWidth, METH_VARARGS},
        {"GetHeight", grpGetHeight, METH_VARARGS},
        {NULL, NULL, NULL},
    };

    Py_InitModule("grpImage", s_methods);
}

#include "StdAfx.h"
#include "../eterLib/Camera.h"
#include "../ScriptLib/PythonUtils.h"

#include <shlobj.h>
#include "../EterLib/CullingManager.h"
#include "PythonGraphic.h"
#include "../EterGrnLib/GrannyState.hpp"
#include "../EterLib/GrpDevice.h"

PyObject *grpCulling(PyObject *poSelf, PyObject *poArgs)
{
    CCullingManager::Instance().Process();
    Py_RETURN_NONE;
}

PyObject *grpInitScreenEffect(PyObject *poSelf, PyObject *poArgs)
{
    CPythonGraphic::Instance().InitScreenEffect();
    Py_RETURN_NONE;
}

PyObject *grpClear(PyObject *poSelf, PyObject *poArgs)
{
    Engine::GetDevice().Clear(true, true);

    Py_RETURN_NONE;
}

PyObject *grpClearDepthBuffer(PyObject *poSelf, PyObject *poArgs)
{
    Engine::GetDevice().Clear(false, true);
    Py_RETURN_NONE;
}

PyObject *grpPushState(PyObject *poSelf, PyObject *poArgs)
{
    CPythonGraphic::Instance().PushState();
    Py_RETURN_NONE;
}

PyObject *grpPopState(PyObject *poSelf, PyObject *poArgs)
{
    CPythonGraphic::Instance().PopState();
    Py_RETURN_NONE;
}

PyObject *grpPushMatrix(PyObject *poSelf, PyObject *poArgs)
{
    CPythonGraphic::Instance().PushMatrix();
    Py_RETURN_NONE;
}

PyObject *grpPopMatrix(PyObject *poSelf, PyObject *poArgs)
{
    CPythonGraphic::Instance().PopMatrix();
    Py_RETURN_NONE;
}

PyObject *grpTranslate(PyObject *poSelf, PyObject *poArgs)
{
    float x;
    if (!PyTuple_GetFloat(poArgs, 0, &x))
    {
        return Py_BuildException("grp.Translate:Wrong X");
    }

    float y;
    if (!PyTuple_GetFloat(poArgs, 1, &y))
    {
        return Py_BuildException("grp.Translate:Wrong Y");
    }

    float z;
    if (!PyTuple_GetFloat(poArgs, 2, &z))
    {
        return Py_BuildException("grp.Translate:Wrong Z");
    }

    CPythonGraphic::Instance().Translate(x, y, z);

    Py_RETURN_NONE;
}

PyObject *grpRotate(PyObject *poSelf, PyObject *poArgs)
{
    float Degree;
    if (!PyTuple_GetFloat(poArgs, 0, &Degree))
    {
        return Py_BuildException();
    }

    float x;
    if (!PyTuple_GetFloat(poArgs, 1, &x))
    {
        return Py_BuildException();
    }

    float y;
    if (!PyTuple_GetFloat(poArgs, 2, &y))
    {
        return Py_BuildException();
    }

    float z;
    if (!PyTuple_GetFloat(poArgs, 3, &z))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().Rotate(Degree, x, y, z);

    Py_RETURN_NONE;
}

PyObject *grpSetColorRenderState(PyObject *poSelf, PyObject *poArgs)
{
    Py_RETURN_NONE;
}

PyObject *grpSetAroundCamera(PyObject *poSelf, PyObject *poArgs)
{
    float distance;
    if (!PyTuple_GetFloat(poArgs, 0, &distance))
    {
        return Py_BuildException();
    }

    float pitch;
    if (!PyTuple_GetFloat(poArgs, 1, &pitch))
    {
        return Py_BuildException();
    }

    float roll;
    if (!PyTuple_GetFloat(poArgs, 2, &roll))
    {
        return Py_BuildException();
    }

    float lookAtZ;
    if (!PyTuple_GetFloat(poArgs, 3, &lookAtZ))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().SetAroundCamera(distance, pitch, roll, lookAtZ);
    Py_RETURN_NONE;
}

PyObject *grpSetPositionCamera(PyObject *poSelf, PyObject *poArgs)
{
    float fx;
    if (!PyTuple_GetFloat(poArgs, 0, &fx))
    {
        return Py_BuildException();
    }

    float fy;
    if (!PyTuple_GetFloat(poArgs, 1, &fy))
    {
        return Py_BuildException();
    }

    float fz;
    if (!PyTuple_GetFloat(poArgs, 2, &fz))
    {
        return Py_BuildException();
    }

    float distance;
    if (!PyTuple_GetFloat(poArgs, 3, &distance))
    {
        return Py_BuildException();
    }

    float pitch;
    if (!PyTuple_GetFloat(poArgs, 4, &pitch))
    {
        return Py_BuildException();
    }

    float roll;
    if (!PyTuple_GetFloat(poArgs, 5, &roll))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().SetPositionCamera(fx, fy, fz, distance, pitch, roll);
    Py_RETURN_NONE;
}

PyObject *grpSetEyeCamera(PyObject *poSelf, PyObject *poArgs)
{
    float xEye;
    if (!PyTuple_GetFloat(poArgs, 0, &xEye))
    {
        return Py_BuildException();
    }

    float yEye;
    if (!PyTuple_GetFloat(poArgs, 1, &yEye))
    {
        return Py_BuildException();
    }

    float zEye;
    if (!PyTuple_GetFloat(poArgs, 2, &zEye))
    {
        return Py_BuildException();
    }

    float xCenter;
    if (!PyTuple_GetFloat(poArgs, 3, &xCenter))
    {
        return Py_BuildException();
    }

    float yCenter;
    if (!PyTuple_GetFloat(poArgs, 4, &yCenter))
    {
        return Py_BuildException();
    }

    float zCenter;
    if (!PyTuple_GetFloat(poArgs, 5, &zCenter))
    {
        return Py_BuildException();
    }

    float xUp;
    if (!PyTuple_GetFloat(poArgs, 6, &xUp))
    {
        return Py_BuildException();
    }

    float yUp;
    if (!PyTuple_GetFloat(poArgs, 7, &yUp))
    {
        return Py_BuildException();
    }

    float zUp;
    if (!PyTuple_GetFloat(poArgs, 8, &zUp))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().SetEyeCamera(xEye, yEye, zEye, xCenter, yCenter, zCenter, xUp, yUp, zUp);
    Py_RETURN_NONE;
}

PyObject *grpSetPerspective(PyObject *poSelf, PyObject *poArgs)
{
    float fov;
    if (!PyTuple_GetFloat(poArgs, 0, &fov))
    {
        return Py_BuildException();
    }

    float aspect;
    if (!PyTuple_GetFloat(poArgs, 1, &aspect))
    {
        return Py_BuildException();
    }

    float zNear;
    if (!PyTuple_GetFloat(poArgs, 2, &zNear))
    {
        return Py_BuildException();
    }

    float zFar;
    if (!PyTuple_GetFloat(poArgs, 3, &zFar))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().SetPerspective(fov, aspect, zNear, zFar);
    Py_RETURN_NONE;
}

PyObject *grpSetOrtho2D(PyObject *poSelf, PyObject *poArgs)
{
    float width;
    if (!PyTuple_GetFloat(poArgs, 0, &width))
    {
        return Py_BuildException();
    }

    float height;
    if (!PyTuple_GetFloat(poArgs, 1, &height))
    {
        return Py_BuildException();
    }

    float length;
    if (!PyTuple_GetFloat(poArgs, 2, &length))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().SetOrtho2D(width, height, length);
    Py_RETURN_NONE;
}

PyObject *grpSetOrtho3D(PyObject *poSelf, PyObject *poArgs)
{
    float width;
    if (!PyTuple_GetFloat(poArgs, 0, &width))
    {
        return Py_BuildException();
    }

    float height;
    if (!PyTuple_GetFloat(poArgs, 1, &height))
    {
        return Py_BuildException();
    }

    float zmin;
    if (!PyTuple_GetFloat(poArgs, 2, &zmin))
    {
        return Py_BuildException();
    }

    float zmax;
    if (!PyTuple_GetFloat(poArgs, 3, &zmax))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().SetOrtho3D(width, height, zmin, zmax);
    Py_RETURN_NONE;
}

PyObject *grpGenerateColor(PyObject *poSelf, PyObject *poArgs)
{
    float r;
    if (!PyTuple_GetFloat(poArgs, 0, &r))
    {
        return Py_BuildException();
    }

    float g;
    if (!PyTuple_GetFloat(poArgs, 1, &g))
    {
        return Py_BuildException();
    }

    float b;
    if (!PyTuple_GetFloat(poArgs, 2, &b))
    {
        return Py_BuildException();
    }

    float a;
    if (!PyTuple_GetFloat(poArgs, 3, &a))
    {
        return Py_BuildException();
    }

    return Py_BuildValue("l", CPythonGraphic::Instance().GenerateColor(r, g, b, a));
}

PyObject *grpSetColor(PyObject *poSelf, PyObject *poArgs)
{
    DWORD color;
    if (!PyTuple_GetUnsignedLong(poArgs, 0, &color))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().SetDiffuseColor(DWORD(color));
    Py_RETURN_NONE;
}

PyObject *grpSetAlpha(PyObject *poSelf, PyObject *poArgs)
{
    float Alpha;
    if (!PyTuple_GetFloat(poArgs, 0, &Alpha))
    {
        return Py_BuildException();
    }

    //CPythonGraphic::Instance().SetAlpha(Alpha);
    Py_RETURN_NONE;
}

PyObject *grpGetCursorPosition3d(PyObject *poSelf, PyObject *poArgs)
{
    float x, y, z;
    CPythonGraphic::Instance().GetCursorPosition(&x, &y, &z);
    return Py_BuildValue("(fff)", x, y, z);
}

PyObject *grpSetCursorPosition(PyObject *poSelf, PyObject *poArgs)
{
    int ix;
    if (!PyTuple_GetInteger(poArgs, 0, &ix))
    {
        return Py_BuildException();
    }
    int iy;
    if (!PyTuple_GetInteger(poArgs, 1, &iy))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().SetCursorPosition(ix, iy);
    Py_RETURN_NONE;
}

PyObject *grpRenderLine(PyObject *poSelf, PyObject *poArgs)
{
    int x;
    if (!PyTuple_GetInteger(poArgs, 0, &x))
    {
        return Py_BuildException();
    }

    int y;
    if (!PyTuple_GetInteger(poArgs, 1, &y))
    {
        return Py_BuildException();
    }

    int width;
    if (!PyTuple_GetInteger(poArgs, 2, &width))
    {
        return Py_BuildException();
    }

    int height;
    if (!PyTuple_GetInteger(poArgs, 3, &height))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().RenderLine2d((float)x, (float)y, (float)x + width, (float)y + height);
    Py_RETURN_NONE;
}

PyObject *grpRenderRoundBox(PyObject *poSelf, PyObject *poArgs)
{
    int x;
    if (!PyTuple_GetInteger(poArgs, 0, &x))
    {
        return Py_BuildException();
    }

    int y;
    if (!PyTuple_GetInteger(poArgs, 1, &y))
    {
        return Py_BuildException();
    }

    int width;
    if (!PyTuple_GetInteger(poArgs, 2, &width))
    {
        return Py_BuildException();
    }

    int height;
    if (!PyTuple_GetInteger(poArgs, 3, &height))
    {
        return Py_BuildException();
    }

    float fx = float(x);
    float fy = float(y);
    float fWidth = float(width);
    float fHeight = float(height);

    CPythonGraphic::Instance().RenderLine2d(fx + 2.0f, fy, fx + 2.0f + (fWidth - 3.0f), fy);
    CPythonGraphic::Instance().RenderLine2d(fx + 2.0f, fy + fHeight, fx + 2.0f + (fWidth - 3.0f), fy + fHeight);
    CPythonGraphic::Instance().RenderLine2d(fx, fy + 2.0f, fx, fy + 2.0f + fHeight - 4.0f);
    CPythonGraphic::Instance().RenderLine2d(fx + fWidth, fy + 1.0f, fx + fWidth, fy + 1.0f + fHeight - 3.0f);
    CPythonGraphic::Instance().RenderLine2d(fx, fy + 2.0f, fx + 2.0f, fy);
    CPythonGraphic::Instance().RenderLine2d(fx, fy + fHeight - 2.0f, fx + 2.0f, fy + fHeight);
    CPythonGraphic::Instance().RenderLine2d(fx + fWidth - 2.0f, fy, fx + fWidth, fy + 2.0f);
    CPythonGraphic::Instance().RenderLine2d(fx + fWidth - 2.0f, fy + fHeight, fx + fWidth, fy + fHeight - 2.0f);
    Py_RETURN_NONE;
}

PyObject *grpRenderBox(PyObject *poSelf, PyObject *poArgs)
{
    int x;
    if (!PyTuple_GetInteger(poArgs, 0, &x))
    {
        return Py_BuildException();
    }

    int y;
    if (!PyTuple_GetInteger(poArgs, 1, &y))
    {
        return Py_BuildException();
    }

    int width;
    if (!PyTuple_GetInteger(poArgs, 2, &width))
    {
        return Py_BuildException();
    }

    int height;
    if (!PyTuple_GetInteger(poArgs, 3, &height))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().RenderBox2d((float)x, (float)y, (float)x + width, (float)y + height);
    Py_RETURN_NONE;
}

PyObject *grpRenderBar(PyObject *poSelf, PyObject *poArgs)
{
    int x;
    if (!PyTuple_GetInteger(poArgs, 0, &x))
    {
        return Py_BuildException();
    }

    int y;
    if (!PyTuple_GetInteger(poArgs, 1, &y))
    {
        return Py_BuildException();
    }

    int width;
    if (!PyTuple_GetInteger(poArgs, 2, &width))
    {
        return Py_BuildException();
    }

    int height;
    if (!PyTuple_GetInteger(poArgs, 3, &height))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().RenderBar2d((float)x, (float)y, (float)x + width, (float)y + height);
    Py_RETURN_NONE;
}

PyObject *grpRenderCube(PyObject *poSelf, PyObject *poArgs)
{
    float sx;
    if (!PyTuple_GetFloat(poArgs, 0, &sx))
    {
        return Py_BuildException();
    }

    float sy;
    if (!PyTuple_GetFloat(poArgs, 1, &sy))
    {
        return Py_BuildException();
    }

    float sz;
    if (!PyTuple_GetFloat(poArgs, 2, &sz))
    {
        return Py_BuildException();
    }

    float ex;
    if (!PyTuple_GetFloat(poArgs, 3, &ex))
    {
        return Py_BuildException();
    }

    float ey;
    if (!PyTuple_GetFloat(poArgs, 4, &ey))
    {
        return Py_BuildException();
    }

    float ez;
    if (!PyTuple_GetFloat(poArgs, 5, &ez))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().RenderCube(sx, sy, sz, ex, ey, ez);
    Py_RETURN_NONE;
}

PyObject *grpRenderBar3d(PyObject *poSelf, PyObject *poArgs)
{
    int sx;
    if (!PyTuple_GetInteger(poArgs, 0, &sx))
    {
        return Py_BuildException();
    }

    int sy;
    if (!PyTuple_GetInteger(poArgs, 1, &sy))
    {
        return Py_BuildException();
    }

    int sz;
    if (!PyTuple_GetInteger(poArgs, 2, &sz))
    {
        return Py_BuildException();
    }

    int ex;
    if (!PyTuple_GetInteger(poArgs, 3, &ex))
    {
        return Py_BuildException();
    }

    int ey;
    if (!PyTuple_GetInteger(poArgs, 4, &ey))
    {
        return Py_BuildException();
    }

    int ez;
    if (!PyTuple_GetInteger(poArgs, 5, &ez))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().RenderBar3d(sx, sy, sz, ex, ey, ez);
    Py_RETURN_NONE;
}

PyObject *grpRenderGradationBar(PyObject *poSelf, PyObject *poArgs)
{
    int x;
    if (!PyTuple_GetInteger(poArgs, 0, &x))
    {
        return Py_BadArgument();
    }

    int y;
    if (!PyTuple_GetInteger(poArgs, 1, &y))
    {
        return Py_BadArgument();
    }

    int width;
    if (!PyTuple_GetInteger(poArgs, 2, &width))
    {
        return Py_BadArgument();
    }

    int height;
    if (!PyTuple_GetInteger(poArgs, 3, &height))
    {
        return Py_BadArgument();
    }

    int iStartColor;
    if (!PyTuple_GetInteger(poArgs, 4, &iStartColor))
    {
        return Py_BadArgument();
    }

    int iEndColor;
    if (!PyTuple_GetInteger(poArgs, 5, &iEndColor))
    {
        return Py_BadArgument();
    }

    CPythonGraphic::Instance().RenderGradationBar2d((float)x, (float)y, (float)x + width, (float)y + height,
                                                    iStartColor, iEndColor);
    Py_RETURN_NONE;
}

PyObject *grpRenderDownButton(PyObject *poSelf, PyObject *poArgs)
{
    int x;
    if (!PyTuple_GetInteger(poArgs, 0, &x))
    {
        return Py_BuildException();
    }

    int y;
    if (!PyTuple_GetInteger(poArgs, 1, &y))
    {
        return Py_BuildException();
    }

    int width;
    if (!PyTuple_GetInteger(poArgs, 2, &width))
    {
        return Py_BuildException();
    }

    int height;
    if (!PyTuple_GetInteger(poArgs, 3, &height))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().RenderDownButton((float)x, (float)y, (float)x + width, (float)y + height);
    Py_RETURN_NONE;
}

PyObject *grpRenderUpButton(PyObject *poSelf, PyObject *poArgs)
{
    int x;
    if (!PyTuple_GetInteger(poArgs, 0, &x))
    {
        return Py_BuildException();
    }

    int y;
    if (!PyTuple_GetInteger(poArgs, 1, &y))
    {
        return Py_BuildException();
    }

    int width;
    if (!PyTuple_GetInteger(poArgs, 2, &width))
    {
        return Py_BuildException();
    }

    int height;
    if (!PyTuple_GetInteger(poArgs, 3, &height))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().RenderUpButton((float)x, (float)y, (float)x + width, (float)y + height);
    Py_RETURN_NONE;
}

PyObject *grpRenderBox3d(PyObject *poSelf, PyObject *poArgs)
{
    float sx;
    if (!PyTuple_GetFloat(poArgs, 0, &sx))
    {
        return Py_BuildException();
    }

    float sy;
    if (!PyTuple_GetFloat(poArgs, 1, &sy))
    {
        return Py_BuildException();
    }

    float sz;
    if (!PyTuple_GetFloat(poArgs, 2, &sz))
    {
        return Py_BuildException();
    }

    float ex;
    if (!PyTuple_GetFloat(poArgs, 3, &ex))
    {
        return Py_BuildException();
    }

    float ey;
    if (!PyTuple_GetFloat(poArgs, 4, &ey))
    {
        return Py_BuildException();
    }

    float ez;
    if (!PyTuple_GetFloat(poArgs, 5, &ez))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().RenderBox3d((float)sx, (float)sy, (float)sz, (float)ex, (float)ey, (float)ez);
    Py_RETURN_NONE;
}

PyObject *grpSetClearColor(PyObject *poSelf, PyObject *poArgs)
{
    float fr;
    if (!PyTuple_GetFloat(poArgs, 0, &fr))
    {
        return Py_BuildException();
    }

    float fg;
    if (!PyTuple_GetFloat(poArgs, 1, &fg))
    {
        return Py_BuildException();
    }

    float fb;
    if (!PyTuple_GetFloat(poArgs, 2, &fb))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().SetClearColor(fr, fg, fb, 1.0f);
    Py_RETURN_NONE;
}

PyObject *grpSetDiffuseColor(PyObject *poSelf, PyObject *poArgs)
{
    float r;
    if (!PyTuple_GetFloat(poArgs, 0, &r))
    {
        return Py_BuildException();
    }

    float g;
    if (!PyTuple_GetFloat(poArgs, 1, &g))
    {
        return Py_BuildException();
    }

    float b;
    if (!PyTuple_GetFloat(poArgs, 2, &b))
    {
        return Py_BuildException();
    }

    float a;
    if (!PyTuple_GetFloat(poArgs, 3, &a))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().SetDiffuseColor(r, g, b, a);
    Py_RETURN_NONE;
}

PyObject *grpIdentity(PyObject *poSelf, PyObject *poArgs)
{
    CPythonGraphic::Instance().Identity();
    Py_RETURN_NONE;
}

PyObject *grpGetAvailableMemory(PyObject *poSelf, PyObject *poArgs)
{
    int iValue = CPythonGraphic::Instance().GetAvailableMemory();
    return Py_BuildValue("i", iValue);
}

// SCREENSHOT_CWDSAVE
PyObject *grpSaveScreenShotToPath(PyObject *poSelf, PyObject *poArgs)
{
    std::string szBasePath;
    if (!PyTuple_GetString(poArgs, 0, szBasePath))
        return Py_BuildException();

    struct tm *tmNow;
    time_t ct;

    ct = time(0);
    tmNow = localtime(&ct);

    std::string path = fmt::format("{}\\{}{}_{}{}{}.png",
                                   szBasePath,
                                   tmNow->tm_mon + 1,
                                   tmNow->tm_mday,
                                   tmNow->tm_hour,
                                   tmNow->tm_min,
                                   tmNow->tm_sec);

    bool bResult = CPythonGraphic::Instance().SaveScreenShot(path.c_str());
    return Py_BuildValue("(is)", bResult, path.c_str());
}

// END_OF_SCREENSHOT_CWDSAVE

PyObject *grpSaveScreenShot(PyObject *poSelf, PyObject *poArgs)
{
    struct tm *tmNow;
    time_t ct;

    ct = time(0);
    tmNow = localtime(&ct);

    char szPath[MAX_PATH + 256];
    SHGetSpecialFolderPath(NULL, szPath, CSIDL_PERSONAL, true);
    strcat(szPath, "\\METIN2\\");

    if (-1 == _access(szPath, 0))
        if (!CreateDirectory(szPath, NULL))
        {
            SPDLOG_ERROR("Failed to create directory [{0}]", szPath);
            return Py_BuildValue("(is)", false, "");
        }

    sprintf(szPath + strlen(szPath), "%02d%02d_%02d%02d%02d.png",
            tmNow->tm_mon + 1,
            tmNow->tm_mday,
            tmNow->tm_hour,
            tmNow->tm_min,
            tmNow->tm_sec);

    bool bResult = CPythonGraphic::Instance().SaveScreenShot(szPath);
    return Py_BuildValue("(is)", bResult, szPath);
}

PyObject *grpSetGamma(PyObject *poSelf, PyObject *poArgs)
{
    float gamma;

    if (!PyTuple_GetFloat(poArgs, 0, &gamma))
    {
        return Py_BuildException();
    }

    CPythonGraphic::Instance().SetGamma(gamma);
    Py_RETURN_NONE;
}

PyObject *grpSetInterfaceRenderState(PyObject *poSelf, PyObject *poArgs)
{
    CPythonGraphic::Instance().SetInterfaceRenderState();
    Py_RETURN_NONE;
}

PyObject *grpSetGameRenderState(PyObject *poSelf, PyObject *poArgs)
{
    CPythonGraphic::Instance().SetGameRenderState();
    Py_RETURN_NONE;
}

PyObject *grpSetViewport(PyObject *poSelf, PyObject *poArgs)
{
    float fx;
    if (!PyTuple_GetFloat(poArgs, 0, &fx))
    {
        return Py_BuildException();
    }
    float fy;
    if (!PyTuple_GetFloat(poArgs, 1, &fy))
    {
        return Py_BuildException();
    }

    float fWidth;
    if (!PyTuple_GetFloat(poArgs, 2, &fWidth))
    {
        return Py_BuildException();
    }
    float fHeight;
    if (!PyTuple_GetFloat(poArgs, 3, &fHeight))
    {
        return Py_BuildException();
    }

    UINT uWidth;
    UINT uHeight;
    CPythonGraphic::Instance().GetBackBufferSize(&uWidth, &uHeight);
    CPythonGraphic::Instance().SetViewport(fx * uWidth, fy * uHeight, fWidth * uWidth, fHeight * uHeight);
    Py_RETURN_NONE;
}

PyObject *grpRestoreViewport(PyObject *poSelf, PyObject *poArgs)
{
    CPythonGraphic::Instance().RestoreViewport();
    Py_RETURN_NONE;
}

PyObject *grpSetOmniLight(PyObject *poSelf, PyObject *poArgs)
{

     D3DLIGHT9 Light;
    Light.Type = D3DLIGHT_SPOT;
    Light.Position = {50.0f, 150.0f, 350.0f};
    Light.Direction = {-0.15f, -0.3f, -0.9f};
    Light.Theta = DirectX::XMConvertToRadians(30.0f);
    Light.Phi = DirectX::XMConvertToRadians(45.0f);
    Light.Falloff = 1.0f;
    Light.Attenuation0 = 0.0f;
    Light.Attenuation1 = 0.005f;
    Light.Attenuation2 = 0.0f;
    Light.Diffuse.r = 1.0f;
    Light.Diffuse.g = 1.0f;
    Light.Diffuse.b = 1.0f;
    Light.Diffuse.a = 1.0f;
    Light.Ambient.r = 1.0f;
    Light.Ambient.g = 1.0f;
    Light.Ambient.b = 1.0f;
    Light.Ambient.a = 1.0f;
    Light.Range = 500.0f;

	    D3DMATERIAL9 Material;
    Material.Ambient = {1.0f, 1.0f, 1.0f, 1.0f};
    Material.Diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
    Material.Emissive = {0.1f, 0.1f, 0.1f, 1.0f};
    STATEMANAGER.SetMaterial(&Material);

    auto *ms = GrannyState::instance().GetModelShader();
    if (ms)
    {
        ms->SetMaterial(Material);
        ms->SetLightDirection(Vector3((const float*)&Light.Direction ));
        ms->SetLightColor(Vector4(Light.Diffuse.r, Light.Diffuse.g, Light.Diffuse.b, Light.Diffuse.a));
        ms->SetMaterial(Material);
        STATEMANAGER.SetMaterial(&Material);
    }
    
   

    CPythonGraphic::Instance().SetOmniLight();
    Py_RETURN_NONE;
}

PyObject *grpGetCameraPosition(PyObject *poSelf, PyObject *poArgs)
{
    Vector3 v3Eye = CCameraManager::Instance().GetCurrentCamera()->GetEye();
    return Py_BuildValue("fff", v3Eye.x, v3Eye.y, v3Eye.z);
}

PyObject *grpGetTargetPosition(PyObject *poSelf, PyObject *poArgs)
{
    Vector3 v3Target = CCameraManager::Instance().GetCurrentCamera()->GetTarget();
    return Py_BuildValue("fff", v3Target.x, v3Target.y, v3Target.z);
}

/*!
 * \brief
 * Render target을 임시 텍스쳐로 변경함.
 */
PyObject *grpBeginRenderInTmpTexture(PyObject *poSelf, PyObject *poArgs)
{
    Py_RETURN_NONE;
}

/*!
 * \briefof
 * Render target을 임시 텍스쳐에서 backbuffer로 돌려놓음.
 */
PyObject *grpEndRenderInTmpTexture(PyObject *poSelf, PyObject *poArgs)
{
    Py_RETURN_NONE;
}

/*!
 * \brief
 * 임시 텍스쳐의 특정 영역을 backbuffer에 그림.
 */
PyObject *grpRenderSubTmpTexture(PyObject *poSelf, PyObject *poArgs)
{
    Py_RETURN_NONE;
}

extern "C" void initgrp()
{
    static PyMethodDef s_methods[] =
    {
        {"InitScreenEffect", grpInitScreenEffect, METH_VARARGS},
        {"Culling", grpCulling, METH_VARARGS},
        {"Clear", grpClear, METH_VARARGS},
        {"ClearDepthBuffer", grpClearDepthBuffer, METH_VARARGS},
        {"Identity", grpIdentity, METH_VARARGS},
        {"GenerateColor", grpGenerateColor, METH_VARARGS},
        {"PopState", grpPopState, METH_VARARGS},
        {"PushState", grpPushState, METH_VARARGS},
        {"Translate", grpTranslate, METH_VARARGS},
        {"Rotate", grpRotate, METH_VARARGS},
        {"SetColorRenderState", grpSetColorRenderState, METH_VARARGS},
        {"SetAroundCamera", grpSetAroundCamera, METH_VARARGS},
        {"SetPositionCamera", grpSetPositionCamera, METH_VARARGS},
        {"SetEyeCamera", grpSetEyeCamera, METH_VARARGS},
        {"SetPerspective", grpSetPerspective, METH_VARARGS},
        {"SetOrtho2d", grpSetOrtho2D, METH_VARARGS},
        {"SetOrtho3d", grpSetOrtho3D, METH_VARARGS},
        {"SetColor", grpSetColor, METH_VARARGS},
        {"SetAlpha", grpSetAlpha, METH_VARARGS},
        {"SetDiffuseColor", grpSetDiffuseColor, METH_VARARGS},
        {"SetClearColor", grpSetClearColor, METH_VARARGS},
        {"GetCursorPosition3d", grpGetCursorPosition3d, METH_VARARGS},
        {"SetCursorPosition", grpSetCursorPosition, METH_VARARGS},
        {"RenderLine", grpRenderLine, METH_VARARGS},
        {"RenderBox", grpRenderBox, METH_VARARGS},
        {"RenderRoundBox", grpRenderRoundBox, METH_VARARGS},
        {"RenderBox3d", grpRenderBox3d, METH_VARARGS},
        {"RenderBar", grpRenderBar, METH_VARARGS},
        {"RenderBar3d", grpRenderBar3d, METH_VARARGS},
        {"RenderGradationBar", grpRenderGradationBar, METH_VARARGS},
        {"RenderCube", grpRenderCube, METH_VARARGS},
        {"RenderDownButton", grpRenderDownButton, METH_VARARGS},
        {"RenderUpButton", grpRenderUpButton, METH_VARARGS},
        {"GetAvailableMemory", grpGetAvailableMemory, METH_VARARGS},
        {"SaveScreenShot", grpSaveScreenShot, METH_VARARGS},
        {"SaveScreenShotToPath", grpSaveScreenShotToPath, METH_VARARGS},
        {"SetGamma", grpSetGamma, METH_VARARGS},
        {"SetInterfaceRenderState", grpSetInterfaceRenderState, METH_VARARGS},
        {"SetGameRenderState", grpSetGameRenderState, METH_VARARGS},
        {"SetViewport", grpSetViewport, METH_VARARGS},
        {"RestoreViewport", grpRestoreViewport, METH_VARARGS},
        {"SetOmniLight", grpSetOmniLight, METH_VARARGS},
        {"GetCameraPosition", grpGetCameraPosition, METH_VARARGS},
        {"GetTargetPosition", grpGetTargetPosition, METH_VARARGS},

        // Clipping을 위한 함수들.
        {"BeginRenderInTmpTexture", grpBeginRenderInTmpTexture, METH_VARARGS},
        {"EndRenderInTmpTexture", grpEndRenderInTmpTexture, METH_VARARGS},
        {"RenderSubTmpTexture", grpRenderSubTmpTexture, METH_VARARGS},

        {NULL, NULL, NULL},
    };

    auto poModule = Py_InitModule("grp", s_methods);
    PyModule_AddIntConstant(poModule, "TEXTBAR_ALIGN_BASELINE", TA_BASELINE);
    PyModule_AddIntConstant(poModule, "TEXTBAR_ALIGN_BOTTOM", TA_BOTTOM);
    PyModule_AddIntConstant(poModule, "TEXTBAR_ALIGN_TOP", TA_TOP);
    PyModule_AddIntConstant(poModule, "TEXTBAR_ALIGN_CENTER", TA_CENTER);
    PyModule_AddIntConstant(poModule, "TEXTBAR_ALIGN_LEFT", TA_LEFT);
    PyModule_AddIntConstant(poModule, "TEXTBAR_ALIGN_RIGHT", TA_RIGHT);
}

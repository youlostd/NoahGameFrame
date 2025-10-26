#include "StdAfx.h"
#include "PythonApplication.h"

PyObject *sndPlaySoundNew(PyObject *poSelf, PyObject *poArgs)
{
    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 0, szFileName))
        return Py_BadArgument();

    CSoundManager &rkSndMgr = CSoundManager::Instance();
    rkSndMgr.PlaySound2D(szFileName);
    Py_RETURN_NONE;
}

PyObject *sndPlaySound3D(PyObject *poSelf, PyObject *poArgs)
{
    float fx;
    if (!PyTuple_GetFloat(poArgs, 0, &fx))
        return Py_BadArgument();
    float fy;
    if (!PyTuple_GetFloat(poArgs, 1, &fy))
        return Py_BadArgument();
    float fz;
    if (!PyTuple_GetFloat(poArgs, 2, &fz))
        return Py_BadArgument();
    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 3, szFileName))
        return Py_BadArgument();

    CSoundManager &rkSndMgr = CSoundManager::Instance();
    rkSndMgr.PlaySound3D(fx, fy, fz, szFileName);
    Py_RETURN_NONE;
}

PyObject *sndFadeInMusic(PyObject *poSelf, PyObject *poArgs)
{
    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 0, szFileName))
        return Py_BadArgument();

    CSoundManager &rkSndMgr = CSoundManager::Instance();
    rkSndMgr.FadeInMusic(szFileName);
    Py_RETURN_NONE;
}

PyObject *sndFadeOutMusic(PyObject *poSelf, PyObject *poArgs)
{
    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 0, szFileName))
        return Py_BadArgument();

    CSoundManager &rkSndMgr = CSoundManager::Instance();
    rkSndMgr.FadeOutMusic(szFileName);
    Py_RETURN_NONE;
}

PyObject *sndFadeOutAllMusic(PyObject *poSelf, PyObject *poArgs)
{
    CSoundManager &rkSndMgr = CSoundManager::Instance();
    rkSndMgr.FadeOutAllMusic();
    Py_RETURN_NONE;
}

PyObject *sndFadeLimitOutMusic(PyObject *poSelf, PyObject *poArgs)
{
    std::string szFileName;
    if (!PyTuple_GetString(poArgs, 0, szFileName))
        return Py_BadArgument();

    float fLimitVolume;
    if (!PyTuple_GetFloat(poArgs, 1, &fLimitVolume))
        return Py_BadArgument();

    CSoundManager &rkSndMgr = CSoundManager::Instance();
    rkSndMgr.FadeLimitOutMusic(szFileName, fLimitVolume);
    Py_RETURN_NONE;
}

PyObject *sndStopAllSound(PyObject *poSelf, PyObject *poArgs)
{
    CSoundManager &rkSndMgr = CSoundManager::Instance();
    rkSndMgr.StopAllSound3D();
    Py_RETURN_NONE;
}

PyObject *sndSetMusicVolume(PyObject *poSelf, PyObject *poArgs)
{
    float fVolume;
    if (!PyTuple_GetFloat(poArgs, 0, &fVolume))
        return Py_BadArgument();

    CSoundManager &rkSndMgr = CSoundManager::Instance();
    rkSndMgr.SetMusicVolume(fVolume);
    Py_RETURN_NONE;
}

PyObject *sndSetSoundVolume(PyObject *poSelf, PyObject *poArgs)
{
    float fVolume;
    if (!PyTuple_GetFloat(poArgs, 0, &fVolume))
        return Py_BadArgument();

    CSoundManager &rkSndMgr = CSoundManager::Instance();
    rkSndMgr.SetSoundVolume(fVolume);
    Py_RETURN_NONE;
}

PyObject *sndSetSoundScale(PyObject *poSelf, PyObject *poArgs)
{
    float fScale;
    if (!PyTuple_GetFloat(poArgs, 0, &fScale))
        return Py_BadArgument();

    CSoundManager &rkSndMgr = CSoundManager::Instance();
    rkSndMgr.SetSoundScale(fScale);
    Py_RETURN_NONE;
}

PyObject *sndSetAmbienceSoundScale(PyObject *poSelf, PyObject *poArgs)
{
    float fScale;
    if (!PyTuple_GetFloat(poArgs, 0, &fScale))
        return Py_BadArgument();

    CSoundManager &rkSndMgr = CSoundManager::Instance();
    rkSndMgr.SetAmbienceSoundScale(fScale);
    Py_RETURN_NONE;
}

extern "C" void initsnd()
{
    static PyMethodDef s_methods[] =
    {
        {"PlaySound", sndPlaySoundNew, METH_VARARGS},
        {"PlaySound3D", sndPlaySound3D, METH_VARARGS},
        {"FadeInMusic", sndFadeInMusic, METH_VARARGS},
        {"FadeOutMusic", sndFadeOutMusic, METH_VARARGS},
        {"FadeOutAllMusic", sndFadeOutAllMusic, METH_VARARGS},
        {"FadeLimitOutMusic", sndFadeLimitOutMusic, METH_VARARGS},
        {"StopAllSound", sndStopAllSound, METH_VARARGS},

        {"SetMusicVolume", sndSetMusicVolume, METH_VARARGS},
        {"SetSoundVolume", sndSetSoundVolume, METH_VARARGS},
        {"SetSoundScale", sndSetSoundScale, METH_VARARGS},
        {"SetAmbienceSoundScale", sndSetAmbienceSoundScale, METH_VARARGS},
        {NULL, NULL, NULL},
    };

    Py_InitModule("snd", s_methods);
}

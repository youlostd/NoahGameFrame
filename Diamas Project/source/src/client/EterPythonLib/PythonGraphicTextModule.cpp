#include "StdAfx.h"

std::vector<std::string> SplitLines(std::string_view input,
                                    uint32_t limit)
{
    std::vector<std::string> lines;

    auto first = input.begin();
    auto last = input.end();
    auto line = first;

    for (uint32_t codePointCount = 0; first != last; ++codePointCount)
    {
        if (codePointCount == limit)
        {
            lines.emplace_back(line, first);

            line = first;
            codePointCount = 0;
        }

        boost::locale::utf::utf_traits<char>::decode(first, last);
    }

    if (line != first)
        lines.emplace_back(line, first);

    return lines;
}

PyObject *grpTextGetLines(PyObject *poSelf, PyObject *poArgs)
{
    std::string szText;
    if (!PyTuple_GetString(poArgs, 0, szText))
        return Py_BadArgument();

    int limit;
    if (!PyTuple_GetInteger(poArgs, 1, &limit))
        return Py_BadArgument();

    const auto lines = SplitLines(szText, limit);

    PyObject *pyLines = PyTuple_New(lines.size());
    for (uint32_t i = 0, s = lines.size(); i < s; ++i)
    {
        const auto &line = lines[i];
        const auto pyLine = PyString_FromStringAndSize(line.data(),
                                                       line.length());
        PyTuple_SET_ITEM(pyLines, i, pyLine);
    }

    return pyLines;
}

extern "C" void initgrpText()
{
    static PyMethodDef s_methods[] =
    {
        {"GetLines", grpTextGetLines, METH_VARARGS},
        {NULL, NULL, NULL},
    };

    Py_InitModule("grpText", s_methods);
}

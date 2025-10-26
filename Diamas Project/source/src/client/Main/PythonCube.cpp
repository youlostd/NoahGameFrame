#include "PythonCube.hpp"

#include <pak/ClientDb.hpp>
#include <pak/Vfs.hpp>
#include <pak/Util.hpp>
#include "PythonBindings.h"

bool CPythonCube::Initialize()
{
    std::vector<CubeProto> v;
    if (!LoadClientDb(GetVfs(), "data/cube_proto", v))
        return false;

    for (const auto &item : v)
    {
        m_cubeProtoByVnum.emplace(item.vnum, item);
        m_cubeProto.emplace_back(item);
    }

    return true;
}

CubeProto *CPythonCube::GetCubeDataByVnum(GenericVnum vnum)
{
    if (const auto it = m_cubeProtoByVnum.find(vnum); it != m_cubeProtoByVnum.end())
        return &it->second;

    return nullptr;
}

const std::vector<CubeProto> &CPythonCube::GetCubeProto() const
{
    return m_cubeProto;
}

PYBIND11_MAKE_OPAQUE(std::vector<CubeProto>);

void init_cube(py::module &m)
{
    py::module cube = m.def_submodule("cube", "Provides cube support");

    py::class_<CubeItem>(cube,
                         "CubeItem")
        .def(py::init<>())
        .def_readwrite("vnum", &CubeItem::vnum)
        .def_readwrite("count", &CubeItem::count);

    py::class_<CubeProto>(cube, "CubeProto")
        .def(py::init<>())
        .def_readwrite("vnum", &CubeProto::vnum)
        .def_readwrite("category", &CubeProto::category)
        .def_readwrite("probability", &CubeProto::probability)
        .def_readwrite("cost", &CubeProto::cost)
        .def_readwrite("items", &CubeProto::items)
        .def_readwrite("reward", &CubeProto::reward)
        .def("__repr__",
             [](const CubeProto &a)
             {
                 return fmt::format("<cube.CubeProto vnum={}, category={}, probability={}, cost={}>", a.vnum,
                                    a.category, a.probability, a.cost);
             }
            );

    py::class_<std::vector<CubeProto>>(m, "CubeProtoVector")
        .def(py::init<>())
        .def("clear", &std::vector<CubeProto>::clear)
        .def("pop_back", &std::vector<CubeProto>::pop_back)
        .def("__len__", [](const std::vector<CubeProto> &v)
        {
            return v.size();
        })
        .def("__iter__", [](std::vector<CubeProto> &v)
        {
            return py::make_iterator(v.begin(), v.end());
        }, py::keep_alive<0, 1>()) /* Keep vector alive while iterator is used */
        ;

    py::class_<CPythonCube, std::unique_ptr<CPythonCube, py::nodelete>>(cube, "cubeMgr")
        .def(py::init([]()
        {
            return std::unique_ptr<CPythonCube, py::nodelete>(CPythonCube::InstancePtr());
        }), pybind11::return_value_policy::reference_internal)

        .def("GetCubeProto", &CPythonCube::GetCubeProto,pybind11::return_value_policy::reference)
        .def("GetCubeDataByVnum", &CPythonCube::GetCubeDataByVnum,  pybind11::return_value_policy::reference);
}

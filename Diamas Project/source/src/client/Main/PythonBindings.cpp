#include "PythonBindings.h"
#include <pybind11/embed.h>

#include "../GameLib/WikiManager.hpp"
#include <game/HuntingManager.hpp>

void init_wiki(py::module &m)
{
    py::module safebox = m.def_submodule("wiki", "Provides wiki data access");

        py::class_<WikiManager, std::unique_ptr<WikiManager, py::nodelete>>(safebox, "wikiInst")
        .def(py::init([]()
        {
            return std::unique_ptr<WikiManager, py::nodelete>(WikiManager::InstancePtr());
        }), pybind11::return_value_policy::reference_internal)
        .def("GetItemWhitelist", &WikiManager::GetItemWhitelist, pybind11::return_value_policy::reference)
        .def("GetMobWhitelist", &WikiManager::GetMobWhitelist, pybind11::return_value_policy::reference)
        .def("GetItemOriginMap", &WikiManager::GetItemOriginMap, pybind11::return_value_policy::reference)
        .def("GetMobOriginMap", &WikiManager::GetMobOriginMap, pybind11::return_value_policy::reference)
        .def("GetBossChests", &WikiManager::GetBossChests, pybind11::return_value_policy::reference)
        .def("GetEventChests", &WikiManager::GetEventChests, pybind11::return_value_policy::reference)
        .def("GetAltChests", &WikiManager::GetAltChests, pybind11::return_value_policy::reference)
        .def("GetCostumeWeapons", &WikiManager::GetCostumeWeapons, pybind11::return_value_policy::reference)
        .def("GetCostumeArmors", &WikiManager::GetCostumeArmors, pybind11::return_value_policy::reference)
        .def("GetCostumeHairs", &WikiManager::GetCostumeHairs, pybind11::return_value_policy::reference)
        .def("GetCostumeWings", &WikiManager::GetCostumeWings, pybind11::return_value_policy::reference)
        .def("GetCostumeShinings", &WikiManager::GetCostumeShinings, pybind11::return_value_policy::reference)

    ;
}
void init_hunting(py::module &m)
{
    py::module hunting = m.def_submodule("hunting", "Provides hunting mission data access");

        py::class_<HuntingManager, std::unique_ptr<HuntingManager, py::nodelete>>(hunting, "huntingInst")
        .def(py::init([]()
        {
            return std::unique_ptr<HuntingManager, py::nodelete>(HuntingManager::InstancePtr());
        }), pybind11::return_value_policy::reference_internal)

        .def("GetMissions", &HuntingManager::GetMissions, pybind11::return_value_policy::reference)
        .def("GetHuntingMissionById", &HuntingManager::GetHuntingMissionById, pybind11::return_value_policy::reference)
        .def("GetHuntingMissionByMob", &HuntingManager::GetHuntingMissionByMob, pybind11::return_value_policy::reference)

    ;
}


PYBIND11_EMBEDDED_MODULE(pygame, m)
{
    init_app(m);
    init_item(m);
    init_safebox(m);
    init_emoji(m);
    init_player(m);
    init_auth(m);
    init_net(m);
    init_shop(m);
    init_guild(m);
    init_npc(m);
    init_cube(m);
    init_exchange(m);
    init_background(m);
    init_fishing(m);
    init_wiki(m);
    init_hunting(m);
    init_privateShopSearch(m);
}

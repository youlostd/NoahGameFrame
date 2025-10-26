#ifndef METIN2_CLIENT_MAIN_PYTHONBINDINGS_H
#define METIN2_CLIENT_MAIN_PYTHONBINDINGS_H
#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/embed.h>

namespace py = pybind11;

void init_app(py::module &);
void init_net(py::module &);
void init_item(py::module &);
void init_emoji(py::module &);
void init_safebox(py::module &);
void init_player(py::module &);
void init_auth(py::module &);
void init_shop(py::module &);
void init_guild(py::module &);
void init_npc(py::module &);
void init_cube(py::module &);
void init_exchange(py::module &);
void init_background(py::module &);
void init_fishing(py::module &m);
void init_privateShopSearch(py::module &m);
#endif /* METIN2_CLIENT_MAIN_PYTHONBINDINGS_H */

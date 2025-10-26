#include "StdAfx.h"
#include "../GameLib/EmojiManager.h"

void init_emoji(py::module &m)
{
    py::module emoji = m.def_submodule("emoji", "Provides emoji replacement support for the game client");

    emoji.def("instance",
              &EmojiManager::instance,
              py::return_value_policy::reference,
              "Get reference to the singleton");

    py::class_<EmojiManager>(emoji, "Manager")
        .def("Register", &EmojiManager::Register);
}

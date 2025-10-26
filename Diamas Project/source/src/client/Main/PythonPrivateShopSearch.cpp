#include "PythonPrivateShopSearch.h"
#include "stdafx.h"

CPythonPrivateShopSearch::CPythonPrivateShopSearch() {}

CPythonPrivateShopSearch::~CPythonPrivateShopSearch() {}

void CPythonPrivateShopSearch::AddItemData(const ShopSearchItemData& rItemData)
{
    m_ItemInstanceVector.push_back(rItemData);
}

void CPythonPrivateShopSearch::ClearItemData()
{
    m_ItemInstanceVector.clear();
}

uint32_t
CPythonPrivateShopSearch::GetItemDataPtr(uint32_t index,
                                         ShopSearchItemData** ppInstance)
{
    *ppInstance = &m_ItemInstanceVector.at(index);
    return 1;
}

ShopSearchItemData* CPythonPrivateShopSearch::GetItemData(uint32_t index)
{
    return &m_ItemInstanceVector[index];
}

void init_privateShopSearch(py::module& m)
{
    py::module shop = m.def_submodule("privateShopSearch", "");

    py::class_<TOfflineShopItemData>(shop, "TOfflineShopItemData")
        .def_readwrite("info", &TOfflineShopItemData::info)
        .def_readwrite("dwPosition", &TOfflineShopItemData::dwPosition)
        .def_readwrite("llPrice", &TOfflineShopItemData::llPrice);

    py::class_<ShopSearchItemData>(shop, "ShopSearchItemData")
        .def_readwrite("data", &ShopSearchItemData::data)
        .def_readwrite("ownerVid", &ShopSearchItemData::ownerVid)
        .def_readwrite("mapIndex", &ShopSearchItemData::mapIndex)
        .def_readwrite("channel", &ShopSearchItemData::channel)
       .def_property("ownerName",
        [](const ShopSearchItemData &self) { return py::bytes(self.ownerName); },
        [](ShopSearchItemData &self, const char *new_a) { storm::CopyStringSafe(self.ownerName, new_a); }
    );
    ;

    py::class_<ShopSearchPageInfo>(shop, "ShopSearchPageInfo")
        .def_readwrite("pageCount", &ShopSearchPageInfo::pageCount)
        .def_readwrite("perPage", &ShopSearchPageInfo::perPage)
        .def_readwrite("page", &ShopSearchPageInfo::page);

    py::class_<ShopSearchFilter>(shop, "ShopSearchFilter")
        .def_readwrite("minAvgDmg", &ShopSearchFilter::minAvgDmg)
        .def_readwrite("maxAvgDmg", &ShopSearchFilter::maxAvgDmg)
        .def_readwrite("minSkillDmg", &ShopSearchFilter::minSkillDmg)
        .def_readwrite("maxSkillDmg", &ShopSearchFilter::maxSkillDmg)
        .def_readwrite("minLevel", &ShopSearchFilter::minLevel)
        .def_readwrite("maxLevel", &ShopSearchFilter::maxLevel)
        .def_readwrite("sex", &ShopSearchFilter::sex)
    ;

    auto cps =
        py::class_<CPythonPrivateShopSearch,
                   std::unique_ptr<CPythonPrivateShopSearch, py::nodelete>>(
            shop, "shopSearchInst")
            .def(py::init([]() {
                     return std::unique_ptr<CPythonPrivateShopSearch,
                                            py::nodelete>(
                         CPythonPrivateShopSearch::InstancePtr());
                 }),
                 pybind11::return_value_policy::reference_internal)
            .def("GetItemData", &CPythonPrivateShopSearch::GetItemData,
                 pybind11::return_value_policy::reference)
            .def("ClearItemData", &CPythonPrivateShopSearch::ClearItemData)
            .def("GetItemDataCount",
                 &CPythonPrivateShopSearch::GetItemDataCount)
            .def("SetShopSearchFilters",
                 &CPythonPrivateShopSearch::SetShopSearchFilters)
            .def("GetShopSearchFilters",
                 &CPythonPrivateShopSearch::GetShopSearchFilters)
            .def("GetPageInfo",
                 &CPythonPrivateShopSearch::GetPageInfo)

        ;

    
}

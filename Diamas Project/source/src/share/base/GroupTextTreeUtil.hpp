#ifndef METIN2_BASE_GROUPTEXTTREEUTIL_HPP
#define METIN2_BASE_GROUPTEXTTREEUTIL_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <base/GroupTextTree.hpp>

#include <storm/String.hpp>
#include <storm/StringUtil.hpp>

#include <boost/type_traits/is_arithmetic.hpp>

#include <Eigen/Core>
#include <optional>

namespace DirectX {
namespace SimpleMath {
struct Vector3;
}
}

bool GetGroupProperty(const GroupTextGroup* group,
                      const storm::StringRef& name,
                      std::string& value);

bool GetGroupProperty(const GroupTextGroup* group,
                      const storm::StringRef& name,
                      std::string_view& value);

bool GetGroupProperty(const GroupTextGroup* group,
                      const std::string& name,
                      std::optional<std::string>& value);

bool GetGroupProperty(const GroupTextGroup* group,
                      const std::string& name,
                      std::optional<float>& value);

template <typename T>
typename boost::enable_if<
	boost::is_arithmetic<T>,
	bool
>::type GetGroupProperty(const GroupTextGroup* group,
                         const storm::StringRef& name,
                         T& val)
{
	const auto& prop = group->GetProperty(name);
	if (prop.empty())
		return false;

	return storm::ParseNumber(prop, val);
}

bool GetGroupProperty(const GroupTextGroup* group,
                      const storm::StringRef& name,
                      Eigen::Vector3f& vec);

#if VSTD_PLATFORM_WINDOWS

bool GetGroupProperty(const GroupTextGroup* group,
                      const std::string&    name,
                      DirectX::SimpleMath::Vector3&          vec);

#endif

template <std::size_t N>
bool VerifyRequiredPropertiesPresent(const GroupTextGroup* group,
                                     const std::string_view (&properties) [N])
{
	for (std::size_t i = 0; i != N; ++i) {
		const auto& prop = group->GetProperty(properties[i]);
		if (!prop.empty())
			continue;

		SPDLOG_ERROR("Required property {0} is missing", properties[i]);
		return false;
	}

	return true;
}

#endif

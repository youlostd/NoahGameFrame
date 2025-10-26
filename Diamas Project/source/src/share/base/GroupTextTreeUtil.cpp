#include <base/GroupTextTreeUtil.hpp>
#include <base/GroupTextTree.hpp>

bool GetGroupProperty(const GroupTextGroup* group,
                      const storm::StringRef& name,
                      Eigen::Vector3f& vec)
{
	const auto tokens = group->GetTokens(name);
	if (!tokens || tokens->size() != 3)
		return false;

	return storm::ParseNumber((*tokens)[0], vec.x()) &&
	       storm::ParseNumber((*tokens)[1], vec.y()) &&
	       storm::ParseNumber((*tokens)[2], vec.z());
}

#if VSTD_PLATFORM_WINDOWS
#include <base/SimpleMath.h>

bool GetGroupProperty(const GroupTextGroup* group, const std::string& name, DirectX::SimpleMath::Vector3& vec) {
	const auto tokens = group->GetTokens(name);
	if (!tokens || tokens->size() != 3)
		return false;

	bool succeeded = storm::ParseNumber((*tokens)[0], vec.x);
	if (!succeeded)
		return false;

	succeeded = storm::ParseNumber((*tokens)[1], vec.y);
	if (!succeeded)
		return false;

	succeeded = storm::ParseNumber((*tokens)[2], vec.z);
	if (!succeeded)
		return false;

	return succeeded;
}
#endif

bool GetGroupProperty(const GroupTextGroup* group,
                      const storm::StringRef& name,
                      std::string& value)
{
	const auto& prop = group->GetProperty(name);
	if (!prop.empty()) {
		value.assign(prop.data(), prop.size());
		return true;
	}

	return false;
}

bool GetGroupProperty(const GroupTextGroup* group,
                      const storm::StringRef& name,
                      std::string_view& value)
{
	const auto& prop = group->GetProperty(name);
	if (!prop.empty()) {
		value = prop;
		return true;
	}

	return false;
}

bool GetGroupProperty(const GroupTextGroup* group, const std::string& name, std::optional<std::string>& value)
{
	const auto& prop = group->GetProperty(name);
	if(!prop.empty()) {
		value = prop;
		return true;
	}

	return false;
}

bool GetGroupProperty(const GroupTextGroup* group, const std::string& name, std::optional<float>& value)
{
	const auto& prop = group->GetProperty(name);
	if (prop.empty())
		return false;

	float val = 0.0f;
	if(storm::ParseNumber(prop, val)) {
		value=val;
		return true;
	}

	value = std::nullopt;
	return false;
}
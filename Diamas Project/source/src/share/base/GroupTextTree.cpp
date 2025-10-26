#include "GroupTextTree.hpp"

#include <storm/memory/StdAllocator.hpp>
#include <storm/memory/New.hpp>

#include <list>
#include <iterator>
#include <utility>



//
// GroupTextNode
//

uint32_t GroupTextNode::GetType() const
{
	return m_type;
}

storm::StringRef GroupTextNode::GetName() const
{
	return m_name;
}

GroupTextNode::GroupTextNode(uint32_t type,
                             const storm::StringRef& name,
                             GroupTextMemory* memory)
	: m_type(type)
	, m_name(name)
	, m_memory(memory)
{
	// ctor
}

/*virtual*/ GroupTextNode::~GroupTextNode()
{
	// dtor
}

//
// GroupTextList
//

GroupTextList::GroupTextList(const storm::StringRef& name,
                             GroupTextMemory* memory)
	: GroupTextNode(kList, name, memory)
{
	// ctor
}

uint32_t GroupTextList::GetLineCount() const
{
	return m_lines.size();
}

const std::vector<storm::String>& GroupTextList::GetLines() const
{
	return m_lines;
}

storm::StringRef GroupTextList::GetLine(uint32_t index) const
{
	STORM_ASSERT(index < m_lines.size(), "Out of bounds");
	return m_lines[index];
}

void GroupTextList::AddLine(storm::String line)
{
	m_lines.push_back(line);
}

//
// GroupTextGroup
//

GroupTextGroup::GroupTextGroup(const storm::StringRef& name,
                               GroupTextMemory* memory)
	: GroupTextNode(kGroup, name, memory)
{
	// ctor
}

/*virtual*/ GroupTextGroup::~GroupTextGroup()
{
	auto f = [this] (const ChildMap::value_type& p) {
		auto node = p.second;
		if (node->GetType() == kGroup)
			m_memory->DeleteGroup(static_cast<GroupTextGroup*>(node));
		else
			m_memory->DeleteList(static_cast<GroupTextList*>(node));
	};

	std::for_each(m_children.begin(), m_children.end(), f);
}

const GroupTextGroup::ChildMap& GroupTextGroup::GetChildren() const
{
	return m_children;
}

const GroupTextGroup* GroupTextGroup::GetGroup(storm::StringRef name) const
{
	const auto it = m_children.find(name);

	if (it != m_children.end() &&
	    it->second->GetType() == kGroup)
		return static_cast<const GroupTextGroup*>(it->second);

	return nullptr;
}

const GroupTextList* GroupTextGroup::GetList(storm::StringRef name) const
{
	const auto it = m_children.find(name);

	if (it != m_children.end() &&
	    it->second->GetType() == kList)
		return static_cast<const GroupTextList*>(it->second);

	return nullptr;
}

bool GroupTextGroup::AddChild(GroupTextNode* node)
{
	STORM_ASSERT(node != nullptr, "NULL node given");

	auto r = m_children.insert(std::make_pair(node->GetName(), node));
	if (!r.second) {
		auto old = r.first->second;
		if (old->GetType() == kGroup)
			m_memory->DeleteGroup(static_cast<GroupTextGroup*>(old));
		else
			m_memory->DeleteList(static_cast<GroupTextList*>(old));

		r.first->second = node;
		return true;
	}

	return false;
}

const GroupTextGroup::PropertyMap& GroupTextGroup::GetProperties() const
{
	return m_properties;
}

storm::StringRef GroupTextGroup::GetProperty(storm::StringRef key) const
{
	const auto it = m_properties.find(key);

	if (it != m_properties.end())
		return it->second[0];

	return storm::StringRef();
}

const std::vector<storm::String>* GroupTextGroup::GetTokens(
    storm::StringRef key) const
{
	const auto it = m_properties.find(key);

	if (it != m_properties.end())
		return &it->second;

	return nullptr;
}

void GroupTextGroup::SetProperty(storm::StringRef key,
                                 storm::StringRef value)
{
	const std::string keyStr(key);
	auto& vec  = m_properties[keyStr];

	vec.clear();
	vec.emplace_back(value);
}

void GroupTextGroup::SetProperty(const storm::StringRef& key,
                                 const std::vector<storm::String>& value)
{
	const std::string keyStr(key);
	auto& vec  = m_properties[keyStr];

	vec.clear();
	vec.reserve(value.size());

	std::copy(value.begin(), value.end(), std::back_inserter(vec));
}

//
// GroupTextMemory
//

GroupTextMemory::GroupTextMemory(const storm::Allocator& allocator)
	: m_allocator(allocator)
{
	// ctor
}

const storm::Allocator& GroupTextMemory::GetRawAllocator() const
{
	return m_allocator;
}

GroupTextList* GroupTextMemory::NewList(const storm::StringRef& name)
{
	return storm::New<GroupTextList>(m_allocator, 0, name, this);
}

GroupTextGroup* GroupTextMemory::NewGroup(const storm::StringRef& name)
{
	return storm::New<GroupTextGroup>(m_allocator, 0, name, this);
}

void GroupTextMemory::DeleteList(GroupTextList* l)
{
	storm::Delete(m_allocator, l);
}

void GroupTextMemory::DeleteGroup(GroupTextGroup* g)
{
	storm::Delete(m_allocator, g);
}

//
// GroupTextReader
//

GroupTextReader::GroupTextReader(GroupTextMemory* memory)
	: GroupTextGroup("Root", memory)
{
	// ctor
}

namespace
{

class Parser : public GroupTextParser<Parser>
{
	public:
		Parser(GroupTextGroup* root,
		       GroupTextMemory* memory,
		       const storm::StringRef& filename)
			: m_memory(memory)
			, m_parents(memory->GetRawAllocator())
			, m_filename(filename)
			, m_currentGroup(root)
		{
			// ctor
		}

		void Warn(const char* msg)
		{
			SPDLOG_WARN("{0}:{1}: warning: {2}",
			          m_filename, m_currentLine, msg);
		}

		// GroupTextParser<T> callbacks

		void OnError(const char* msg)
		{
			spdlog::error("{0}:{1} error: {2}",
			          m_filename, m_currentLine, msg);

			m_errorOccurred = true;
		}

		void OnScopeOpen(GroupTextScope type, const storm::StringRef& name)
		{
			m_parents.push_back(m_currentGroup);

			if (type == GroupTextScope::kGroup) {
				auto newCurrent = m_memory->NewGroup(name);

				if (m_currentGroup->AddChild(newCurrent))
					Warn("Existing Group node replaced");

				m_currentGroup = newCurrent;
			} else if (type == GroupTextScope::kList) {
				auto newCurrent = m_memory->NewList(name);

				if (m_currentGroup->AddChild(newCurrent))
					Warn("Existing List node replaced");

				m_currentList = newCurrent;
			}
		}

		void OnScopeClose()
		{
			if (m_parents.empty()) {
				OnError("Too many closing braces");
				return;
			}

			m_currentGroup = m_parents.back();
			m_parents.pop_back();
		}

		void OnKeyValuePair(const storm::StringRef& key,
		                    storm::ArgumentExtractor& ex)
		{
			STORM_ASSERT(m_currentScopeType == GroupTextScope::kGroup,
			             "Can only be used in group mode.");

			std::vector<storm::String> tokens;

			while (ex.Next())
				tokens.emplace_back(ex.GetCurrent());

			m_currentGroup->SetProperty(key, tokens);
		}

		void OnListItems(storm::StringRef line)
		{
			STORM_ASSERT(m_currentScopeType == GroupTextScope::kList,
			             "Can only be used in group mode.");

			m_currentList->AddLine(storm::String(line.data(), line.size()));
		}

	private:
		GroupTextMemory* m_memory;
		std::list<GroupTextGroup*, storm::StdAllocator<GroupTextGroup*> > m_parents;
		const storm::StringRef m_filename;

		union {
			GroupTextGroup* m_currentGroup;
			GroupTextList* m_currentList;
		};
};

}

bool GroupTextReader::LoadString(const storm::StringRef& data)
{
	Parser p(this, m_memory, "<string>");
	return p.Parse(data);
}

bool GroupTextReader::LoadFile(const storm::StringRef& filename)
{
	Parser p(this, m_memory, filename);
	return LoadGroupTextFile(m_memory->GetRawAllocator(), filename, p);
}



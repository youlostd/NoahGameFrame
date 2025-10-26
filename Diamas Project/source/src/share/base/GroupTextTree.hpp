#ifndef METIN2_BASE_GROUPTEXTTREE_HPP
#define METIN2_BASE_GROUPTEXTTREE_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <base/GroupText.hpp>

#include <storm/String.hpp>

#include <boost/unordered_map.hpp>

#include <vector>
#include <map>
#undef LoadString
struct StringCmp {
  using is_transparent = void;
  bool operator()(std::string_view a, std::string_view b) const {
    return a < b;
  }
};

class GroupTextGroup;
class GroupTextMemory;

/// An abstract GroupText node.
///
/// This class represents an abstract GroupText node,
/// which shouldn't be used directly. Use the two child classes
/// @c GroupTextList and @c GroupTextGroup instead.
class GroupTextNode
{
	public:
		/// The two GroupText node types.
		enum Type {
			kGroup,
			kList
		};

		/// Get the type of this node.
		///
		/// @return A type listed in the @c Type enum.
		uint32_t GetType() const;

		/// Get the node name:
		///
		/// @return The name of this node.
		storm::StringRef GetName() const;

	protected:
		GroupTextNode(uint32_t type,
		              const storm::StringRef& name,
		              GroupTextMemory* memory);

		virtual ~GroupTextNode();

		uint32_t m_type;
		storm::String m_name;
		GroupTextMemory* m_memory;
};

/// A list element.
///
/// This class represents a list element inside a GroupText document.
///
/// @par Syntax Example
/// @code
/// List MyListName
/// {
///     Line1
///     Line2
///     Line3
///     ...
/// }
/// @endcode
class GroupTextList : public GroupTextNode
{
	public:
		// undocumented
		GroupTextList(const storm::StringRef& name, GroupTextMemory* memory);

		/// Get the number of lines this list has.
		///
		/// @return The number of lines.
		uint32_t GetLineCount() const;

		/// Get the list lines.
		///
		/// This function is used to retrieve a vector of all lines.
		///
		/// @return A const ref. to the line vector.
		const std::vector<storm::String>& GetLines() const;

		/// Get a list line.
		///
		/// This function is used to retrieve a specific line from the list.
		///
		/// @param index The index of the line to retrieve.
		/// Must be in [0, LineCount].
		///
		/// @return A const ref. to the specified line.
		storm::StringRef GetLine(uint32_t index) const;

		/// Add a line to the list.
		///
		/// This function adds the given line to this list.
		///
		/// @param line The line to add.
		void AddLine(storm::String line);

	private:
		std::vector<storm::String> m_lines;
};

  struct ci_less
  {
	 using is_transparent = std::true_type;
  	
    // case-independent (ci) compare_less binary function
    struct nocase_compare
    {
      bool operator() (const unsigned char& c1, const unsigned char& c2) const {
          return tolower (c1) < tolower (c2); 
      }
    };
    bool operator() (const std::string & s1, const std::string & s2) const {
      return std::lexicographical_compare 
        (s1.begin (), s1.end (),   // source range
        s2.begin (), s2.end (),   // dest range
        nocase_compare ());  // comparison
    }

    bool operator() (std::string_view  s1, std::string_view  s2) const {
      return std::lexicographical_compare 
        (s1.begin (), s1.end (),   // source range
        s2.begin (), s2.end (),   // dest range
        nocase_compare ());  // comparison
    }
  };

/// A group element.
///
/// This class represents a group element inside a GroupText document.
///
/// @par Syntax Example
/// @code
/// Group MyGroupName
/// {
///     Key1 Value1
///     Key1	"Value1"
///     Key1	Value1
///
///     Group SubGroupName
///     {
///         Key1 Value1
///         ...
///     }
///
///     List SubListName
///     {
///         Line1
///         ...
///     }
/// }
/// @endcode
///
///



class GroupTextGroup : public GroupTextNode
{
	public:
		typedef std::map<
			storm::String,
			GroupTextNode*, ci_less
		> ChildMap;

		typedef std::map<
			storm::String,
			std::vector<storm::String>, ci_less
		> PropertyMap;

		// undocumented
		GroupTextGroup(const storm::StringRef& name, GroupTextMemory* memory);
		/*virtual*/ ~GroupTextGroup();

		/// Get the map containing all child nodes.
		const ChildMap& GetChildren() const;

		/// Get a child group.
		const GroupTextGroup* GetGroup(storm::StringRef name) const;

		/// Get a child list.
		const GroupTextList* GetList(storm::StringRef name) const;

		/// Add a child node.
		///
		/// This function adds the given child node (List or Group) to the
		/// child list, replacing any other node with the same name.
		/// Ownership is transferred to the parent @c GroupTextGroup.
		///
		/// @param node The node to insert.
		///
		/// @return A @c bool denoting whether the node replaced an existing.
		bool AddChild(GroupTextNode* node);

		/// Get the map containing all property tokens.
		const PropertyMap& GetProperties() const;

		/// Get the first token of a property.
		storm::StringRef GetProperty(storm::StringRef key) const;

		/// Get all tokens of a property.
		const std::vector<storm::String>* GetTokens(
		     storm::StringRef key) const;

		/// Set a property to a single token.
		void SetProperty(storm::StringRef key,
		                 storm::StringRef value);

		/// Set a property to a token vector.
		void SetProperty(const storm::StringRef& key,
		                 const std::vector<storm::String>& value);

	private:
		ChildMap m_children;
		PropertyMap m_properties;
};

/// Memory manager for GroupText documents.
///
/// This class is used to new/delete GroupText nodes (lists, groups).
class GroupTextMemory
{
	public:
		GroupTextMemory(const storm::Allocator& allocator);

		const storm::Allocator& GetRawAllocator() const;

		GroupTextList* NewList(const storm::StringRef& name);
		GroupTextGroup* NewGroup(const storm::StringRef& name);

		void DeleteList(GroupTextList* l);
		void DeleteGroup(GroupTextGroup* g);

	private:
		storm::Allocator m_allocator;
};

/// Read a GroupText document from a file or string.
///
/// This class is used to read a GroupText document from a file or a string.
/// It also represents the top-node in the loaded document.
class GroupTextReader : public GroupTextGroup
{
	public:
		/// Construct a new GroupTextReader.
		///
		/// @param memory The @c GroupTextMemory to use for this document.
		GroupTextReader(GroupTextMemory* memory);

		/// Load a GroupText document from a string.
		///
		/// This function is used to load a GroupText document from a string.
		///
		/// @param data The complete document text.
		///
		/// @return A @c bool denoting whether the
		/// string was successfully parsed.
		bool LoadString(const storm::StringRef& data);

		/// Load a GroupText document from a file.
		///
		/// This function is used to load a GroupText document from a file.
		///
		/// @param filename The path of the file to load.
		///
		/// @return A @c bool denoting whether the
		/// file was successfully loaded.
		bool LoadFile(const storm::StringRef& filename);
};



#endif

#include "Main.hpp"

#include <base/SimpleApp.hpp>
#include <base/DumpVersion.hpp>

#include <storm/io/File.hpp>
#include <storm/log/StdLogSink.hpp>
#include <storm/StringUtil.hpp>

#include <boost/system/error_code.hpp>
#include <boost/scoped_ptr.hpp>

#include <vector>

METIN2_BEGIN_NS

namespace
{

class Main : public SimpleApp<Main>
{
	public:
		enum OptionIndex
		{
			kUnknown,
			kDumpVersion,
			kAction,
			kVerbose,
		};

		static const option::Descriptor kUsage[];

		Main();

		int Run();

		bool OnCmdLineParsed(option::Parser& parse,
		                     const ScopedOptionArray& options);

	private:
		ActionRegistrator* GetAction(const storm::StringRef& name);

		storm::StdLogSink m_stdLogSink;

		boost::scoped_ptr<ActionInterface> m_action;
		bool m_verbose;
};

static ActionRegistrator* first = nullptr;

/*static*/ const option::Descriptor Main::kUsage[] = {
	{ kUnknown, 0, "", "", option::Arg::Unknown, "usage: conv_proto [options] files...\n\nOptions:" },
	{ kDumpVersion, 0, "v", "version", option::Arg::None, "  -v,--version  \tPrint version" },
	{ kAction, 0, "a", "action", option::Arg::Required, "  -a,--action  \tName of the conversion to perform" },
	{ kVerbose, 0, "", "verbose", option::Arg::None, "  --verbose  \tBe verbose" },
	{ 0, 0, 0, 0, 0, 0 }
};

Main::Main()
	: m_action(nullptr)
	, m_verbose(false)
{
	m_logger.AddLogSink(storm::LogSeverity::kInfo, &m_stdLogSink);
}

int Main::Run()
{
	if (m_action)
		return m_action->Run(m_verbose) ? 0 : 1;

	return 0;
}

bool Main::OnCmdLineParsed(option::Parser& parse,
                           const ScopedOptionArray& options)
{
	if (options[kDumpVersion]) {
		DumpVersion();
		std::exit(1);
	}

#define METIN2_GET_ARG(name, var) do { \
	if (options[name] && options[name].arg) \
		var = options[name].arg; \
} while (false)

	storm::StringRef action;
	METIN2_GET_ARG(kAction, action);

#undef METIN2_GET_ARG

	if (!action.empty()) {
		auto a = GetAction(action);
		if (!a) {
			STORM_LOG(Error, "No such action: {0}", action);
			return false;
		}

		m_action.reset(a->fn());

		if (!m_action->ParseArguments(parse)) {
			m_action.reset();
			return false;
		}
	} else {
		// TODO(tim): Help syntax
	}

	if (options[kVerbose])
		m_verbose = true;

	return true;
}

ActionRegistrator* Main::GetAction(const storm::StringRef& name)
{
	for (auto a = first; a; a = a->next) {
		if (a->name == name)
			return a;
	}

	return nullptr;
}

}

ActionRegistrator::ActionRegistrator(const storm::StringRef& name,
                                     CreateFunction fn)
	: name(name)
	, fn(fn)
	, next(first)
{
	first = this;
}

METIN2_IMPLEMENT_SIMPLEAPP(Main)

METIN2_END_NS

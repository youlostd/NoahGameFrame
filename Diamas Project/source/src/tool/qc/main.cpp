#include "stdafx.h"
#include "crc32.hpp"

#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <filesystem>

namespace fs = std::filesystem;

extern "C" {

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

// from lua src
#include <lzio.h>
#include <llex.h>
#include <lstring.h>

}

#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <string>
#include <sstream>
#include <utility>
#include <vector>

#if !VSTD_PLATFORM_WINDOWS
	#include <unistd.h>
#else
	#include <direct.h>
	#define mkdir(path, mode) _mkdir(path)
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>


using namespace std;

class Defines
{
	public:
		void Add(std::string name, std::string replacement);
		const std::string& Replace(const std::string& name) const;

	private:
		std::unordered_map<std::string, std::string> m_defines;
};

void Defines::Add(std::string name, std::string replacement)
{
	m_defines[name] = replacement;
}

const std::string& Defines::Replace(const std::string& name) const
{
	const auto it = m_defines.find(name);
	if (it == m_defines.end())
		return name;

	return it->second;
}

lua_State* L;

struct LoadF
{
	LoadF()
		: f(nullptr)
	{
		// ctor
	}

	~LoadF()
	{
		if (f)
			fclose(f);
	}

	FILE *f;
	char buff[LUAL_BUFFERSIZE];
};

const char* g_filename;

static const char *getF (lua_State *L, void *ud, size_t *size)
{
	LoadF* lf = (LoadF *)ud;
	if (feof(lf->f))
		return NULL;

	*size = fread(lf->buff, 1, LUAL_BUFFERSIZE, lf->f);
	return (*size > 0) ? lf->buff : NULL;
}

static void next (LexState * ls)
{
	ls->lastline = ls->linenumber;

	if (ls->lookahead.token != TK_EOS) {
		/* is there a look-ahead token? */
		ls->t = ls->lookahead;  /* use this one */
		ls->lookahead.token = TK_EOS;  /* and discharge it */
	} else {
		ls->t.token = luaX_lex(ls, &ls->t.seminfo);  /* read next token */
	}
}

static bool testnext (LexState * ls, int c)
{
	if (ls->t.token == c) {
		next(ls);
		return true;
	} else {
		return false;
	}
}

static void lookahead (LexState * ls)
{
	lua_assert(ls->lookahead.token == TK_EOS);
	ls->lookahead.token = luaX_lex(ls, &ls->lookahead.seminfo);
}

enum parse_state
{
	ST_START,
	ST_QUEST,
	ST_QUEST_WITH_OR_BEGIN,
	ST_STATELIST,
	ST_STATE_NAME,
	ST_STATE_BEGIN,
	ST_WHENLIST_OR_FUNCTION,
	ST_WHEN_NAME,
	ST_WHEN_WITH_OR_BEGIN,
	ST_WHEN_BODY,
	ST_FUNCTION_NAME,
	ST_FUNCTION_ARG,
	ST_FUNCTION_BODY,
	ST_DEFINE_NAME,
	ST_DEFINE_VALUE,
};

enum SpecialToken
{
	STK_NONE,
	STK_DEFINE,
};

struct LexState * pls = 0;
void errorline(int line, const char* str)
{
	cerr.flush();

	if (g_filename)
		cerr << g_filename << ":";

	cerr << line<< ':';
	cerr << str << endl;

	exit(1);
}

void error(const char * str)
{
	cerr.flush();

	cerr << (g_filename ? g_filename : "<Inline>") << ':';
	cerr << (pls ? pls->linenumber : 0) << ':' << endl;
	cerr << '\t' << str << endl;

	exit(1);
}

#define verify(exp) if (!(exp)) error("assertion failure : " #exp)

uint32_t GetSpecialToken(const Token& t)
{
	if (t.token != TK_NAME)
		return STK_NONE;

	const auto str = getstr(t.seminfo.ts);
	if (0 == strcmp(str, "define"))
		return STK_DEFINE;

	return STK_NONE;
}

void FormatTokens(ostream& ostr, const Token& tok, const Defines& d)
{
	if (tok.token == TK_NAME)
		ostr << d.Replace(getstr(tok.seminfo.ts));
	else if (tok.token == TK_NUMBER)
		ostr << tok.seminfo.r;
	else if (tok.token == TK_STRING)
		ostr << '"' << getstr(tok.seminfo.ts) << '"';
	else
		ostr << luaX_token2str(pls, tok.token);
}

bool check_syntax(const std::string& str, const std::string& module)
{
	int ret = luaL_loadbuffer(L, str.c_str(), str.size(), module.c_str());
	if (ret != 0) {
		cerr << endl << "===== ERRONEOUS CODE BEGIN =====" << endl;
		cerr << str << endl;
		cerr << "===== ERRONEOUS CODE END =====" << endl << endl;

		error(lua_tostring(L, -1));
		return false;
	}

	lua_pop(L,1);
	return true;
}

struct AScript
{
	std::string when_condition;
	std::string when_argument;
	std::string script;

	AScript(const std::string& a, const std::string& b, const std::string& c)
		: when_condition(a)
		, when_argument(b)
		, script(c)
	{
		// ctor
	}

	AScript() {}
};

std::string MakeWhenPath(const std::string& output_path,
                         const std::string& when_name)
{
	std::string path;

	const auto dotPos = when_name.find('.');
	if (dotPos == std::string::npos) {
		// one
		std::string s(when_name);
		std::transform(s.begin(), s.end(), s.begin(), ::tolower);

		path = output_path + "/notarget";
		mkdir(path.c_str(), 0755);

		path += '/'; path += s;
		mkdir(path.c_str(), 0755);

		path += '/';
	} else {
		// two like [WHO].Kill
		std::string s = when_name;
		transform(s.begin(), s.end(), s.begin(), ::tolower);

		path = output_path + "/" + when_name.substr(0, dotPos);
		mkdir(path.c_str(), 0755);

		path += '/'; path += s.substr(dotPos + 1, std::string::npos);
		mkdir(path.c_str(), 0755);

		path += '/';
	}

	return std::move(path);
}

void WriteStringToFile(const std::string& path, const std::string& data)
{
	std::ofstream ouf(path.c_str());
	if (!ouf)
		throw std::runtime_error("Failed to save " + path);

	std::copy(data.begin(), data.end(),
	          std::ostreambuf_iterator<char>(ouf));
}

void parse(const std::string& output_path, const char* filename)
{
	LoadF lf;
	lf.f = fopen(filename, "r");

	if (lf.f == nullptr) {
		cerr << "Failed to open " << filename << endl;
		return;
	}

	g_filename = filename;

	ZIO z;
	luaZ_init(&z, getF, &lf, "quest");

	struct LexState lexstate;
	pls = &lexstate;

	Mbuffer b;
	luaZ_initbuffer(L,&b);
	lexstate.buff = &b;

	luaX_setinput(L, &lexstate, &z, luaS_new(L, zname(&z)));

	parse_state ps = ST_START;

	int nested = 0;

	std::string quest_name;
	std::string start_condition;
	std::string current_state_name;
	std::string current_when_name;
	std::string current_when_condition;
	std::string current_when_argument;

	std::unordered_set<std::string> define_state_name_set;
	std::unordered_map<int, std::string> used_state_name_map;

	std::unordered_map<string, std::unordered_map<std::string, std::string> > state_script_map;
	std::unordered_map<string, std::unordered_map<std::string, std::vector<AScript> > > state_arg_script_map;

	std::vector<std::pair<std::string, std::string> > when_name_arg_vector;

	std::string current_function_name;
	std::string current_function_arg;
	std::string all_functions;

	// define <name> <value>
	Defines defines;
	std::string cur_define_name;

	while (true) {
		next(&lexstate);
		if (lexstate.t.token == TK_EOS)
			break;

		Token& t= lexstate.t;

		switch(ps)
		{
			case ST_START:
				{
					verify(nested==0);
					if (t.token == TK_QUEST)
						ps = ST_QUEST;
					else if (GetSpecialToken(t) == STK_DEFINE)
						ps = ST_DEFINE_NAME;
					else
						error("must start with 'quest'");
				}
				break;
			case ST_DEFINE_NAME: {
				if (t.token != TK_NAME)
					error("'define' needs to be followed by TK_NAME");

				cur_define_name = getstr(lexstate.t.seminfo.ts);
				ps = ST_DEFINE_VALUE;
				break;
			}
			case ST_DEFINE_VALUE: {
				std::ostringstream os;

				if (t.token != '(') {
					FormatTokens(os, t, defines);
				} else {
					do {
						FormatTokens(os, t, defines);
						os << ' ';
						next(&lexstate);
					} while (t.token != ')' && t.token != TK_EOS);

					if (t.token == TK_EOS)
						error("eof encountered while scanning for )");

					os << ')';
				}

				defines.Add(std::move(cur_define_name), os.str());

				ps = ST_START;
				break;
			}
			case ST_QUEST:
				{
					verify(nested==0);
					if (t.token == TK_NAME || t.token == TK_STRING)
					{
						quest_name = getstr(lexstate.t.seminfo.ts);
						cout << "QUEST : " << quest_name << endl;
						ps = ST_QUEST_WITH_OR_BEGIN;
					}
					else
						error("quest name must be given");
				}
				break;
			case ST_QUEST_WITH_OR_BEGIN:
				verify(nested==0);
				if (t.token == TK_WITH)
				{
					verify(nested==0);
					next(&lexstate);
					ostringstream os;
					FormatTokens(os, lexstate.t, defines);
					next(&lexstate);
					while (lexstate.t.token!=TK_DO)
					{
						os << " ";
						FormatTokens(os, lexstate.t, defines);
						//cout << TK_DO<<lexstate.t.token << " " <<(lexstate.t) <<endl;
						next(&lexstate);
					}
					start_condition = os.str();
					check_syntax("if "+start_condition+" then end", quest_name);
					cout << "\twith ";
					cout << start_condition;
					cout << endl;
					t = lexstate.t;
				}
				if (t.token == TK_DO)
				{
					ps = ST_STATELIST;
					nested++;
				}
				else
				{
					ostringstream os;
					os << "quest doesn't have begin-end clause. (";
					FormatTokens(os, t, defines);
					os << ")";
					error(os.str().c_str());
				}
				break;
			case ST_STATELIST:
				{
					verify(nested==1);
					if (t.token == TK_STATE)
					{
						ps = ST_STATE_NAME;
					}
					else if (t.token == TK_END)
					{
						nested --;
						ps = ST_START;
					}
					else
					{
						error("expecting 'state'");
					}
				}
				break;
			case ST_STATE_NAME:
				{
					verify(nested==1);
					if (t.token == TK_NAME || t.token == TK_STRING)
					{
						current_state_name = getstr(t.seminfo.ts);
						define_state_name_set.insert(current_state_name);
						cout << "STATE : " << current_state_name << endl;
						ps = ST_STATE_BEGIN;
					}
					else
					{
						error("state name must be given");
					}
				}
				break;

			case ST_STATE_BEGIN:
				{
					verify(nested==1);

					if (t.token == TK_DO)
					{
						nested ++;
						ps = ST_WHENLIST_OR_FUNCTION;
					}
					else
					{
						error("state doesn't have begin-end clause.");
					}
				}
				break;

			case ST_WHENLIST_OR_FUNCTION:
				{
					verify(nested==2);

					if (t.token == TK_WHEN)
					{
						ps = ST_WHEN_NAME;

						when_name_arg_vector.clear();
					}
					else if (t.token == TK_END)
					{
						nested--;
						ps = ST_STATELIST;
					}
					else if (t.token == TK_FUNCTION)
					{
						ps = ST_FUNCTION_NAME;
					}
					else
					{
						error("expecting 'when' or 'function'");
					}
				}
				break;

			case ST_WHEN_NAME:
				{
					verify(nested==2);
					if (t.token == TK_NAME || t.token == TK_STRING || t.token == TK_NUMBER)
					{
						if (t.token == TK_NUMBER)
						{
							ostringstream os;
							os << (unsigned int)t.seminfo.r;
							current_when_name = os.str();
							lexstate.lookahead.token  = '.';
						}
						else
						{
							current_when_name = getstr(t.seminfo.ts);
							lookahead(&lexstate);
						}

						if (t.token == TK_NAME)
							current_when_name = defines.Replace(current_when_name);

						ps = ST_WHEN_WITH_OR_BEGIN;
						current_when_argument = "";
						if (lexstate.lookahead.token == '.')
						{
							next(&lexstate);
							current_when_name += '.';
							next(&lexstate);
							ostringstream os;
							t = lexstate.t;
							FormatTokens(os, t, defines);
							if (os.str() == "target")
							{
								current_when_argument = "." + current_when_name;
								current_when_argument.resize(current_when_argument.size()-1);
								current_when_name = "target";
							}
							else
							{
								current_when_name += os.str();
							}
							lookahead(&lexstate);
						}

						{
							// make when argument
							ostringstream os;
							while (lexstate.lookahead.token == '.')
							{
								next(&lexstate);
								os << '.';
								next(&lexstate);
								t = lexstate.t;
								//if (t.token == TK_STRING)
								//t.token = TK_NAME;
								FormatTokens(os, t, defines);
								lookahead(&lexstate);
							}
							current_when_argument += os.str();
						}
						cout << "WHEN  : " << current_when_name ;
						if (!current_when_argument.empty())
						{
							cout << " (";
							cout << current_when_argument.substr(1);
							cout << ")";
						}
					}
					else
					{
						error("when name must be given");
					}

					if (lexstate.lookahead.token == TK_OR)
					{
						// 다중 when name
						// push to somewhere -.-
						ps = ST_WHEN_NAME;
						when_name_arg_vector.push_back(make_pair(current_when_name, current_when_argument));

						next(&lexstate);
						cout << " or" << endl;
					}
					else
					{
						cout <<endl;
					}
				}
				break;
			case ST_WHEN_WITH_OR_BEGIN:
				{
					verify(nested==2);
					current_when_condition = "";
					if (t.token == TK_WITH)
					{
						// here comes 조건식
						next(&lexstate);
						ostringstream os;
						FormatTokens(os, lexstate.t, defines);
						next(&lexstate);
						while (lexstate.t.token!=TK_DO)
						{
							os << " ";
							FormatTokens(os, lexstate.t, defines);
							//cout << TK_DO<<lexstate.t.token << " " <<(lexstate.t) <<endl;
							next(&lexstate);
						}
						current_when_condition = os.str();
						check_syntax("if "+current_when_condition+" then end", current_state_name+current_when_condition);
						cout << "\twith ";
						cout << current_when_condition;
						cout << endl;
						t = lexstate.t;
					}
					if (t.token == TK_DO)
					{
						ps = ST_WHEN_BODY;
						nested++;
					}
					else
					{
						//error("when doesn't have begin-end clause.");
						ostringstream os;
						os << "when doesn't have begin-end clause. (";
						FormatTokens(os, t, defines);
						os << ")";
						error(os.str().c_str());
					}

				}
				break;
			case ST_WHEN_BODY:
				{
					verify(nested==3);

					// output
					ostringstream os;
					int state_check = 0;
					auto prev = lexstate;
					string callname;
					bool registered = false;
					if (prev.t.token == '.')
						prev.t.token = TK_DO; // any token
					while (1)
					{
						if (lexstate.t.token == TK_DO || lexstate.t.token == TK_IF /*|| lexstate.t.token == TK_FOR*/ || lexstate.t.token == TK_BEGIN || lexstate.t.token == TK_FUNCTION)
						{
							//cout << ">>>" << endl;
							nested++;
						}
						else if (lexstate.t.token == TK_END)
						{
							//cout << "<<<" << endl;
							nested--;
						}

						if (!callname.empty())
						{
							lookahead(&lexstate);
							if (lexstate.lookahead.token == '(')
							{
								//RegisterUsedFunction(callname);
								registered = true;
							}
							callname.clear();
						}
						else if (lexstate.t.token == '(')
						{
							//if (!registered && prev.t.token == TK_NAME)
							//	RegisterUsedFunction(getstr(prev.t.seminfo.ts));
							registered = false;
						}

						if (lexstate.t.token == '.')
						{
							ostringstream fname;
							lookahead(&lexstate);
							FormatTokens(fname, prev.t, defines);
							fname << '.';
							FormatTokens(fname, lexstate.lookahead, defines);
							callname = fname.str();
						}

						if (state_check)
						{
							state_check--;
							if (!state_check)
							{
								if (lexstate.t.token == TK_NAME || lexstate.t.token == TK_STRING)
								{
									used_state_name_map[lexstate.linenumber] = (getstr(lexstate.t.seminfo.ts));
									lexstate.t.token = TK_STRING;
								}
							}
						}

						if (lexstate.t.token == TK_NAME && ((strcmp(getstr(lexstate.t.seminfo.ts),"set_state")==0) || (strcmp(getstr(lexstate.t.seminfo.ts),"newstate")==0) || (strcmp(getstr(lexstate.t.seminfo.ts),"setstate")==0)))
						{
							state_check = 2;
						}
						if (nested==2) break;

						FormatTokens(os, lexstate.t, defines);
						os << ' ';

						prev = lexstate;
						next(&lexstate);
						if (lexstate.linenumber != lexstate.lastline)
							os << endl;
					}


					//cout << os.str() << endl;

					check_syntax(os.str(), current_state_name+current_when_condition);
					reverse(when_name_arg_vector.begin(), when_name_arg_vector.end());
					while (1)
					{
						if (current_when_argument.empty())
						{
							if (current_when_condition.empty())
								state_script_map[current_when_name][current_state_name]+=os.str();
							else
								state_script_map[current_when_name][current_state_name]+="if "+current_when_condition+" then "+os.str()+" return end ";
						}
						else
						{
							state_arg_script_map[current_when_name][current_state_name].push_back(AScript(current_when_condition, current_when_argument, os.str()));
						}

						if (!when_name_arg_vector.empty())
						{
							current_when_name = when_name_arg_vector.back().first;
							current_when_argument = when_name_arg_vector.back().second;
							when_name_arg_vector.pop_back();
						}
						else
							break;
					}

					ps = ST_WHENLIST_OR_FUNCTION;
				}
				break;
			case ST_FUNCTION_NAME:
				if (t.token == TK_NAME)
				{
					current_function_name = getstr(t.seminfo.ts);
					//RegisterDefFunction(quest_name+"."+current_function_name);
					ps = ST_FUNCTION_ARG;
				}
				break;
			case ST_FUNCTION_ARG:
				{
					verify(t.token == '(');
					next(&lexstate);
					current_function_arg = '(';
					if (t.token != ')')
					{
						do
						{
							if (t.token == TK_NAME)
							{
								current_function_arg += getstr(t.seminfo.ts);
								next(&lexstate);
								if (t.token != ')')
									current_function_arg += ',';
							}
							else
							{
								ostringstream os;
								os << "invalud argument name " << getstr(t.seminfo.ts) << " for function " << current_function_name;
								error(os.str().c_str());
							}
						} while (testnext(&lexstate, ','));
					}
					current_function_arg += ')';
					ps = ST_FUNCTION_BODY;
					nested ++;
				}
				break;

			case ST_FUNCTION_BODY:
				{
					verify(nested == 3);
					ostringstream os;
					auto prev = lexstate;
					bool registered = false;
					if (prev.t.token == '.')
						prev.t.token = TK_DO;
					string callname;
					while (nested >= 3)
					{
						if (lexstate.t.token == TK_DO || lexstate.t.token == TK_IF /*|| lexstate.t.token == TK_FOR*/ || lexstate.t.token == TK_BEGIN || lexstate.t.token == TK_FUNCTION)
						{
							//cout << ">>>" << endl;
							nested++;
						}
						else if (lexstate.t.token == TK_END)
						{
							//cout << "<<<" << endl;
							nested--;
						}

						if (!callname.empty())
						{
							lookahead(&lexstate);
							if (lexstate.lookahead.token == '(')
							{
								//RegisterUsedFunction(callname);
								registered = true;
							}
							callname.clear();
						}
						else if (lexstate.t.token == '(')
						{
							//if (!registered && prev.t.token == TK_NAME)
							//	RegisterUsedFunction(getstr(prev.t.seminfo.ts));
							registered = false;
						}

						if (lexstate.t.token == '.')
						{
							ostringstream fname;
							lookahead(&lexstate);
							FormatTokens(fname, prev.t, defines);
							fname << '.';
							FormatTokens(fname, lexstate.lookahead, defines);
							callname = fname.str();
						}

						FormatTokens(os, lexstate.t, defines);
						os << ' ';

						if (nested == 2)
							break;
						prev = lexstate;
						next(&lexstate);
						//cout << lexstate.t << ' ' << lexstate.linenumber << ' ' << lexstate.lastline << endl;
						if (lexstate.linenumber != lexstate.lastline)
							os << endl;

					}
					ps = ST_WHENLIST_OR_FUNCTION;
					all_functions += ',';
					all_functions += current_function_name;
					all_functions += "= function ";
					all_functions += current_function_arg;
					all_functions += os.str();
					cout << "FUNCTION " << current_function_name << current_function_arg << endl;
				}
				break;

		} // end of switch
	}

	verify(nested==0);
	for (auto it = used_state_name_map.begin(); it != used_state_name_map.end(); ++it) {
		if (define_state_name_set.find(it->second) == define_state_name_set.end())
			errorline(it->first, ("state name not found : " + it->second).c_str());
	}

	if (!define_state_name_set.empty()) {
		auto path = output_path + "/state";
		if (0 != mkdir(path.c_str(), S_IRWXU)) {
			if (errno != EEXIST) {
				perror("cannot create directory");
				exit(1);
			}
		}

		path += '/' + quest_name;

		ofstream ouf(path.c_str());
		ouf << quest_name << "={[\"start\"]=0";

		std::unordered_map<std::string, int> state_crc;
		std::unordered_set<int> crc_set;

		state_crc["start"] = 0;
		for (const auto& name : define_state_name_set) {
			int crc = (int)CRC32(name.c_str());
			if (crc_set.find(crc) == crc_set.end()) {
				crc_set.insert(crc);
			} else {
				++crc;
				while (crc_set.find(crc) != crc_set.end())
					++crc;

				printf("WARN: state CRC conflict occur! state index may differ in next compile time.\n");
				crc_set.insert(crc);
			}

			state_crc.insert(std::make_pair(name, crc));
		}

		int idx = 1;
		for (const auto& name : define_state_name_set) {
			if (name != "start") {
				ouf << ",[\"" << name << "\"]=" <<state_crc[name];
				++idx;
			}
		}

		// quest function들을 기록
		ouf << all_functions;
		ouf << "}";
	}

	if (!start_condition.empty()) {
		auto path = output_path + "/begin_condition";
		if (0 != mkdir(path.c_str(), S_IRWXU)) {
			if (errno != EEXIST) {
				perror("cannot create directory");
				exit(1);
			}
		}

		path += '/';
		path += quest_name;

		ofstream ouf(path.c_str());
		ouf << "return " << start_condition;
		ouf.close();
	}

	for (const auto& p : state_arg_script_map) {
		const auto path = MakeWhenPath(output_path, p.first);

		for (const auto& p2 : p.second) {
			for (int i = 0; i != p2.second.size(); ++i) {
				std::ostringstream os;
				os << i;

				std::string path2 = path + quest_name + "." + p2.first + "." + os.str() + ".";

				WriteStringToFile(path2 + "script", p2.second[i].script);

				{
					std::ofstream ouf((path2 + "when").c_str());
					if (!p2.second[i].when_condition.empty()) {
						ouf << "return ";
						std::copy(p2.second[i].when_condition.begin(),
						          p2.second[i].when_condition.end(),
						          std::ostreambuf_iterator<char>(ouf));
					}
				}

				{
					std::ofstream ouf((path2 + "arg").c_str());
					std::copy(p2.second[i].when_argument.begin() + 1,
					          p2.second[i].when_argument.end(),
					          std::ostreambuf_iterator<char>(ouf));
				}
			}
		}
	}

	for (const auto& p : state_script_map) {
		const auto path = MakeWhenPath(output_path, p.first);
		for (const auto& p2 : p.second)
			WriteStringToFile(path + quest_name + "." + p2.first, p2.second);
	}
}

// Additional command line parser which interprets '@something' as a
// option "config-file" with the value "something"
std::pair<std::string, std::string> at_option_parser(const std::string& s)
{
	if ('@' == s[0])
		return std::make_pair("response-file", s.substr(1));
	else
		return std::pair<std::string, std::string>();
}

void parse_arguments(int ac, char* av[],
                     std::string& output_path, std::string& stamp,
                     std::vector<std::string>& sources)
{
	boost::program_options::options_description desc("Allowed options");
	desc.add_options()
	    ("help", "produce a help message")
	    ("output-path", boost::program_options::value<std::string>(), "output path")
	    ("source-file", boost::program_options::value<std::vector<std::string>>()->composing(), "sources")
	    ("stamp", boost::program_options::value<std::string>(), "stamp file")
	    ("response-file", boost::program_options::value<std::string>(), "can be specified with '@name'")
	;

	boost::program_options::positional_options_description p;
	p.add("source-file", -1);

	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::command_line_parser(ac, av).options(desc).
	          positional(p).extra_parser(at_option_parser).run(), vm);

	if (vm.count("help"))
		std::cout << desc;

	if (vm.count("response-file")) {
		// Load the file and tokenize it
		std::ifstream ifs(vm["response-file"].as<std::string>().c_str());
		if (!ifs)
			throw std::runtime_error("Failed to open response-file");

		// Read the whole file into a string
		std::stringstream ss;
		ss << ifs.rdbuf();
		std::string data = ss.str();

		// Split the file content
		std::vector<std::string> args;
		boost::trim_if(data, boost::is_any_of("\r\n"));
		boost::split(args, data, boost::is_any_of("\r\n"), boost::token_compress_on);

		// Parse the file and store the options
		boost::program_options::store(boost::program_options::command_line_parser(args).options(desc).
		          positional(p).run(), vm);
	}

	if (vm.count("source-file"))
		sources = vm["source-file"].as<std::vector<std::string>>();

	if (vm.count("stamp"))
		stamp = vm["stamp"].as<std::string>();

	if (vm.count("output-path"))
		output_path = vm["output-path"].as<std::string>();
}

extern "C" int main(int argc, char* argv[])
{
	std::string output_path, stamp_file;
	std::vector<std::string> sources;

	// TODO(tim): UTF-8 support for boost.filesystem

	try {
		parse_arguments(argc, argv, output_path, stamp_file, sources);

		if (!output_path.empty() && !sources.empty()) {
			fs::remove_all(output_path);
			fs::create_directories(output_path);

			L = lua_open();
			luaX_init(L);

			for (const auto& src : sources)
				parse(output_path, src.c_str());

			lua_close(L);
		}

		if (!stamp_file.empty())
			WriteStringToFile(stamp_file, std::string());
	} catch (std::exception& e) {
		cout << e.what() << "\n";
		return 1;
	}

	return 0;
}




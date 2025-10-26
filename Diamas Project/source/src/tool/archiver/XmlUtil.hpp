#pragma once
#include <filesystem>
#include <stack>
#include <boost/algorithm/string.hpp>
/*
 * Get the list of all files in given directory and its sub directories.
 *
 * Arguments
 * 	dirPath : Path of directory to be traversed
 * 	dirSkipList : List of folder names to be skipped
 *
 * Returns:
 * 	vector containing paths of all the files in given directory and its sub directories
 *
 */

namespace filesys = std::filesystem;

inline std::vector<std::string> getAllFilesInDir(const std::string &dirPath, 	const std::string prefix = "")
{
 
	// Create a vector of string
	std::vector<std::string> listOfFiles;
	try {
		// Check if given path exists and points to a directory
		if (filesys::exists(dirPath) && filesys::is_directory(dirPath))
		{
			// Create a Recursive Directory Iterator object and points to the starting of directory
			filesys::recursive_directory_iterator iter(dirPath);
 
			// Create a Recursive Directory Iterator object pointing to end.
			filesys::recursive_directory_iterator end;
 
			// Iterate till end
			while (iter != end)
			{

				
				if(!filesys::is_directory(iter->path()))
					listOfFiles.push_back(boost::replace_all_copy(iter->path().generic_u8string(), prefix, ""));
				

				std::error_code ec;
				// Increment the iterator to point to next entry in recursive iteration
				iter.increment(ec);
				if (ec) {
					std::cerr << "Error While Accessing : " << iter->path().generic_u8string() << " :: " << ec.message() << '\n';
				}
			}
		}
	}
	catch (std::system_error & e)
	{
		std::cerr << "Exception :: " << e.what();
	}
	return listOfFiles;
}

inline bool wildcard(const char* text, const char* pattern)
{
	const char* cp = nullptr;
	const char* mp = nullptr;

	while ((*text) && (*pattern != '*')) {
		if ((*pattern != *text) && (*pattern != '?')) {
			return false;
		}
		++pattern;
		++text;
	}

	while (*text) {
		if (*pattern == '*') {
			if (!*++pattern) {
				return true;
			}
			mp = pattern;
			cp = text + 1;
		}
		else if ((*pattern == *text) || (*pattern == '?')) {
			++pattern;
			++text;
		}
		else {
			pattern = mp;
			text = cp++;
		}
	}

	while (*pattern == '*') {
		++pattern;
	}

	return !*pattern;
}


inline bool replace(std::string& str, const std::string& from, const std::string& to)
{
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}

inline void NormalizeFileName(std::string& input, std::string& prefix)
{
	if (prefix.length() && prefix[prefix.length() - 1] != '\\' && prefix[prefix.length() - 1] != '/') {
		prefix += "/";
		for (char& i : prefix) {
			if (i == '\\') {
				i = '/';
			}
		}
	}
	if (input[input.length() - 1] != '\\' && input[input.length() - 1] != '/') {
		input += "/";
	}
	for (char& i : input) {
		if (i == '\\') {
			i = '/';
		}
	}
}


struct XmlGenerator
{
	void operator()(const std::string& out, std::string input, std::string prefix, const std::string& archive,
					const std::vector<std::string>& ignores, const std::vector<std::pair<std::string, std::string>>& patches,
					const std::vector<std::string>& adds)
	{
		NormalizeFileName(input, prefix);

		std::cout << input << '\n';

		std::vector<std::string> files = getAllFilesInDir(input, input);
		std::vector<std::string> filesAdd;
		std::vector<std::string> filesRemove;

		for (auto& file : files) {
			for (auto& add : adds) {
				if (wildcard(file.c_str(), add.c_str())) {
					filesAdd.push_back(file);
					break;
				}
			}

			for (auto& ignore : ignores) {
				if (wildcard(file.c_str(), ignore.c_str())) {
					filesRemove.push_back(file);
					break;
				}
			}
		}

		files.erase(remove_if(begin(files),
							  end(files),
							  [&](auto x){
								  return find(begin(filesRemove), end(filesRemove), x) != end(filesRemove);
							  }),
					end(files));


		files.insert(files.end(), filesAdd.begin(), filesAdd.end());

		std::ofstream o(out.c_str(), std::ofstream::out);
		if (o) {
			o << "<ScriptFile>\n";
			o << "\t<CreateEterPack ArchivePath=\"" << archive << "\"> \n";

			for (auto& file : files) {
				std::string fn = file;
				if (fn.length() && fn[0] == '\\' || fn[0] == '/') {
					fn.erase(0, 1);
				}

				for (const auto& patche : patches) {
					if (replace(fn, patche.first, patche.second))
						break;
				}

				o << "\t\t<File ArchivedPath=\"" << (prefix + fn) << "\"><![CDATA[" << (input + file) << "]]></File>\n";
			}
			o << "\t</CreateEterPack>\n";
			o << "</ScriptFile>";
			o.close();
		}
	}

	std::vector<std::string> parse(const std::string& basepath)
	{
		std::vector<std::string> result;
		std::stack<std::string> directories;
		directories.push(basepath);

		HANDLE find = INVALID_HANDLE_VALUE;
		WIN32_FIND_DATAA ffd;

		while (!directories.empty()) {
			std::string path = directories.top();
			directories.pop();

			std::string curpath = path + "\\*";
			find = FindFirstFileA(curpath.c_str(), &ffd);
			if (find == INVALID_HANDLE_VALUE) {
				break;
			}
			do {
				if (strcmp(ffd.cFileName, ".") != 0 && strcmp(ffd.cFileName, "..") != 0) {
					if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
						directories.push(path + "\\" + ffd.cFileName);
					}
					else {
						std::string fname = path + "\\" + ffd.cFileName;
						fname.erase(0, basepath.length());
						if (fname[0] == '\\' || fname[0] == '/') {
							fname.erase(0, 1);
						}
						for (size_t i = 0; i < fname.length(); ++i) {
							if (fname[i] == '\\') {
								fname[i] = '/';
							}
						}
						result.push_back(fname);
					}
				}
			}
			while (FindNextFileA(find, &ffd) != 0);
			FindClose(find);
		}
		return result;
	}
};

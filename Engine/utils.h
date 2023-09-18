#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <locale>
#include <codecvt>


#define RANGE(c)  std::begin(c), std::end(c)
#define RRANGE(c) std::rbegin(c), std::rend(c)

namespace StrUtil
{
	bool IsNumber(const std::string& s);

	inline bool  ParseBool(const std::string& s) { bool b; std::istringstream(s) >> std::boolalpha >> b; return b; }
	inline int   ParseInt(const std::string& s) { return std::atoi(s.c_str()); }
	inline float ParseFloat(const std::string& s) { return static_cast<float>(std::atof(s.c_str())); }

	std::vector<std::string> split(const char* s, char c = ' ');
	std::vector<std::string> split(const std::string& s, char c = ' ');
	std::vector<std::string> split(const std::string& s, const std::vector<char>& delimiters);
}
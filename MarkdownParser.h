#pragma once
#include "lex_parse.h"
#include <sstream>

void parse_markdown(std::wstring& str)
{
	std::wostringstream wos;
	auto scanned = scanner(str);
	parse_fromlex(wos, std::begin(scanned), std::end(scanned));
	str.clear();
	str.append(wos.str());
}

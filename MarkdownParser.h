#pragma once
#include "lex_parse.h"
#include <sstream>
#include <regex>

void parse_markdown(std::wstring& str)
{
	//ת��һ���ַ���
	//str = std::regex_replace(str, std::wregex(L"<"), L"&lt;");
	//str = std::regex_replace(str, std::wregex(L">"), L"&gt;");

	std::wostringstream wos;
	auto scanned = scanner(str);
	parse_fromlex(wos, std::begin(scanned), std::end(scanned));
	str = wos.str();
}

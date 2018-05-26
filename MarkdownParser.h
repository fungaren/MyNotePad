#pragma once
#include "lex_parse.h"
#include <sstream>
#include <regex>
#include <cstdlib>
#include <iostream>
void parse_markdown(std::wstring& str)
{
	//std::wregex regex_table_align(L"^([:\\s]?-+[:\\s]?\\|?)+$");
	//bool re = std::regex_match(str, regex_table_align);
	//int j = 5;
	//clock_t begin = clock();

	std::wostringstream wos;
	//clock_t begin = clock();
	auto scanned = scanner(str);
	parse_fromlex(wos, std::begin(scanned), std::end(scanned));

	//clock_t end = clock();
	//int t = double(end - begin) * 1000 / CLOCKS_PER_SEC;
	//wos << t;
	str = wos.str();
}

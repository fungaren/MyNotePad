#pragma once
#include "lex_parse.h"
#include <sstream>
#include <regex>
#include <cstdlib>
#include <iostream>
#include <iomanip>
//#define SHOW_PARSEING_TIME
void parse_markdown(std::wstring& str)
{
	//std::wregex regex_table_align(L"^([:\\s]?-+[:\\s]?\\|?)+$");
	//bool re = std::regex_match(str, regex_table_align);
	//int j = 5;
#ifdef SHOW_PARSEING_TIME
	clock_t begin = clock();
#endif

	std::wostringstream wos;

	auto scanned = scanner(str);
	parse_fromlex(wos, std::begin(scanned), std::end(scanned));

#ifdef SHOW_PARSEING_TIME
	clock_t end = clock();
	double t = double(end - begin) / CLOCKS_PER_SEC;
	wos << L"<br><p>”√ ±£∫" << std::setprecision(2) << t << L" s</p>";
#endif // SHOW_PARSEING_TIME
	str = wos.str();
}

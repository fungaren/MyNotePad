#pragma once
#include "lex_parse.h"
#include <sstream>
#include <regex>
#include <stdlib.h>
#include <iostream>
void parse_markdown(std::wstring& str)
{
	/*std::wstring d(L" `55`  ");
	auto i = trim(d, 0, d.size());*/
	//转义一下字符串
	//str = std::regex_replace(str, std::wregex(L"<"), L"&lt;");
	//str = std::regex_replace(str, std::wregex(L">"), L"&gt;");
	std::wostringstream wos;
	clock_t begin = clock();
	auto scanned = scanner(str);
	parse_fromlex(wos, std::begin(scanned), std::end(scanned));
	clock_t end = clock();
	int t = double(end - begin) * 1000 / CLOCKS_PER_SEC;
	wos << t;
	str = wos.str();
	
}

#pragma once
#include "lex_parse.h"
std::wostream &parse_fromlex(std::wostream &os, std::list<Item>::iterator beg, std::list<Item>::iterator end);
std::wostream &writeInner(std::wostream &os, const std::wstring &data);
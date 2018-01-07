#pragma once

#include "parser.h"
#include <sstream>
// make sure str begin with '\n'
void parse_markdown(std::wstring& str) {
	/*_ASSERT(str[0] == '\n');
	str = std::regex_replace(str, std::wregex(L"<"), L"&lt;");
	str = std::regex_replace(str, std::wregex(L">"), L"&gt;");
	str = std::regex_replace(str, std::wregex(L"\\n&gt;(.+)"), L"\n<blockquote>$1</blockquote>");

	str = std::regex_replace(str, std::wregex(L"\\n\\d+\\. (.+)"), L"\n<ol><li>$1</li></ol>");
	str = std::regex_replace(str, std::wregex(L"\\n[\\*\\+-] (.+)"), L"\n<ul><li>$1</li></ul>");

	str = std::regex_replace(str, std::wregex(L"###### ?(.+)"), L"<h6>$1</h6>");
	str = std::regex_replace(str, std::wregex(L"##### ?(.+)"), L"<h5>$1</h5>");
	str = std::regex_replace(str, std::wregex(L"#### ?(.+)"), L"<h4>$1</h4>");
	str = std::regex_replace(str, std::wregex(L"### ?(.+)"), L"<h3>$1</h3>");
	str = std::regex_replace(str, std::wregex(L"## ?(.+)"), L"<h2>$1</h2>");
	str = std::regex_replace(str, std::wregex(L"# ?(.+)"), L"<h1>$1</h1>");

	str = std::regex_replace(str, std::wregex(L"\\n([^<].*)"), L"\n<p>$1</p>");

	str = std::regex_replace(str, std::wregex(L"\\*\\*(.+)\\*\\*"), L"<b>$1</b>");
	str = std::regex_replace(str, std::wregex(L"\\*(.+)\\*"), L"<i>$1</i>");
	str = std::regex_replace(str, std::wregex(L"!\\[(.*)\\]\\((.*)\\)"), L"<img src='$2' alt='$1' align='middle'>");
	str = std::regex_replace(str, std::wregex(L"\\[(.*)\\]\\((.*)\\)"), L"<a href='$2' target='_blank'>$1</a>");

	str = std::regex_replace(str, std::wregex(L"<p>```(.+)[\\r\\n]?</p>([^`]+)```"), L"<pre lang='$1'>$2</p></pre>");
	str = std::regex_replace(str, std::wregex(L"`(.*)`"), L"<code>$1</code>");

	str = std::regex_replace(str, std::wregex(L"</ol>\\n<ol>"), L"");
	str = std::regex_replace(str, std::wregex(L"</ul>\\n<ul>"), L"");*/
	std::wostringstream wos;
	auto scanned = scanner(str);
	parse_fromlex(wos, std::begin(scanned), std::end(scanned));
	str.clear();
	str.append(wos.str());
}

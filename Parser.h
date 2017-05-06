#pragma once

#include <string>
#include <regex>
#include <list>


class section {

	bool isUnorderedList(std::wstring const& input) {
		if (input.size() < 2) return false;
		return ((input[0] == '*' || input[0] == '+' || input[0] == '-') && input[1] == ' ');
	}

	bool isOrderedList(std::wstring const& input) {
		if (input.size() < 3) return false;
		return ((input[0] >= '0' && input[0] <= '9') && input[1] == '.' && input[2] == ' ');
	}

	bool isBlockQuote(std::wstring const& input) {
		if (input.size() < 2) return false;
		return (input[0] == '>' && input[1] == ' ');
	}

	bool isHorizonLine(std::wstring const& input) {
		return (input.size() >= 3 && input[0] == '-' && input[1] == '-' && input[2] == '-');
	}

	enum codeType { python, cpp, unknown, notCodeBlock };

	codeType isCodeBlock_begin(std::wstring const& input) {
		std::wregex re_codeBlock(L"``` ?(\\w+)");
		std::wsmatch result;
		if (std::regex_search(input, result, re_codeBlock))
		{
			if (result[1] == L"python") return codeType::python;
			else if (result[1] == L"cpp") return codeType::cpp;
			else return codeType::unknown;
		}
		else return codeType::notCodeBlock;
	}

	bool isCodeBlock_end(std::wstring const& input) {
		return (input == L"```");
	}

	int isHeader(std::wstring const& input) {
		size_t n = input.size();
		if (n>=6 && input.substr(0,7).compare(L"###### ") == 0) return 6;
		else if (n >= 5 && input.substr(0, 6).compare(L"##### ") == 0) return 5;
		else if (n >= 4 && input.substr(0, 5).compare(L"#### ") == 0) return 4;
		else if (n >= 3 && input.substr(0, 4).compare(L"### ") == 0) return 3;
		else if (n >= 2 && input.substr(0, 3).compare(L"## ") == 0) return 2;
		else if (n >= 1 && input.substr(0, 2).compare(L"# ") == 0) return 1;
		else return 0;
	}

	//------------------------------------------

	std::wstring handle_tags(std::wstring const& input)
	{
		std::wstring result;

		std::wregex re_img(L"!\\[([^\\]]*)\\]\\(([^\\]]*)\\)");
		result = std::regex_replace(input, re_img, L"&nbsp;<img src=\"$2\" alt=\"$1\" align=\"middle\">");

		std::wregex re_a(L"\\[([^\\]]*)\\]\\(([^\\]]*)\\)");
		result = std::regex_replace(result, re_a, L"<a href=\"$2\" target=\"_blank\">$1</a>");

		std::wregex re_sup(L"\\[([^\\]]*)\\]\\[([^\\]]*)\\]");
		result = std::regex_replace(result, re_sup, L"$1<sup>$2</sup>");

		std::wregex re_b(L"\\*\\*([^\\*]*)\\*\\*");
		result = std::regex_replace(result, re_b, L"<b>$1</b>");

		std::wregex re_i(L"\\*([^\\*]*)\\*");
		result = std::regex_replace(result, re_i, L"<i>$1</i>");

		std::wregex re_code(L"`([^`]*)`");
		result = std::regex_replace(result, re_code, L"<code>$1</code>");

		return result;
	}

	std::wstring handle_escape_lt(std::wstring const& input)
	{
		std::wstring out;
		for (wchar_t c : input) {
			if (c != L'<')
				out += c;
			else
				out += L"&lt;";
		}
		return out;
	}

	std::wstring handle_escape_gt(std::wstring const& input)
	{
		std::wstring out;
		for (wchar_t c : input) {
			if (c != L'>')
				out += c;
			else
				out += L"&gt;";
		}
		return out;
	}

	std::wstring handle_p(std::wstring const& input) {
		return (L"<p>" + input + L"</p>");
	}

	std::wstring handle_h(std::wstring const& input, int n) {
		switch (n) {
		case 1:return (L"<h1>" + input.substr(2) + L"</h1>");
		case 2:return (L"<h2>" + input.substr(3) + L"</h2>");
		case 3:return (L"<h3>" + input.substr(4) + L"</h3>");
		case 4:return (L"<h4>" + input.substr(5) + L"</h4>");
		case 5:return (L"<h5>" + input.substr(6) + L"</h5>");
		case 6:return (L"<h6>" + input.substr(7) + L"</h6>");
		}
		return input;
	}

	std::wstring handle_ul(std::wstring const& input) {
		std::wstring str;
		std::wregex re_ul(L"[\\*\\+-] (.*)");
		std::wsmatch result;
		auto begin = input.begin();
		auto end = input.end();
		while (std::regex_search(begin, end, result, re_ul))
		{
			str += L"<li>";
			str += result[1];
			str += L"</li>";
			begin = result[0].second;
		}
		return str;
	}

	std::wstring handle_ol(std::wstring const& input) {
		std::wstring str;
		std::wregex re_ol(L"\\d+\\. (.*)");
		std::wsmatch result;
		auto begin = input.begin();
		auto end = input.end();
		while (std::regex_search(begin, end, result, re_ol))
		{
			str += L"<li>";
			str += result[1];
			str += L"</li>";
			begin = result[0].second;
		}
		return str;
	}

	std::wstring handle_blockquote(std::wstring const& input) {
		return L"<blockquote>" + input.substr(2) + L"</blockquote>";
	}

	std::wstring handle_pre(codeType ct) {
		switch (ct) {
		case section::python: return L"<pre class=\"lang:python\">";
		case section::cpp: return L"<pre class=\"lang:cpp\">";
		case section::unknown: return L"<pre class=\"lang:unknown\">";
		}
		return L"";
	}

	//------------------------------------------------------

	const std::wstring raw_str;

public:

	enum section_type {
		Paragraph, UnorderedList, OrderedList, BlockQuote, HorizonLine, Header,
		CodeBlock_begin, CodeBlock_end, CodeBlock
	};

	union section_param {
		int header_n;
		codeType codeBlock_type;
	};

private:

	section_type type;
	section_param param;

public:

	const section_type& getType() { return type; }
	const std::wstring& getRawStr() { return raw_str; }
	const section_param& getParam() { return param; }

	// constructor
	section(std::wstring& str, section_type last) :raw_str(str) {
		// cross-section block
		if (last == CodeBlock  || last == CodeBlock_begin) {
			if (isCodeBlock_end(str))
				type = CodeBlock_end;
			else
				type = CodeBlock;
		}
		// get type
		else if (isHorizonLine(str))
			type = HorizonLine;
		else if (isUnorderedList(str))
			type = UnorderedList;
		else if (isOrderedList(str))
			type = OrderedList;
		else if (isBlockQuote(str))
			type = BlockQuote;
		else if ((param.codeBlock_type = isCodeBlock_begin(str)) != codeType::notCodeBlock)
			type = CodeBlock_begin;
		else if ((param.header_n = isHeader(str)) != 0)
			type = Header;
		else
			type = Paragraph;
	}

	std::wstring getHTMLStr() {
		if (type == CodeBlock_begin)
			return handle_pre(param.codeBlock_type);
		else if (type == CodeBlock) {
			auto& str = handle_escape_gt(handle_escape_lt(raw_str));
			str += '\n';
			return str;
		}
		else if (type == CodeBlock_end)
			return L"</pre>";
		else if (type == HorizonLine)
			return L"<hr>";
		
		auto& str = handle_escape_lt(raw_str);
		switch (type) {
		case section::Paragraph: str = handle_p(str); break;
		case section::BlockQuote: str = handle_blockquote(str); break;
		case section::UnorderedList: str = handle_ul(str); break;	// add <ul></ul> manually
		case section::OrderedList: str = handle_ol(str); break;		// add <ol></ol> manually
		case section::Header: str = handle_h(str, param.header_n); break;
		}
		return handle_tags(str);
	}
};

//------------------------------------------------------

template <class T>
void spliter(std::list<section>& sections, T const& str) {

	T::const_iterator section_begin = str.begin(), section_end;
	section::section_type last = section::Paragraph;

	auto i = str.begin();
	while (i != str.end()) {
		if (*i == '\n') {
			// new section
			sections.push_back(section(std::wstring(section_begin, i), last));
			last = (--sections.end())->getType();
			section_begin = section_end = i;
			++section_begin;
		}
		++i;
	}
}
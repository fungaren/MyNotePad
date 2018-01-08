#include "stdafx.h"
#include "lex_parse.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <utility>
#include <regex>
#include <algorithm>

Item::Item(const std::wstring &data, MD_TOKEN token, MD_ITEM itemtype, const std::wstring &tag)
	:m_token(token),
	m_mditemtype(itemtype),
	m_tag(tag)
{ 
//转义数据
	if (token != MD_TOKEN::HTML)//不处理HTML，因为他们已经被转义了
	{
		m_data = std::regex_replace(data, REGEX_LT, L"&lt;");
		m_data = std::regex_replace(m_data, REGEX_GT, L"&gt;");
	}
}

Item::Item(const Item &rhs)
	: m_data(rhs.m_data),
	m_token(rhs.m_token),
	m_mditemtype(rhs.m_mditemtype),
	m_tag(rhs.m_tag)
{ }

std::wostream &Item::operator<<(std::wostream &os) const
{
	os << "<" << m_token << "," << m_data << "," << m_mditemtype << ">";
	return os;
}

void Item::setData(const std::wstring &str)
{
	m_data = str; 
}

const std::wstring &Item::getData() const
{ 
	return m_data;
}

void Item::setToken(MD_TOKEN token)
{
	m_token = token;
}

const MD_TOKEN &Item::getToken() const
{
	return m_token;
}

void Item::setItemType(MD_ITEM itemtype)
{
	m_mditemtype = itemtype;
}

const MD_ITEM &Item::getItemType() const
{
	return m_mditemtype;
}

void Item::setTag(const std::wstring & tag)
{
	m_tag = tag;
}

const std::wstring & Item::getTag() const
{
	return m_tag;
}

int determineData(MD_TOKEN tokenType, const std::wstring &str, int start = 0)
{
	int beg=start;
	switch (tokenType)
	{
	case MD_TOKEN::HEADER1:
	case MD_TOKEN::DELIMITER:
	case MD_TOKEN::UNORDERED_LIST:
	case MD_TOKEN::ORDERED_LIST:
	case MD_TOKEN::QUTOE:
		for (beg = start; beg < str.length(); ++beg)
			if (str[beg] == '\t' || str[beg] == ' ')
				break;
		return beg + 1;
	case MD_TOKEN::TABLE_ITEM:
		//可能需要屏蔽转义字符，这个用来确定边界
		beg = start;
		for (; beg < str.length();)
		{
			if (str[beg] == '\\')
			{
				beg = beg + 2;//这是一个转义字符
				continue;
			}
			if (str[beg] == '|')
				break;
			++beg;
		}
		return beg == str.length() ? std::wstring::npos : beg;
	default:
		break;
	}
	return beg;//如果错误的话
}

std::wstring trim(const std::wstring &str, int start, int count)
{
	if (count == 0)
		return str;

	int f = str.find_first_not_of(L"\t ", start, count);
	count = count - (f - start) - 1;
	while (count > 0)
	{
		wchar_t ch = str[f + count];
		if (ch == '\t' || ch == ' ')
			--count;
		else
			break;
	}
	return str.substr(f, count + 1);
}

std::list<Item> scanner(const std::wstring &str)
{
	std::list<Item> items;
	std::wistringstream istringStream(str);
	std::wstring line;
	bool multilines;

	//跨行使用的上下文信息
	MD_TOKEN token;
	std::wstring data;
	std::wstring html_tag;

	multilines = false;
	while (std::getline(istringStream, line))
	{
		int index = 0;
		while (index < line.length())
		{

			//解析每一个字符
			wchar_t ch = line[index];
			if (ch == '`')
			{
				if (multilines == false)
				{
					data = L"";
					//优先处理代码项目，也包括普通的代码行引用
					int dl = line.find_first_not_of('`');
					if (dl == std::wstring::npos || dl == 3)
					{
						if (dl == 3)
						{
							//可能指定了代码的语言
							html_tag = L"class=" + line.substr(3);
						}
						index = line.length();
						//代码块
						multilines = true;
						token = MD_TOKEN::CODE;
						continue;//不需要将标志写入
					}
				}
				else
				{
					index = line.length();
					//是终止
					multilines = false;
					items.emplace_back(data, token, MD_ITEM::LINE, html_tag);
					data = L"";//清空
					html_tag = L"";
					continue;
				}
			}
			if (multilines == true)
			{
				data.append(line + L"\n");
				index = line.length();
				continue;
			}
			if (ch == '#')
			{
				int beg = determineData(MD_TOKEN::HEADER1, line);
				int syms = line.find_first_not_of('#');
				items.emplace_back(line.substr(beg), static_cast<MD_TOKEN>(syms - 1), MD_ITEM::LINE);
				index = line.length();
			}
			else if (ch == '-')
			{
				//分隔符或者列表的某一项
				if (line.find_last_not_of('-') == std::wstring::npos)
				{
					//分隔符
					items.emplace_back(L"", MD_TOKEN::DELIMITER, MD_ITEM::LINE);
					index = line.length();
				}
				else {
					int beg = determineData(MD_TOKEN::UNORDERED_LIST, line);
					items.emplace_back(line.substr(beg), MD_TOKEN::UNORDERED_LIST, MD_ITEM::LINE);
					index = line.length();
				}
			}
			else if (ch >= '0' && ch <= '9')
			{
				//以数字开头，有可能是序列表
				int beg = determineData(MD_TOKEN::ORDERED_LIST, line);
				items.emplace_back(line.substr(beg), MD_TOKEN::ORDERED_LIST, MD_ITEM::LINE);
				index = line.length();
			}
			else if (ch == '[')
			{
				//内引用格式的标题信息，可以使用正则表达式匹配
				auto regex = std::wregex(L"\\[(.*)\\]\\((.*)\\)");
				std::wostringstream os;
				std::regex_replace(std::ostreambuf_iterator<wchar_t>(os),
					line.begin() + index, line.end(),
					regex, L"<a href='$2' target='_blank'>$1</a>");
				std::wstring htmlcode(os.str());
				items.emplace_back(htmlcode, MD_TOKEN::HTML, MD_ITEM::LINE);
				index = line.length();
			}
			else if (ch == '!')
			{
				//图片的引用格式，可以使用正则表达式匹配
				auto regex = std::wregex(L"!\\[(.*)\\]\\((.*)\\)");
				std::wostringstream os;
				std::regex_replace(std::ostreambuf_iterator<wchar_t>(os),
					line.begin() + index, line.end(),
					regex,
					L"<img src='$2' alt='$1' align='middle'>");
				std::wstring htmlcode(os.str());
				items.emplace_back(htmlcode, MD_TOKEN::HTML, MD_ITEM::LINE);
				index = line.length();
			}
			else if (ch == '>')
			{
				//引用
				int beg = determineData(MD_TOKEN::QUTOE, line);
				items.emplace_back(line.substr(beg), MD_TOKEN::QUTOE, MD_ITEM::LINE);
				index = line.length();
			}
			else if (ch == '|')
			{
				//表格，可能是表头也可能是数据项、对其选项
				if (items.back().getToken() == MD_TOKEN::TABLE_COLUMN_LEFT ||
					items.back().getToken() == MD_TOKEN::TABLE_COLUMN_CENTER ||
					items.back().getToken() == MD_TOKEN::TABLE_COLUMN_RIGHT)
				{
					//对齐控制
					int sp;
					int start = index;
					//使用反向迭代器
					auto checked_header = items.rbegin();
					for (; checked_header != items.rend(); ++checked_header)
					{
						MD_TOKEN token = checked_header->getToken();
						if (token == MD_TOKEN::TABLE_COLUMN_CENTER ||
							token == MD_TOKEN::TABLE_COLUMN_LEFT ||
							token == MD_TOKEN::TABLE_COLUMN_RIGHT)
							continue;
						break;
					}
					std::list<Item>::iterator head_iter = items.end();
					//确定了第一个表头的位置或者是头前位置
					if (checked_header != items.rend())
						head_iter = checked_header.base();//前向的迭代器
					do {
						sp = determineData(MD_TOKEN::TABLE_ITEM, line, start + 1);
						//start、sp包裹一对 | |
						if (line[start + 1] == ':' && line[sp - 1] == ':')
							head_iter->setToken(MD_TOKEN::TABLE_COLUMN_CENTER);
						else if (line[sp - 1] == ':')
							head_iter->setToken(MD_TOKEN::TABLE_COLUMN_RIGHT);
						else if (line[start + 1] == ':')
							;//默认左对齐
						else
						{
							auto content = trim(line, start + 1, sp - start - 1);
							if (content.find_last_not_of(L"-") != std::wstring::npos)
								items.emplace_back(content, head_iter->getToken(), MD_ITEM::NESTED);
						}
						start = sp;
						++head_iter;
					} while (start < line.length() - 1 && sp != std::wstring::npos);
					index = line.length();
				}
				else
				{
					//表头
					int sp;
					int start = index;
					do {
						sp = determineData(MD_TOKEN::TABLE_ITEM, line, start + 1);
						//解析内容
						auto content = trim(line, start + 1, sp - start - 1);

						items.emplace_back(content, MD_TOKEN::TABLE_COLUMN_LEFT, MD_ITEM::LINE);
						start = sp;
					} while (start < line.length() - 1 && sp != std::wstring::npos);
					index = line.length();
				}
			}
			else
			{
				//一个其他的文本
				std::wregex regex = std::wregex(L"([`\\*`]){1,3}([^\\`*]*)([`\\*]){1,3}");
				//是否能够匹配
				bool res = std::regex_search(line.begin() + index, line.end(), regex);
				if (!res)
				{
					//不能匹配的话作为一行
					items.emplace_back(line.substr(index), MD_TOKEN::DATA, MD_ITEM::LINE);
					//itemtype = MD_ITEM::NESTED;
				}
				else
				{
					//这是一行的内容
					MD_ITEM itemtype = MD_ITEM::LINE;//只有文本的第一个字段视为整个一行，其余行视为嵌套的
					//寻找所有匹配
					std::wstring suffix;
					std::wsregex_iterator end;
					std::wsregex_iterator iter(line.begin() + index, line.end(), regex);
					for (; iter != end; ++iter) {
						//每一个项目
						if (iter->operator[](1).matched)
						{
							//处理其他的内容
							auto prefix = iter->prefix();
							if (prefix.length() > 0)
							{
								//表示有前缀不为空
								items.emplace_back(prefix.str(), MD_TOKEN::DATA, itemtype);
								itemtype = MD_ITEM::NESTED;
							}
							int symsize = ((*iter)[0].length() - (*iter)[2].length()) / 2;
							switch (symsize)
							{
							case 1:
								//
								//可能是代码或者斜体
								if (*(*iter)[1].first == '`')
									items.emplace_back(iter->operator[](2).str(), MD_TOKEN::CODE, itemtype);
								else
									items.emplace_back(iter->operator[](2).str(), MD_TOKEN::ITALIC, itemtype);
								break;
							case 2:
								items.emplace_back(iter->operator[](2).str(), MD_TOKEN::BOLD, itemtype);
								break;
							case 3:
								items.emplace_back(iter->operator[](2).str(), MD_TOKEN::ITALIC_BOLD, itemtype);
								break;
							}
							//设置最后一个后缀
							if (iter->suffix().length() > 0)
								suffix = iter->suffix().str();
							else
								suffix = L"";
							itemtype = MD_ITEM::NESTED;
						}

					}
					if (suffix.compare(L"") != 0)
						items.emplace_back(suffix, MD_TOKEN::DATA, MD_ITEM::NESTED);
				}
				index = line.length();
			}
			//items.emplace_back(data, token, itemtype);
		}
	}
	return items;
}
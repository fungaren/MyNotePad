#include "stdafx.h"
#include "lex_parse.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <utility>
#include <regex>
#include <algorithm>
#include <iomanip>

Item::Item(const std::wstring &data, MD_TOKEN token, MD_ITEM itemtype, const std::wstring &tag)
	:m_token(token),
	m_mditemtype(itemtype),
	m_tag(tag),m_data(data)
{ 
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

size_t determineData(MD_TOKEN tokenType, const std::wstring &str, size_t start)
{
	size_t beg=start;
	switch (tokenType)
	{
	case MD_TOKEN::HEADER1:
	case MD_TOKEN::DELIMITER:
	case MD_TOKEN::UNORDERED_LIST:
	case MD_TOKEN::ORDERED_LIST:
	case MD_TOKEN::QUTOE:
		for (; beg < str.length(); ++beg)
			if (str[beg] == '\t' || str[beg] == ' ')
				break;
		return beg == str.length() ? std::wstring::npos : beg + 1;
	case MD_TOKEN::TABLE_ITEM:
		//可能需要屏蔽转义字符，这个用来确定边界
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

size_t getClosedRegion(const std::wstring & str, wchar_t start_ch, wchar_t end_ch, size_t start)
{
	int count = 0;
	for (; start < str.length();++start)
	{
		if (str[start] == start_ch)
		{
			++count;
		}
		if (str[start] == end_ch)
		{
			--count;
		}
		if (count == 0)
			break;//已经到达边界
	}
	if (start < str.length())
	{
		return start;//返回最终的]的位置
	}
	return std::wstring::npos;
}
std::wstring mdToHTMLDoc(const std::wstring &str)
{
	
	//主要是解决文本中混个HTML语句的问题
	/*
	可能的标签类型<xxx>yyy</xxx>, <xxx />, <xxx>
	*/
	std::wregex regex_htmltag(L"</?[A-Za-z0-9].*>");
	auto tag_begin = std::wsregex_iterator(str.begin(), str.end(), regex_htmltag);
	auto tag_end = std::wsregex_iterator();
	if (tag_begin == tag_end)
	{
		std::wstring temp;
		temp = std::regex_replace(str, REGEX_LT, L"&lt;");
		return std::regex_replace(temp, REGEX_GT, L"&gt;");
	}
	std::wostringstream result;//存在标签需要替换
	ptrdiff_t distan = std::distance(tag_begin, tag_end);
	size_t count = 0;
	std::wstring temp;
	for (; tag_begin != tag_end; ++tag_begin)
	{
		++count;
		temp = std::regex_replace(tag_begin->prefix().str(), REGEX_LT, L"&lt;");
		result << std::regex_replace(temp, REGEX_GT, L"&gt;");
		//标签写入
		auto tag = tag_begin->str();
		if (tag.size() > 8u && tag.substr(0u, 8u).compare(L"<script>") == 0)
		{
			result << L"&lt;script&gt;" << tag.substr(8u, tag.length() - 17) << L"&lt;/script&gt;";
		}
		else
			result << tag;
		if (count == distan)
		{
			//最后一个需要处理suffix
			temp = std::regex_replace(tag_begin->suffix().str(), REGEX_LT, L"&lt;");
			result << std::regex_replace(temp, REGEX_GT, L"&gt;");
		}
	}
	return result.str();
}
std::wstring parse_inner(const std::wstring &str, size_t begin)
{
	//这个函数解析所有的行内的引用
	std::wostringstream result;
	//用于URL连接的行列式的正则表达式
	std::wregex regex_url(L"(\\!)?\\[(.*)\\]\\(([\\S]*)(\\s[\"']([\\S]*)[\"'])?\\)");
	std::wregex regex_url_link(L"\\(([\\S]*)(\\s[\"']([\\S]*)[\"'])?\\)");
	for (size_t index = begin; index < str.length();)
	{
		const wchar_t &ch = str[index];
		//遍历每一个字符
		if (ch == '*')
		{
			//斜体、粗斜体、粗体
			//确定*的个数
			//需要验证边界
			size_t sz = 1;
			for (size_t s = index + 1; s < str.length() && str[s] == '*' && sz < 3; ++s)
				++sz;
			size_t posOfEnd = str.find(std::wstring(sz, '*'), index + sz);
			//是否有固定的范围？
			if (posOfEnd == std::wstring::npos)
			{
				result << str[index++];continue;//作为非法的输入
			}
			//截取内容
			auto &&content = str.substr(index + sz, posOfEnd - index - sz);
			//同时需要对内容进行解析
			content = parse_inner(content, 0u);
			switch (sz)
			{
			case 1:
				//斜体
				result << L"<i>"<<content<<L"</i>";
				break;
			case 2:
				//粗体
				result << L"<strong>" << content << L"</strong>";
				break;
			case 3:
				//粗斜体
				result << L"<strong><i>" << content << L"</i></strong>";
				break;
			}
			index = posOfEnd + sz;
		}
		else if (ch == '`')
		{
			//代码
			size_t sz = 1;
			for (size_t s = index + 1; s < str.length() && str[s] == '`' && sz < 3; ++s)
				++sz;
			size_t posOfEnd = str.find(std::wstring(sz, '`'), index + sz);
			//是否有固定的范围？
			if (posOfEnd == std::wstring::npos)
			{
				result << str[index++]; continue;//作为非法的输入
			}
			//截取内容
			auto &&content = str.substr(index + sz, posOfEnd - index - sz);
			//``形式、``````形式
			result << L"<code>" << content << L"</code>";
			index = posOfEnd + sz;
		}
		else if (ch == '[')
		{
			size_t right_kuohao = getClosedRegion(str, '[', ']', index);
			if (right_kuohao == std::wstring::npos)
			{
				result << ch;
				++index;
				continue;
			}
			size_t left_url = right_kuohao + 1;
			size_t right_url = getClosedRegion(str, '(', ')', left_url);
			if (right_kuohao == std::wstring::npos)
			{
				result << ch;
				++index;
				continue;
			}
			//确定了范围
			std::wsmatch matched;
			if (std::regex_search(std::begin(str) + index + 1, std::begin(str) + right_kuohao, matched, regex_url))
			{
				if (matched[1].matched)
				{
					//图片链接
					std::wsmatch link_matched;
					if (std::regex_search(std::begin(str) + left_url, std::begin(str) + right_url + 1, link_matched, regex_url_link))
					{
						result << L"<a href=\"" << link_matched[1].str() << L"\"";
						if (link_matched[3].matched)
							result << L" title=\"" << link_matched[3].str() << L"\"";
						result << L" ><img src=\"" << matched[3].str() << L"\"";
						if (matched[2].matched)
							result << L" alt=\"" << matched[2].str() << L"\"";
						result << L" /></a>";
						index = right_url + 1;
					}
					else
					{
						result << ch;
						++index;
					}
				}
				else
				{
					//非法的
					result << ch;
					++index;
				}
			}
			else
			{
				std::wsmatch matched_url;
				if (std::regex_search(std::begin(str) + left_url, std::begin(str) + right_url + 1, matched_url, regex_url_link))
				{
					//先解析最普通的URL
					result << L"<a href=\"" << matched_url[1].str() << L"\"";
					if (matched_url[3].matched)
						result << L" title=\"" << matched_url[3].str() << L"\"";
					result << L">" << parse_inner(str.substr(index + 1, right_kuohao - index - 1), 0u) << L"</a>";
					index = right_url + 1;
				}
				else
				{
					result << ch;
					++index;
				}
			}
		}
		else if (ch == '!')
		{
			if (index + 1 < str.length() && str[index + 1] == '[')
			{
				size_t right_kuohao = getClosedRegion(str, '[', ']', index + 1);
				if (right_kuohao == std::wstring::npos)
				{
					result << ch;
					++index;
					continue;
				}
				size_t left_url = right_kuohao + 1;
				size_t right_url = getClosedRegion(str, '(', ')', left_url);
				if (right_kuohao == std::wstring::npos)
				{
					result << ch;
					++index;
					continue;
				}
				std::wsmatch matched_url;
				if (std::regex_search(std::begin(str) + left_url, std::begin(str) + right_url + 1, matched_url, regex_url_link))
				{
					//先解析最普通的URL
					result << L"<img src=\"" << matched_url[1].str() << L"\"";
					if (matched_url[3].matched)
						result << L" title=\"" << matched_url[3].str() << L"\"";
					result << L" alt=\"" << str.substr(index + 2, right_kuohao - index - 2) << L"\" />";
					index = right_url + 1;
				}
				else
				{
					result << ch;
					++index;
				}
			}
			else
			{
				result << ch;
				++index;
			}
		}
		else
		{
			result << str[index++];//不识别的内容
		}
		
	}
	return result.str();
}
std::wstring trim(const std::wstring &str, size_t start, size_t count)
{
	/*if (count == 0)
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
	return str.substr(f, count + 1);*/
	if (count == 0)
		return L"";
	auto &&temp = str.substr(start, count);
	size_t last = temp.find_last_not_of(L"\t ");
	size_t first = temp.find_first_not_of(L"\t ");
	if (first < last)
	{
		return temp.substr(first, last - first + 1);
	}
	else
	{
		if (first == last)
		{
			if(first != std::wstring::npos)
				return temp.substr(first, 1);
		}
	}
	return temp;
}
std::vector<std::wstring> split(const std::wstring &str, const wchar_t delimiter)
{
	std::vector<std::wstring> result;
	size_t curr = 0u;
	int last = -1;
	while (curr < str.length())
	{
		if (str[curr] == delimiter)
		{
			if (last + 1 != curr)
			{
				//不是一个空的
				result.emplace_back(str.substr(last + 1, curr - last - 1));
				last = curr;
			}
			else
			{
				//是一个空字符串，设置last
				last = curr;
			}
		}
		++curr;
	}
	//还有最后一个
	if (last != -1 && last + 1 != curr)
	{
		result.emplace_back(str.substr(last + 1, str.length() - last - 1));
	}
	return result;
}

std::list<Item> scanner(const std::wstring &str, bool onlynested)
{
	std::wregex regex_orderedlist(L"[0-9]+\\.\\s(.*)");
	std::wregex regex_table_align(L"^([:\\s]?-+[:\\s]?\\|?)+$");
	std::wregex regex_delimiter(L"^[=*-]{1,3}$");
	std::list<Item> items;
	std::wistringstream istringStream(str);
	std::wstring line;
	bool multilines;

	//跨行使用的上下文信息
	MD_TOKEN lastToken = MD_TOKEN::EMPTY;
	std::wstring data;
	std::wstring html_tag;

	multilines = false;
	istringStream >> std::noskipws;//不要忽略空白
	while (std::getline(istringStream, line))
	{
		if (line.compare(L"") == 0)
		{
			lastToken = MD_TOKEN::NEWLINE;//上一个是新的换行
			continue;
		}

		size_t index = 0u;
		while (index < line.length())
		{
			
			//解析每一个字符
			wchar_t ch = line[index];
			if (ch == '`' && line.find(L"```") == 0u && !onlynested)//多行代码
			{
				if (multilines == false)
				{
					if(line.length() > 3)
						html_tag = L"class=" + line.substr(3u);
					//代码块
					multilines = true;
					break;//不需要将标志写入
				}
				else
				{
					//是终止
					multilines = false;
					items.emplace_back(data, MD_TOKEN::CODE, MD_ITEM::LINE, html_tag);
					data = L"";//清空
					html_tag = L"";
					lastToken = MD_TOKEN::CODE;
					break;
				}
			}
			if (multilines == true)
			{
				//需要处理原始数据，使之兼容html
				line = std::regex_replace(line, REGEX_LT, L"&lt;");
				line = std::regex_replace(line, REGEX_GT, L"&gt;");
				data.append(line + L"\n");
				break;
			}
			if (ch == '#' && !onlynested)
			{
				size_t beg = determineData(MD_TOKEN::HEADER1, line);
				size_t syms = line.find_first_not_of('#');
				if (beg >= line.length())
				{
					//是一个非法的标题标记，例如###aa这种，需要计算一共多少个#
					beg = syms;//直接使用最后一个#之后的位置
				}
				auto tk = static_cast<MD_TOKEN>(syms - 1);
				items.emplace_back(line.substr(beg), tk, MD_ITEM::LINE);
				lastToken = tk;
				break;
			}
			else if (!onlynested && (ch == '-' || ch=='*' || ch=='+') && (line.size() > 2u && iswspace(line[1])))
			{
				//分隔符或者列表的某一项
				//if (line.find_last_not_of('-') == std::wstring::npos)
				//{
				//	if (items.size() > 0u)//也可能是标题的分隔
				//	{
				//		auto &&t = items.back();
				//		if (t.getToken() == MD_TOKEN::DATA)
				//		{
				//			//标题格式
				//			t.setToken(MD_TOKEN::HEADER2);
				//			lastToken = MD_TOKEN::HEADER2;
				//			break;
				//		}
				//	}
				//	//分隔符
				//	items.emplace_back(L"", MD_TOKEN::DELIMITER, MD_ITEM::LINE);
				//	lastToken = MD_TOKEN::DELIMITER;
				//	break;
				//}
				//else {
				//	size_t beg = determineData(MD_TOKEN::UNORDERED_LIST, line);
				//	if (beg >= line.length())
				//	{
				//		//非法的标记类似于 -fff
				//		beg = line.find_first_not_of(L"-");
				//	}
				//	if (beg == std::wstring::npos)
				//	{
				//		items.emplace_back(line, MD_TOKEN::DATA, MD_ITEM::LINE);
				//		lastToken = MD_TOKEN::DATA;
				//	}
				//	else
				//	{
				//		items.emplace_back(line.substr(beg), MD_TOKEN::UNORDERED_LIST, MD_ITEM::LINE);
				//		lastToken = MD_TOKEN::UNORDERED_LIST;
				//	}
				//	break;
				//}

				//一定是一个无序列表
				items.emplace_back(line.substr(2), MD_TOKEN::UNORDERED_LIST, MD_ITEM::LINE);
				lastToken = MD_TOKEN::UNORDERED_LIST;
				break;
			}
			else if (!onlynested && (ch == '-' || ch == '*' || ch == '=') && std::regex_match(line, regex_delimiter))
			{
				//一定是一个分隔符
				if (items.size() > 0u && (ch == '=' || ch == '-'))
				{
					auto &&t = items.back();
					if (t.getToken() == MD_TOKEN::DATA)
					{
						//标题格式
						if (ch == '=')
						{
							t.setToken(MD_TOKEN::HEADER1);
							lastToken = MD_TOKEN::HEADER1;
						}
						else {
							t.setToken(MD_TOKEN::HEADER2);
							lastToken = MD_TOKEN::HEADER2;
						}
						break;
					}
				}
				items.emplace_back(L"", MD_TOKEN::DELIMITER, MD_ITEM::LINE);
				lastToken = MD_TOKEN::DELIMITER;
				break;
			}
			else if (!onlynested && ch >= '0' && ch <= '9' && std::regex_match(line, regex_orderedlist))
			{
				//以数字开头，有可能是序列表
				size_t beg = determineData(MD_TOKEN::ORDERED_LIST, line);
				items.emplace_back(line.substr(beg), MD_TOKEN::ORDERED_LIST, MD_ITEM::LINE);
				lastToken = MD_TOKEN::ORDERED_LIST;
				break;
			}
			//else if (ch == '[')
			//{
			//	//内引用格式的标题信息，可以使用正则表达式匹配
			//	auto regex = std::wregex(L"\\[(.*)\\]\\((.*)\\)");
			//	std::wostringstream os;
			//	std::regex_replace(std::ostreambuf_iterator<wchar_t>(os),
			//		line.begin() + index, line.end(),
			//		regex, L"<a href='$2' target='_blank'>$1</a>");
			//	std::wstring htmlcode(os.str());
			//	items.emplace_back(htmlcode, MD_TOKEN::HTML, MD_ITEM::LINE);
			//	break;
			//}
			//else if (ch == '!')
			//{
			//	//图片的引用格式，可以使用正则表达式匹配
			//	auto regex = std::wregex(L"!\\[(.*)\\]\\((.*)\\)");
			//	std::wostringstream os;
			//	std::regex_replace(std::ostreambuf_iterator<wchar_t>(os),
			//		line.begin() + index, line.end(),
			//		regex,
			//		L"<img src='$2' alt='$1' align='middle'>");
			//	std::wstring htmlcode(os.str());
			//	items.emplace_back(htmlcode, MD_TOKEN::HTML, MD_ITEM::LINE);
			//	break;
			//}
			else if (!onlynested && ch == '>')
			{
				//引用
				size_t beg = determineData(MD_TOKEN::QUTOE, line);
				if (beg >= line.length())
				{
					//非法的标记类似于 >fff
					beg = line.find_first_not_of(L">");
				}
				if (beg == std::wstring::npos) {
					items.emplace_back(line, MD_TOKEN::DATA, MD_ITEM::LINE);
					lastToken = MD_TOKEN::DATA;
				}
				else
				{
					//可能有嵌套的引用
					std::wostringstream res, quoteends;
					size_t s;
					size_t pos = line.find_first_not_of('>');
					for (s = index; s < line.length() && s < pos; ++s)
						res << L"<blockquote>";
					res << parse_inner(mdToHTMLDoc(line.substr(beg)), 0u);
					while (s-- != index)
						quoteends << L"</blockquote>";
					items.emplace_back(res.str(), MD_TOKEN::QUTOE, MD_ITEM::LINE, quoteends.str());
					lastToken = MD_TOKEN::QUTOE;
				}
				break;
			}

			else if (!onlynested && ((line.find('|') != std::wstring::npos)))
			{

				bool hasHead = false || lastToken == MD_TOKEN::TABLE_ITEM;
				//表格，可能是表头也可能是数据项、对其选项
				if (hasHead)
				{
					//对齐控制
					//使用反向迭代器
					auto checked_header = items.rbegin();
					/*bool isHead = false;*/
					for (; checked_header != items.rend() && 
						checked_header->getTag().compare(L"head") != 0; ++checked_header)//确定表头
					{
						MD_TOKEN token = checked_header->getToken();
						if (token == MD_TOKEN::TABLE_COLUMN_CENTER ||
							token == MD_TOKEN::TABLE_COLUMN_LEFT ||
							token == MD_TOKEN::TABLE_COLUMN_RIGHT)
							continue;
						break;
					}
					std::list<Item>::iterator head_iter = checked_header.base();
					if (checked_header != items.rend())
					{
						//如果不是没有找到
						--head_iter;//退回其实表头
					}
					//确定了第一个表头的位置或者是头前位置
					auto hitems = split(line, '|');
					for (size_t i =0;i <hitems.size() && head_iter != items.end();++i) 
					{
						//每一个单元格的内容
						auto &&subline = trim(hitems[i], 0u, hitems[i].length());
						size_t sz = subline.length();
						if (sz > 0 && subline[0] == ':' && subline[sz - 1] == ':')
							head_iter->setToken(MD_TOKEN::TABLE_COLUMN_CENTER);
						else if (sz > 0 && subline[sz - 1] == ':')
							head_iter->setToken(MD_TOKEN::TABLE_COLUMN_RIGHT);
						else if (sz > 0 && (subline[0] == ':' || subline[0] == '-'))//其他的控制格式（左对齐）
							;
						else
						{
							items.emplace_back(subline, head_iter->getToken(), MD_ITEM::NESTED);
						}
						++head_iter;
					}
				}
				else
				{
					//表头
					auto hitems = split(line, '|');
					for (size_t s =0u;s<hitems.size();++s)
					{
						auto &&item = hitems[s];
						if(s==0u)
							items.emplace_back(trim(item, 0u, item.length()), MD_TOKEN::TABLE_COLUMN_LEFT, MD_ITEM::LINE, L"head");
						else
							items.emplace_back(trim(item, 0u, item.length()), MD_TOKEN::TABLE_COLUMN_LEFT, MD_ITEM::LINE);
					}
					
				}
				lastToken = MD_TOKEN::TABLE_ITEM;//添加了列表项
				break;
			}
			else
			{
				std::wstring content(parse_inner(mdToHTMLDoc(line), 0u));
				MD_ITEM itemtype = MD_ITEM::NESTED;
				if (lastToken == MD_TOKEN::NEWLINE)
					itemtype = MD_ITEM::LINE;//强制换行
				else
				{
					if (!onlynested) {
						//非嵌套指定
						switch (lastToken)
						{
						case MD_TOKEN::HEADER1:
						case MD_TOKEN::HEADER2:
						case MD_TOKEN::HEADER3:
						case MD_TOKEN::HEADER4:
						case MD_TOKEN::HEADER5:
						case MD_TOKEN::HEADER6:
						case MD_TOKEN::DATA:
						case MD_TOKEN::CODE:
							itemtype = MD_ITEM::LINE;
							break;
						}
					}
				}
				items.emplace_back(content, MD_TOKEN::DATA, itemtype);
				lastToken = MD_TOKEN::DATA;
				break;
			}
			
		}
	}
	return items;
}
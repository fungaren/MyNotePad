#include "stdafx.h"
#include "lex_parse.h"
#include <iostream>
#include <string>
#include <sstream>
#include "mdsymbols.h"
#include <vector>
#include <utility>
#include <regex>
#include <list>
#include <algorithm>
Item::Item(const std::wstring &data, MD_TOKEN token, MD_ITEM itemtype) :m_data(data), m_token(token), m_mditemtype(itemtype)
{

}
Item::Item(const Item &rhs) : m_data(rhs.m_data), m_token(rhs.m_token), m_mditemtype(rhs.m_mditemtype)
{
}
std::wostream &Item::operator<<(std::wostream &os) const
{
	os << "<" << m_token << "," << m_data << "," << m_mditemtype << ">";
	return os;
}
void Item::setData(const std::wstring &str) { m_data = str; }
std::wstring Item::getData() const { return m_data; }

void Item::setToken(MD_TOKEN token)
{
	m_token = token;
}

MD_TOKEN Item::getToken() const
{
	return m_token;
}

void Item::setItemType(MD_ITEM itemtype)
{
	m_mditemtype = itemtype;
}

MD_ITEM Item::getItemType() const
{
	return m_mditemtype;
}

int determineData(MD_TOKEN tokenType, const std::wstring &str, int start = 0) {
	int beg;
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
		//������Ҫ����ת���ַ����������ȷ���߽�
		beg = start;
		for (; beg < str.length();)
		{
			if (str[beg] == '\\')
			{
				beg = beg + 2;//����һ��ת���ַ�
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

	MD_TOKEN token;
	std::wstring data;
	MD_ITEM itemtype = MD_ITEM::LINE;
	multilines = false;
	while (std::getline(istringStream, line))
	{
		int index = 0;
		while (index < line.length())
		{

			//����ÿһ���ַ�
			wchar_t ch = line[index];
			if (ch == '`')
			{
				if (multilines == false)
				{
					data = L"";
					//���ȴ��������Ŀ��Ҳ������ͨ�Ĵ���������
					int dl = line.find_first_not_of('`');
					if (dl == std::wstring::npos || dl == 3)
					{
						index = line.length();
						//�����
						multilines = true;
						token = MD_TOKEN::CODE;
						continue;//����Ҫ����־д��
					}
				}
				else
				{
					index = line.length();
					//����ֹ
					multilines = false;
					items.emplace_back(data, token, MD_ITEM::LINE);
					data = L"";//���
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
				//�ָ��������б��ĳһ��
				if (line.find_last_not_of('-') == std::wstring::npos)
				{
					//�ָ���
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
				//�����ֿ�ͷ���п��������б�
				int beg = determineData(MD_TOKEN::ORDERED_LIST, line);
				items.emplace_back(line.substr(beg), MD_TOKEN::ORDERED_LIST, MD_ITEM::LINE);
				index = line.length();
			}
			else if (ch == '[')
			{
				//�����ø�ʽ�ı�����Ϣ������ʹ��������ʽƥ��
				auto regex = std::wregex(L"\\[(.*)\\]\\((.*)\\)");
				std::wostringstream os;
				std::regex_replace(std::ostreambuf_iterator<wchar_t>(os), line.begin() + index, line.end(), regex, L"<a href='$2' target='_blank'>$1</a>");
				std::wstring htmlcode(os.str());
				items.emplace_back(htmlcode, MD_TOKEN::HTML, MD_ITEM::LINE);
				index = line.length();
			}
			else if (ch == '!')
			{
				//ͼƬ�����ø�ʽ������ʹ��������ʽƥ��
				auto regex = std::wregex(L"!\\[(.*)\\]\\((.*)\\)");
				std::wostringstream os;
				std::regex_replace(std::ostreambuf_iterator<wchar_t>(os), line.begin() + index, line.end(), regex, L"<img src='$2' alt='$1' align='middle'>");
				std::wstring htmlcode(os.str());
				items.emplace_back(htmlcode, MD_TOKEN::HTML, MD_ITEM::LINE);
				index = line.length();
			}
			else if (ch == '>')
			{
				//����
				int beg = determineData(MD_TOKEN::QUTOE, line);
				items.emplace_back(line.substr(beg), MD_TOKEN::QUTOE, MD_ITEM::LINE);
				index = line.length();
			}
			else if (ch == '|')
			{
				//��񣬿����Ǳ�ͷҲ���������������ѡ��
				if (items.back().getToken() == MD_TOKEN::TABLE_COLUMN_LEFT ||
					items.back().getToken() == MD_TOKEN::TABLE_COLUMN_CENTER ||
					items.back().getToken() == MD_TOKEN::TABLE_COLUMN_RIGHT)
				{
					//�������
					int sp;
					int start = index;
					//ʹ�÷��������
					auto checked_header = items.rbegin();
					for (; checked_header != items.rend(); ++checked_header)
					{
						MD_TOKEN token = checked_header->getToken();
						if (token == MD_TOKEN::TABLE_COLUMN_CENTER || token == MD_TOKEN::TABLE_COLUMN_LEFT || token == MD_TOKEN::TABLE_COLUMN_RIGHT)
							continue;
						break;
					}
					std::list<Item>::iterator head_iter = items.end();
					//ȷ���˵�һ����ͷ��λ�û�����ͷǰλ��
					if (checked_header != items.rend())
						head_iter = checked_header.base();//ǰ��ĵ�����
					do {
						sp = determineData(MD_TOKEN::TABLE_ITEM, line, start + 1);
						//start��sp����һ�� | |
						if (line[start + 1] == ':' && line[sp - 1] == ':')
							head_iter->setToken(MD_TOKEN::TABLE_COLUMN_CENTER);
						else if (line[sp - 1] == ':')
							head_iter->setToken(MD_TOKEN::TABLE_COLUMN_RIGHT);
						else if (line[start + 1] == ':')
							;//Ĭ�������
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
					//��ͷ
					int sp;
					int start = index;
					do {
						sp = determineData(MD_TOKEN::TABLE_ITEM, line, start + 1);
						//��������
						auto content = trim(line, start + 1, sp - start - 1);

						items.emplace_back(content, MD_TOKEN::TABLE_COLUMN_LEFT, MD_ITEM::LINE);
						start = sp;
					} while (start < line.length() - 1 && sp != std::wstring::npos);
					index = line.length();
				}
			}
			else
			{
				//һ���������ı�
				std::wregex regex = std::wregex(L"([`\\*`]){1,3}([\\w\\s]*)([`\\*]){1,3}");
				//�Ƿ��ܹ�ƥ��
				bool res = std::regex_search(line.begin() + index, line.end(), regex);
				if (!res)
				{
					//����ƥ��Ļ���Ϊһ��
					items.emplace_back(line.substr(index), MD_TOKEN::DATA, MD_ITEM::LINE);
					itemtype = MD_ITEM::NESTED;
				}
				else
				{
					//����һ�е�����
					MD_ITEM itemtype = MD_ITEM::LINE;
					//Ѱ������ƥ��
					std::wstring suffix;
					std::wsregex_iterator end;
					std::wsregex_iterator iter(line.begin() + index, line.end(), regex);
					for (; iter != end; ++iter) {
						//ÿһ����Ŀ
						if (iter->operator[](1).matched)
						{
							//��������������
							auto prefix = iter->prefix();
							if (prefix.length() > 0)
							{
								//��ʾ��ǰ׺��Ϊ��
								items.emplace_back(prefix.str(), MD_TOKEN::DATA, itemtype);
								itemtype = MD_ITEM::NESTED;
							}
							int symsize = ((*iter)[0].length() - (*iter)[2].length()) / 2;
							switch (symsize)
							{
							case 1:
								//
								//�����Ǵ������б��
								if (*(*iter)[1].first == '`')
									items.emplace_back(iter->operator[](2).str(), MD_TOKEN::CODE, itemtype);
								else
									items.emplace_back(iter->operator[](2).str(), MD_TOKEN::ITALIC, itemtype);
								break;
							case 2:
								items.emplace_back(iter->operator[](2).str(), MD_TOKEN::BOLD, itemtype); break;
							case 3:
								items.emplace_back(iter->operator[](2).str(), MD_TOKEN::ITALIC_BOLD, itemtype); break;
							}
							//�������һ����׺
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
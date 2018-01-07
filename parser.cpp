#include "stdafx.h"
#include "parser.h"
std::wostream &writeAlignTableItem(std::wostream &os, MD_TOKEN table_token)
{
	switch (table_token)
	{
	case MD_TOKEN::TABLE_COLUMN_CENTER:
		os << "align=\"center\""; break;
	case MD_TOKEN::TABLE_COLUMN_RIGHT:
		os << "align=\"right\""; break;
	default:
		os << "align=\"left\""; break;
	}
	return os;
}
std::wostream &writeInner(std::wostream &os, const std::wstring &data)
{
	auto items = scanner(data);
	for (auto &&item : items)
		item.setItemType(MD_ITEM::NESTED);
	parse_fromlex(os, std::begin(items), std::end(items));
	return os;
}
std::wostream &parse_fromlex(std::wostream &os, std::list<Item>::iterator beg, std::list<Item>::iterator end) {
	for (auto citer = beg; citer != end;)
	{
		MD_TOKEN token = citer->getToken();
		if (token == MD_TOKEN::BOLD || token == MD_TOKEN::ITALIC || token == MD_TOKEN::ITALIC_BOLD || token == MD_TOKEN::DATA || token == MD_TOKEN::CODE)
		{
			if (citer->getItemType() == MD_ITEM::LINE)
			{
				std::list<Item>::iterator nested_iter = citer;
				++nested_iter;
				bool nofurther = true;
				for (; nested_iter != end && nested_iter->getItemType() == MD_ITEM::NESTED; ++nested_iter)
				{
					nofurther = false;
				}
				if (token == MD_TOKEN::CODE)
				{
					if (nofurther == true)
						os << L"<pre>";
				}
				else
					os << L"<p>";
				citer->setItemType(MD_ITEM::NESTED);
				parse_fromlex(os, citer, nested_iter);
				citer->setItemType(MD_ITEM::LINE);
				if (token == MD_TOKEN::CODE)
				{
					if (nofurther == true)
						os << L"</pre>";
				}
				else
					os << L"</p>";
				citer = nested_iter;//返回前一个
			}
			else
			{
				//普通的格式
				switch (token)
				{
				case MD_TOKEN::BOLD:os << L"<strong>" << citer->getData() << L"</strong>"; break;
				case MD_TOKEN::ITALIC:os << L"<I>" << citer->getData() << L"</I>"; break;
				case MD_TOKEN::ITALIC_BOLD:os << L"<I><strong>" << citer->getData() << L"</strong></I>"; break;
				case MD_TOKEN::DATA:os << citer->getData(); break;
				case MD_TOKEN::CODE:os << L"<code>" << citer->getData() << L"</code>"; break;
				}
				++citer;
			}
		}
		else if (token == MD_TOKEN::DELIMITER)
		{
			os << "<hr />";
			++citer;
		}
		else if (token == MD_TOKEN::HEADER1 || token == MD_TOKEN::HEADER2 ||
			token == MD_TOKEN::HEADER3 || token == MD_TOKEN::HEADER4 || token == MD_TOKEN::HEADER5 || token == MD_TOKEN::HEADER6)
		{
			int id = static_cast<int>(token) + 1;
			os << L"<h" << id << L">"; writeInner(os, citer->getData());
			os << L"</h" << id << L">";
			++citer;
		}
		else if (token == MD_TOKEN::HTML)
		{
			os << citer->getData(); ++citer;
		}
		else if (token == MD_TOKEN::ORDERED_LIST)
		{
			//有序列表
			std::list<Item>::iterator nested_iter = citer;
			os << L"<ol start=\"1\">\n";
			for (; nested_iter != end && nested_iter->getToken() == MD_TOKEN::ORDERED_LIST; ++nested_iter)
			{
				os << L"<li>"; writeInner(os, nested_iter->getData());
				os << L"</li>" << std::endl;
			}
			os << L"</ol>";
			citer = nested_iter;
		}
		else if (token == MD_TOKEN::UNORDERED_LIST)
		{
			//无序列表
			std::list<Item>::iterator nested_iter = citer;
			os << L"<ul>\n";
			for (; nested_iter != end && nested_iter->getToken() == MD_TOKEN::UNORDERED_LIST; ++nested_iter)
			{
				os << L"<li>"; writeInner(os, nested_iter->getData());
				os << L"</li>" << std::endl;
			}
			os << L"</ul>";
			citer = nested_iter;
		}
		else if (token == MD_TOKEN::QUTOE)
		{
			os << L"<q>"; writeInner(os, citer->getData());
			os << L"</q>" << std::endl;
			++citer;
		}
		else if (token == MD_TOKEN::TABLE_COLUMN_CENTER || token == MD_TOKEN::TABLE_COLUMN_LEFT || token == MD_TOKEN::TABLE_COLUMN_RIGHT)
		{
			os << "<table>\n<tr>\n";
			auto headers_iter = citer;
			int columns = 0;
			for (; headers_iter != end && headers_iter->getItemType() == MD_ITEM::LINE; ++headers_iter)
			{
				os << L"<th "; writeAlignTableItem(os, headers_iter->getToken()) << L">";
				writeInner(os, headers_iter->getData()) << L"</th>";
				++columns;
			}
			int count = 0;
			//绘制表格其他的内容
			for (; headers_iter != end; ++headers_iter)
			{
				auto td_token = headers_iter->getToken();
				if (td_token == MD_TOKEN::TABLE_COLUMN_CENTER || td_token == MD_TOKEN::TABLE_COLUMN_LEFT || td_token == MD_TOKEN::TABLE_COLUMN_RIGHT)
				{
					if (count == 0)
					{
						os << L"<tr>\n";
					}
					os << L"<td "; writeAlignTableItem(os, td_token) << L">";
					writeInner(os, headers_iter->getData()) << L"</td>\n";
					count = (count + 1) % columns;
					if (count == 0)
						os << L"</tr>\n";
				}
				else
				{
					break;
				}
			}
			os << L"</table>\n";
			citer = headers_iter;
		}
		else
			++citer;
	}
	return os;
}
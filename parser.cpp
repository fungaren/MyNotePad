#include "stdafx.h"
#include "lex_parse.h"

std::wostream &writeAlignTableItem(std::wostream &os, MD_TOKEN table_token)
{
	switch (table_token)
	{
	case MD_TOKEN::TABLE_COLUMN_CENTER:
		os << "style=\"text-align:center\""; break;
	case MD_TOKEN::TABLE_COLUMN_RIGHT:
		os << "style=\"text-align:right\""; break;
	default:
		os << "style=\"text-align:left\""; break;
	}
	return os;
}

std::wostream &writeInner(std::wostream &os, const std::wstring &data)
{
	auto items = scanner(data, true);
	parse_fromlex(os, std::begin(items), std::end(items));
	return os;
}

std::wostream &parse_fromlex(std::wostream &os, 
	std::list<Item>::iterator beg, 
	std::list<Item>::iterator end) 
{
	for (auto citer = beg; citer != end;)
	{
		MD_TOKEN token = citer->getToken();
		if (token == MD_TOKEN::DATA || token == MD_TOKEN::HTML)
		{
			if(citer->getItemType() != MD_ITEM::NESTED)
				os << L"<p>";
			os << citer->getData();
			//允许存在多个行组成的内容
			//是否有嵌套的内容
			auto iter = citer;
			auto next = citer;
			while (++iter != end && iter->getItemType() == MD_ITEM::NESTED)
			{
				;
			}
			//确定了范围
			if (iter != citer)
				parse_fromlex(os, ++next, iter);
			if (citer->getItemType() != MD_ITEM::NESTED)
				os << L"</p>";
			//更新迭代器
			citer = iter;
		}
		else if (token == MD_TOKEN::CODE)
		{
			os << L"<pre";
			if (citer->getTag().compare(L"") != 0)
				os << L" " << citer->getTag();
			os << L">";
			os << citer->getData() << L"</pre>\n";
			++citer;
		}
		else if (token == MD_TOKEN::DELIMITER)
		{
			os << "<hr />";
			++citer;
		}
		else if (token == MD_TOKEN::HEADER1 ||
			token == MD_TOKEN::HEADER2 ||
			token == MD_TOKEN::HEADER3 ||
			token == MD_TOKEN::HEADER4 ||
			token == MD_TOKEN::HEADER5 || 
			token == MD_TOKEN::HEADER6)
		{
			//不持支多行的内容
			int id = static_cast<int>(token) + 1;
			os << L"<h" << id << L">"; writeInner(os, citer->getData());
			os << L"</h" << id << L">";
			++citer;
		}
		else if (token == MD_TOKEN::ORDERED_LIST)
		{
			//有序列表
			std::list<Item>::iterator nested_iter = citer;
			os << L"<ol start=\"1\">\n";
			for (; nested_iter != end && nested_iter->getToken() == MD_TOKEN::ORDERED_LIST;)
			{
				os << L"<li>"; writeInner(os, nested_iter->getData());
				//写入内部可能有的其他数据，需要递增到下一个ORDERED_LIST
				auto iter = nested_iter;
				auto next = nested_iter;
				while (++iter != end && iter->getToken() != MD_TOKEN::ORDERED_LIST && iter->getItemType() == MD_ITEM::NESTED)
					continue;
				++next;//递增为下一个
				if (iter != next && iter != end)
					parse_fromlex(os, next, iter);
				os << L"</li>" << std::endl;
				//更改为下一个
				nested_iter = iter;
			}
			os << L"</ol>";
			citer = nested_iter;
		}
		else if (token == MD_TOKEN::UNORDERED_LIST)
		{
			//无序列表
			std::list<Item>::iterator nested_iter = citer;
			os << L"<ul>\n";
			for (; nested_iter != end && nested_iter->getToken() == MD_TOKEN::UNORDERED_LIST;)
			{
				os << L"<li>"; writeInner(os, nested_iter->getData());
				//写入内部可能有的其他数据，需要递增到下一个ORDERED_LIST
				auto iter = nested_iter;
				auto next = nested_iter;
				while (++iter != end && iter->getToken() != MD_TOKEN::UNORDERED_LIST && iter->getItemType() == MD_ITEM::NESTED)
					continue;
				++next;//递增为下一个
				if (iter != next && iter != end)
					parse_fromlex(os, next, iter);
				os << L"</li>" << std::endl;
				//更改为下一个
				nested_iter = iter;
			}
			os << L"</ul>";
			citer = nested_iter;
		}
		else if (token == MD_TOKEN::QUTOE)
		{
			//引用的内容
			//写入内部可能有的其他数据，一直写入知道不为NESTED
			os << citer->getData();
			auto iter = citer;
			auto next = citer;
			++next;
			while (++iter != end && iter->getItemType() == MD_ITEM::NESTED)
				continue;
			//设置了范围
			if(next != iter)
				parse_fromlex(os, next, iter);
			//添加结束
			os << citer->getTag();
			citer = iter;
		}
		else if ((token == MD_TOKEN::TABLE_COLUMN_CENTER ||
			token == MD_TOKEN::TABLE_COLUMN_LEFT ||
			token == MD_TOKEN::TABLE_COLUMN_RIGHT) && citer->getTag().compare(L"head") == 0)
		{
			os << "<table>\n<tr>\n";
			auto headers_iter = citer;
			int columns = 0;
			do
			{
				os << L"<th "; writeAlignTableItem(os, headers_iter->getToken()) << L">";
				writeInner(os, headers_iter->getData()) << L"</th>";
				++columns;
				++headers_iter;
			} while (headers_iter != end && headers_iter->getItemType() == MD_ITEM::LINE &&
				(headers_iter->getToken() == MD_TOKEN::TABLE_COLUMN_LEFT ||
				headers_iter->getToken() == MD_TOKEN::TABLE_COLUMN_RIGHT ||
				headers_iter->getToken() == MD_TOKEN::TABLE_COLUMN_CENTER )&&//必须是一个表格的单元项
				headers_iter->getTag().compare(L"head") != 0);
			int count = 0;
			//绘制表格其他的内容
			for (; headers_iter != end && 
				headers_iter->getItemType() == MD_ITEM::NESTED; ++headers_iter)
			{
				auto td_token = headers_iter->getToken();
				if (td_token == MD_TOKEN::TABLE_COLUMN_CENTER ||
					td_token == MD_TOKEN::TABLE_COLUMN_LEFT ||
					td_token == MD_TOKEN::TABLE_COLUMN_RIGHT)
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
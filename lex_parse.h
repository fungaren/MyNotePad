#pragma once
#include <string>
#include <list>

enum MD_TOKEN {
	HEADER1 = 0, HEADER2, HEADER3, HEADER4, HEADER5, HEADER6,
	UNORDERED_LIST, ORDERED_LIST,//- NUM.
	DELIMITER,//分割线---
	IMAGE_IDENTIFIER,//!,用于图片URL定界
	TEXT_LEFT, TEXT_RIGHT,//[ ]
	URI_LEFT, URI_RIGHT,//( )
	QUTOE,//>
	CODE,// ```
	ITALIC,//*
	BOLD,//**
	ITALIC_BOLD,//***
	TABLE_ITEM,//表的单元格
	TABLE_COLUMN_LEFT, TABLE_COLUMN_CENTER, TABLE_COLUMN_RIGHT,//表头 左对齐、右对齐、剧中
	DATA,//各种数据
	BEGIN,//Markdown的开头标记
	HTML,//HTML标记
	NEWLINE//换行
};
enum MD_ITEM {
	LINE,
	NESTED
};

class Item {
	MD_TOKEN m_token;
	std::wstring m_data;
	MD_ITEM m_mditemtype;
public:
	Item(const std::wstring &data, MD_TOKEN token, MD_ITEM itemtype);
	Item(const Item&);
	std::wostream &operator<<(std::wostream &os) const;
	void setData(const std::wstring &str);
	std::wstring getData() const;
	void setToken(MD_TOKEN);
	MD_TOKEN getToken() const;
	void setItemType(MD_ITEM);
	MD_ITEM getItemType() const;
};
std::list<Item> scanner(const std::wstring &str);
std::wostream &parse_fromlex(std::wostream &os, std::list<Item>::iterator beg, std::list<Item>::iterator end);
std::wostream &writeInner(std::wostream &os, const std::wstring &data);
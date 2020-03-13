#pragma once
#include <string>
#include <list>
#include <regex>
#include <vector>
static const std::wregex REGEX_LT(std::wregex(L"<"));
static const std::wregex REGEX_GT = std::wregex(L">");

enum MD_TOKEN {
	HEADER1 = 0, HEADER2, HEADER3, HEADER4, HEADER5, HEADER6,
	UNORDERED_LIST, ORDERED_LIST,//- NUM.
	DELIMITER,//分割线---
	IMAGE_IDENTIFIER,//!,用于图片URL定界
	TEXT_LEFT, TEXT_RIGHT,//[ ]
	URI_LEFT, URI_RIGHT,//( )
	QUTOE,//>
	CODE,// ```
	LATEX,//Latex 代码
	ITALIC,//*
	BOLD,//**
	ITALIC_BOLD,//***
	TABLE_ITEM,//表的单元格
	TABLE_COLUMN_LEFT, TABLE_COLUMN_CENTER, TABLE_COLUMN_RIGHT,//表头 左对齐、右对齐、剧中
	DATA,//各种数据
	BEGIN,//Markdown的开头标记
	HTML,//HTML标记
	NEWLINE,//换行
	EMPTY//一开始的循环
};
enum MD_ITEM {
	LINE,
	NESTED
};

class Item {
	MD_TOKEN m_token;
	std::wstring m_data;
	std::wstring m_tag;//自定义的HTML标签属性
	MD_ITEM m_mditemtype;
public:
	Item(const std::wstring &data, MD_TOKEN token, MD_ITEM itemtype, const std::wstring &tag=L"");
	Item(const Item&);
	std::wostream &operator<<(std::wostream &os) const;
	void setData(const std::wstring &str);
	const std::wstring &getData() const;
	void setToken(MD_TOKEN);
	const MD_TOKEN &getToken() const;
	void setItemType(MD_ITEM);
	const MD_ITEM &getItemType() const;
	void setTag(const std::wstring &tag);
	const std::wstring &getTag() const;
};
std::wstring trim(const std::wstring &str, size_t start, size_t count);
void split(std::vector<std::wstring> &items,const std::wstring &str, const wchar_t *delimiter,bool allowSpace=false);
void split(std::vector<std::wstring> &items, const std::wstring &str, wchar_t delimiter, bool allowSpace = false);

std::list<Item> scanner(const std::wstring &str, bool onlynested=false);
std::wostream &parse_fromlex(std::wostream &os, std::list<Item>::iterator beg, std::list<Item>::iterator end);
std::wostream &writeInner(std::wostream &os, const std::wstring &data);
size_t determineData(MD_TOKEN tokenType, const std::wstring &str, size_t start = 0u);
std::wstring parse_inner(const std::wstring &str, size_t begin);
std::wstring mdToHTMLDoc(const std::wstring &str);
size_t getClosedRegion(const std::wstring &str, wchar_t start_ch, wchar_t end_ch,size_t start);


bool isTableItem(const MD_TOKEN token);
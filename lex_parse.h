#pragma once
#include <string>
#include <list>
#include "mdsymbols.h"
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
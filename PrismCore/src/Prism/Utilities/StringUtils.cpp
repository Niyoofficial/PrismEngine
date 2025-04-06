#include "pcpch.h"
#include "StringUtils.h"


namespace Prism
{
std::wstring StringToWString(const std::string& string)
{
	std::wstring wstring;
	size_t size;
	wstring.resize(string.length());
	errno_t err = mbstowcs_s(&size, wstring.data(), wstring.size() + 1, string.c_str(), string.size());
	PE_ASSERT(err == 0);
	return wstring;
}

std::string WStringToString(const std::wstring& wstring)
{
	std::string string;
	size_t size;
	string.resize(wstring.length());
	errno_t err = wcstombs_s(&size, string.data(), string.size() + 1, wstring.c_str(), wstring.size());
	PE_ASSERT(err == 0);
	return string;
}
}

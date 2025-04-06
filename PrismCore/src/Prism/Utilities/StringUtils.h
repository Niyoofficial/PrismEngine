#pragma once

#include <string>

namespace Prism
{
std::wstring StringToWString(const std::string& string);
std::string WStringToString(const std::wstring& wstring);
}

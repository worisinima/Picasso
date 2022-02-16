
#include "StringHelper.h"

bool StringHelper::IsContain(const std::string& stringBody, const char& delim)
{
	bool Ret = false;

	std::string::size_type idx = stringBody.find(delim);
	if (idx != std::string::npos)
		Ret = true;
	else
		Ret = false;

	return Ret;
}

bool StringHelper::IsContain(const std::string& stringBody, const std::string& goleStringBody)
{
	bool Ret = false;

	if(stringBody.find(goleStringBody) != std::string::npos)
		Ret = true;

	return Ret;
}

bool StringHelper::IsContain(const std::vector<std::string>& stringContainer, const std::string& goleStringBody)
{
	for (const std::string& data : stringContainer)
	{
		if(data == goleStringBody)
			return true;
	}
	return false;
}

void StringHelper::Split(
	const std::string& s,
	std::vector<std::string>& outSplitStrings,
	const char& delim
)
{
	std::istringstream iss(s);
	std::string temp;

	temp.clear();
	while (std::getline(iss, temp, delim))
	{
		outSplitStrings.emplace_back(std::move(temp));
	}

	return;
}

void StringHelper::Split(const std::string& s, std::vector<std::string>& OutStrings, const std::string& delim)
{
	std::string::size_type pos1, pos2;
	size_t len = s.length();
	pos2 = s.find(delim);
	pos1 = 0;

	while (std::string::npos != pos2)
	{
		OutStrings.emplace_back(s.substr(pos1, pos2 - pos1));
		pos1 = pos2 + delim.size();
		pos2 = s.find(delim, pos1);
	}

	if (pos1 != len)
		OutStrings.emplace_back(s.substr(pos1));
}

std::string StringHelper::Remove(std::string& s, const std::string& removeString)
{
	std::string::size_type n = removeString.length();
	for (std::string::size_type i = s.find(removeString); i != std::string::npos; i = s.find(removeString))
		s.erase(i, n);

	return s;
}

//需要在Properties->C++ ->Preprocessor ->Preprocessor Definitions -> Edit里增加_CRT_SECURE_NO_WARNINGS
std::string StringHelper::WStringToString(const std::wstring& ws)
{
	std::string curLocale = setlocale(LC_ALL, NULL);        // curLocale = "C";
	setlocale(LC_ALL, "chs");
	const wchar_t* _Source = ws.c_str();
	size_t _Dsize = 2 * ws.size() + 1;
	char* _Dest = new char[_Dsize];
	memset(_Dest, 0, _Dsize);
	wcstombs(_Dest, _Source, _Dsize);
	std::string result = _Dest;
	delete[]_Dest;
	setlocale(LC_ALL, curLocale.c_str());
	return result;
}

std::wstring StringHelper::StringToWString(const std::string& s)
{
	setlocale(LC_ALL, "chs");
	const char* _Source = s.c_str();
	size_t _Dsize = s.size() + 1;
	wchar_t* _Dest = new wchar_t[_Dsize];
	wmemset(_Dest, 0, _Dsize);
	mbstowcs(_Dest, _Source, _Dsize);
	std::wstring result = _Dest;
	delete[]_Dest;
	setlocale(LC_ALL, "C");
	return result;
}

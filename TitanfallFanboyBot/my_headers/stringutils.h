#pragma once
#include <string>
#include <vector>

// a bad split function (i wrote it after all)
std::vector<std::string> strSplit(std::string str, std::string chr) {
	std::vector<std::string> out;
	auto lastfound = str.find(chr);
	while (lastfound != std::string::npos) {
		if (str.substr(0, lastfound) != "") out.push_back(str.substr(0, lastfound));
		str.erase(0, lastfound + chr.length());
		lastfound = str.find(chr);
	}
	if (str.substr(0, lastfound) != "") out.push_back(str);
	return out;
}

// a bad string join function
std::string strJoin(std::vector<std::string> vec, std::string joiner) {
	std::string out;
	for (auto& str : vec) out += str + joiner;
	out.erase(out.end() - joiner.size(), out.end());
	return out;
}

#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <string>
#include <vector>


inline std::string ltrim(std::string &&s) {
	s.erase(0, s.find_first_not_of(" \t\r\n\v\f"));
	return s;
}

inline std::string rtrim(std::string &&s) {
	s.erase(s.find_last_not_of(" \t\r\n\v\f") + 1);
	return s;
}

inline std::string trim(std::string &&s) {
	return ltrim(rtrim(std::move(s)));
}

inline std::string ltrim(const std::string &s) {
	return s.substr(s.find_first_not_of(" \t\r\n\v\f"));
}

inline std::string rtrim(const std::string &s) {
	return s.substr(0, s.find_last_not_of(" \t\r\n\v\f") + 1);
}

inline std::string trim(const std::string &s) {
	return ltrim(rtrim(s));
}

#endif // UTILS_H

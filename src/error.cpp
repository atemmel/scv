#include "error.hpp"

std::string errorString;

namespace error {

void set(const std::string& str) {
	errorString = str;
}

void onToken(const std::string& str, const Token& tok) {
	errorString.resize(128);
	std::snprintf(errorString.data(), errorString.size(), "%d:%d %s\n", tok.row, tok.column, str.c_str());
}

const std::string& get() {
	return errorString;
}

bool empty() {
	return errorString.empty();
}

}

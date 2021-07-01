#include "token.hpp"

std::string_view Token::str() const {
	return !value.empty() ? value : tokenStrings[static_cast<size_t>(type)];
}

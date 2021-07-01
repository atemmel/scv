#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

enum class TokenType {
	Identifier,
	Symbol,
	Struct,
	Trait,
	Code,
	LParens,
	RParens,
	LBrace,
	RBrace,
	Less,
	Greater,
	Quote,
	Comma,
	Is,
	At,
	Requires,
	N_TokenTypes,
};

struct Token {
	std::string_view str() const;

	std::string value;
	TokenType type;
	uint32_t column;
	uint32_t row;
	size_t index;
};

constexpr std::array<std::string_view, static_cast<size_t>(TokenType::N_TokenTypes)> tokenStrings = {
	"",
	"",
	"struct",
	"trait",
	"code",
	"(",
	")",
	"{",
	"}",
	"<",
	">",
	"\"",
	",",
	"is",
	"@",
	"requires",
};

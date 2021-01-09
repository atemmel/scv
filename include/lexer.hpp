#pragma once
#include "token.hpp"

#include <vector>

class Lexer {
public:
	Lexer(const std::string& src);
	std::vector<Token> operator()();
private:
	void lexIdentifierOrKeyword(Token& token, std::vector<Token>& tokens);
	void ignoreWhitespace();
	void errorOnCurrent();
	char peek();
	void next();
	void jump(size_t n);

	const std::string& src;
	size_t current = 0;
	uint32_t column = 0;
	uint32_t row = 0;

};

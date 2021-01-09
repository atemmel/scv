#include "lexer.hpp"

#include "error.hpp"

#include <algorithm>
#include <iostream>

Lexer::Lexer(const std::string& src) : src(src) {}

std::vector<Token> Lexer::operator()() {
	current = 0;
	column = 1;
	row = 1;
	size_t end = src.size();
	Token token;
	std::vector<Token> tokens;
	tokens.reserve(32);

	while(current < end) {
		if(current + 1 < end && peek() == '/' && src[current + 1] == '/') {
			// Comment
			jump(2);
			while(current < end && peek() != '\n') {
				next();
			}
		} else if(std::isalpha(peek())) {
			// Identifier or keyword
			lexIdentifierOrKeyword(token, tokens);
		} else if(std::isspace(peek())) {
			ignoreWhitespace();
		} else {
			// Symbol or unrecognized token
			const std::string punct(&src[current], 1);
			auto it = std::find(tokenStrings.cbegin(), tokenStrings.cend(), punct);
			if(it != tokenStrings.cend()) {
				token.column = column;
				token.row = row;
				token.type = static_cast<TokenType>(std::distance(tokenStrings.cbegin(), it));
				tokens.push_back(token);
				next();
			} else if(std::ispunct(peek()) && peek() != '`') {
				token.column = column;
				token.row = row;
				token.type = TokenType::Symbol;
				token.value = peek();
				tokens.push_back(token);
				token.value.clear();
				next();
			} else {
				errorOnCurrent();
				return {};
			}
		}
	}
	return tokens;
}

void Lexer::lexIdentifierOrKeyword(Token& token, std::vector<Token>& tokens) {
	int tokenStart = current;
	uint32_t startColumn = column;
	uint32_t startRow = row;
	while(current < src.size() && !std::isspace(peek()) && !std::ispunct(peek())) {
		next();
	}
	token.value.assign(&src[tokenStart], current - tokenStart);
	auto it = std::find(tokenStrings.cbegin(), tokenStrings.cend(), token.value);

	token.column = startColumn;
	token.row = startRow;

	if(it != tokenStrings.cend()) {
		// Keyword
		token.type = static_cast<TokenType>(std::distance(tokenStrings.cbegin(), it));
		token.value.clear();
		tokens.push_back(token);
	} else {
		// Identifier
		token.type = TokenType::Identifier;
		tokens.push_back(token);
		token.value.clear();
	}
}

void Lexer::ignoreWhitespace() {
	next();
	while(current < src.size() && std::isspace(src[current])) {
		next();
	}
}

void Lexer::errorOnCurrent() {
	std::string buffer;
	buffer.resize(128);
	std::snprintf(buffer.data(), buffer.size(), "%d:%d: Unrecognized token '%c'\n", row, column, peek());
	error::set(buffer);
}

char Lexer::peek() {
	return src[current];
}

void Lexer::next() {
	++current;
	++column;
	if(peek() == '\n') {
		column = 0;
		++row;
	}
}

void Lexer::jump(size_t n) {
	current += n;
	column += n;
}

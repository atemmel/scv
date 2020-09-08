#include "lexer.hpp"
#include "token.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

std::string consume(const std::string& path) {
	std::ifstream file(path.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
	auto size = file.tellg();
	file.seekg(0, std::ios::beg);
	if(size < 1) {
		return std::string();
	}
	std::vector<char> bytes(size);
	file.read(bytes.data(), size);
	return std::string(bytes.data(), size);
}

void dumpTokens(const std::vector<Token>& tokens) {
	for(auto& t : tokens) {
		size_t index = static_cast<size_t>(t.type);
		std::cout << t.row << ":" << t.column << ": type: "<< tokenStrings[index] << " value: " << t.value << '\n';
	}
}

int main() {
	auto src = consume("../examples/specs/person.scvspec");
	Lexer lexer(src);
	auto tokens = lexer();
	dumpTokens(tokens);
}

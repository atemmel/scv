#include "lexer.hpp"
#include "ast.hpp"
#include "astprinter.hpp"
#include "emitter.hpp"
#include "error.hpp"

#include <fstream>
#include <iostream>

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

void dieIfError() {
	if(!error::empty()) {
		std::cerr << error::get();
		std::exit(EXIT_FAILURE);
	}
}

int main() {
	auto src = consume("../examples/specs/person.scvspec");
	Lexer lexer(src);
	auto tokens = lexer();
	dieIfError();
	//dumpTokens(tokens);
	Parser parser(tokens);
	auto root = parser();
	dieIfError();
	if(root) {
		//AstPrinter printer;
		//printer.print(*root);

		Emitter emitter(*root, "person.hpp");
		emitter();
		dieIfError();
	}
	return EXIT_SUCCESS;
}

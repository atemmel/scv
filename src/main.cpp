#include "lexer.hpp"
#include "ast.hpp"
#include "astprinter.hpp"
#include "emitter.hpp"

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

int main() {
	auto src = consume("../examples/specs/person.scvspec");
	Lexer lexer(src);
	auto tokens = lexer();
	//dumpTokens(tokens);
	Parser parser(tokens);
	auto root = parser();
	if(root) {
		//AstPrinter printer;
		//printer.print(*root);

		Emitter emitter(*root, "person.hpp");
		bool result = emitter();
		return result ? EXIT_SUCCESS : EXIT_FAILURE;
	} else {
		std::cerr << "null :(\n";
		// TODO: This is an error
	}
}

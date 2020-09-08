#include "argparser.hpp"
#include "global.hpp"
#include "lexer.hpp"
#include "ast.hpp"
#include "astprinter.hpp"
#include "emitter.hpp"
#include "error.hpp"
#include "utils.hpp"

void pipeline(const std::string_view& sv) {
	auto src = consume(sv.data());
	Lexer lexer(src);
	auto tokens = lexer();
	dieIfError();

	if(global::verboseAllFlag || global::verboseTokenizationFlag) {
		dumpTokens(tokens);
	}

	Parser parser(tokens);
	auto root = parser();
	dieIfError();
	if(!root) {
		return;
	}

	if(global::verboseAllFlag || global::verboseAstFlag) {
		AstPrinter printer;
		printer.print(*root);
	}

	auto file = getFile(sv);
	file = setStub(file, "hpp");
	auto path = joinPaths(global::outputPath, file);

	Emitter emitter(*root, path);
	emitter();
	dieIfError();
}

int main(int argc, char** argv) {
	ArgParser argParser(argc, argv);
	argParser.addBool(&global::verboseAllFlag, "--verbose");
	argParser.addBool(&global::verboseTokenizationFlag, "--verbose-tokenization");
	argParser.addBool(&global::verboseAstFlag, "--verbose-ast");
	argParser.addString(&global::outputPath, "--output");

	auto input = argParser.unwind();

	for(const auto sv : input) {
		pipeline(sv);
	}

	return EXIT_SUCCESS;
}

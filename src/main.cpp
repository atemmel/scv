#include "argparser.hpp"
#include "global.hpp"
#include "pipeline.hpp"


int main(int argc, char** argv) {
	ArgParser argParser(argc, argv);
	argParser.addBool(&global::verboseAllFlag, "--verbose");
	argParser.addBool(&global::verboseTokenizationFlag, "--verbose-tokenization");
	argParser.addBool(&global::verboseAstFlag, "--verbose-ast");
	argParser.addString(&global::outputPath, "--output");

	auto input = argParser.unwind();

	Pipeline::full(input);

	return EXIT_SUCCESS;
}

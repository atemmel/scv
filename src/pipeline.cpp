#include "pipeline.hpp"

#include "ast.hpp"
#include "astprinter.hpp"
#include "emitter.hpp"
#include "global.hpp"
#include "utils.hpp"
#include "lexer.hpp"

#include <iostream>

std::set<std::string_view> Pipeline::previouslyProcessed;
std::list<std::string> Pipeline::storedSrcs;

void Pipeline::full(const std::vector<std::string_view>& inputs) {
	RootAstNode::Ptr firstRoot = nullptr;
	std::string_view outputFile;

	for(const auto sv : inputs) {
		if(previouslyProcessed.count(sv) > 0) {
			continue;
		}

		if(global::verboseAllFlag) {
			std::cout << "Processing " << sv << '\n';
		}
		auto& src = Pipeline::readFile(sv);
		auto root = Pipeline::buildRootFromSrc(src);
		if(!root) {
			return;
		}

		if(!firstRoot) {
			firstRoot = std::move(root);
			outputFile = sv;
		} else {
			//firstRoot->join(root);
		}
		break;
	}

	if(global::verboseAllFlag || global::verboseAstFlag) {
		AstPrinter printer;
		printer.print(*firstRoot);
	}

	auto file = getFile(outputFile);
	file = setStub(file, "hpp");
	auto path = joinPaths(global::outputPath, file);

	Emitter emitter(*firstRoot, path);
	emitter();
	dieIfError();
}

RootAstNode::Ptr Pipeline::buildRootFromSrc(const std::string& src) {
	Lexer lexer(src);
	auto tokens = lexer();
	dieIfError();

	Parser parser(tokens, src);
	auto root = parser();
	dieIfError();

	return root;
}

const std::string& Pipeline::readFile(const std::string_view sv) {
	previouslyProcessed.emplace(sv);
	auto src = consume(sv.data());
	dieIfError();

	// This is most likely not a malicious move as source files
	// generally are more than 16-32 bytes in size, meaning that
	// the drawbacks of SSO do not need to be accounted for
	storedSrcs.push_back(std::move(src));

	return storedSrcs.back();
}
